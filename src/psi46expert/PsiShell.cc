/*!
 * \file PsiShell.cc
 * \brief Implementation of psi::Shell class.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 07-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Console input moved in the separate thread.
 * 06-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Each command will be executed in a separate thread.
 * 28-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>
#include <readline/readline.h>
#include <readline/history.h>
#include <pthread.h>

#include "psi/log.h"
#include "PsiShell.h"

static const std::string LOG_HEAD = "PsiShell";

using namespace psi::control;

Shell::Shell(const std::string& aHistoryFileName)
    : historyFileName(aHistoryFileName), prompt("psi46expert> "), runNext(true)
{
    rl_bind_key('\t', &rl_insert);
    read_history(historyFileName.c_str());
}

Shell::~Shell()
{
    write_history(historyFileName.c_str());
}

void Shell::Run(bool printHelpLine)
{
    if(printHelpLine)
        Log<Info>() << "Please enter a command or 'help' to see a list of the available commands." << std::endl;

    while(RunNext())
    {
        const std::string p = ReadLine();
        Log<Debug>() << prompt << p << std::endl;
        {
            boost::lock_guard<boost::mutex> lock(mutex);
            if(p.size() == 0 || interruptionRequested)
                continue;
        }
        std::vector<std::string> commandLineArguments;
        boost::algorithm::split(commandLineArguments, p, boost::algorithm::is_any_of(" "),
                                boost::algorithm::token_compress_on);
        boost::shared_ptr<Command> command;
        const bool result = FindAndCreateCommand(*this, commandLineArguments, command);
        if(result)
        {
            boost::unique_lock<boost::mutex> lock(mutex);
            commandRunning = true;
            interruptionRequested = false;
            boost::thread commandThread(boost::bind(&Shell::SafeCommandExecute, this, command));
            while(commandRunning)
            {
                stateChange.wait(lock);
                if(interruptionRequested)
                {
                    commandThread.interrupt();
                    commandThread.join();
                }
            }
        }
    }

    {
        boost::lock_guard<boost::mutex> lock(mutex);
        runNext = true;
        interruptionRequested = false;
    }
}
void Shell::InterruptExecution()
{
    {
        boost::lock_guard<boost::mutex> lock(mutex);
        interruptionRequested = true;
        runNext = false;
    }
    stateChange.notify_one();
}

std::string Shell::ReadLine()
{
    boost::unique_lock<boost::mutex> lock(mutex);
    readLineRunning = true;
    std::string line;
    boost::thread readThread(boost::bind(&Shell::SafeReadLine, this, &line));
    while(readLineRunning)
    {
        stateChange.wait(lock);
        if(interruptionRequested)
        {
            boost::thread::native_handle_type nativeReadThread = readThread.native_handle();
            if(pthread_cancel(nativeReadThread))
                THROW_PSI_EXCEPTION("Unable to cancel the console input thread.");

            void* result;
            if(pthread_join(nativeReadThread, &result))
                THROW_PSI_EXCEPTION("Unable to join the console input thread.");

            if(result != PTHREAD_CANCELED)
                THROW_PSI_EXCEPTION("The console input thread is not canceled after a successful join.");
            readLineRunning = false;
        }
    }
    if (line.size() > 0)
        add_history (line.c_str());
    return line;
}

bool Shell::RunNext()
{
    boost::lock_guard<boost::mutex> lock(mutex);
    return runNext;
}

void Shell::Execute(const commands::Exit&)
{
    Log<Info>(LOG_HEAD) << "Exiting..." << std::endl;
    boost::lock_guard<boost::mutex> lock(mutex);
    runNext = false;
}

void Shell::Execute(const commands::Help&)
{
    Log<Info>() << "List of the available commands:" << std::endl;
    Log<Info>() << "exit - exit from the program." << std::endl;
    Log<Info>() << "help - print out this list." << std::endl;
    Log<Info>() << "IV - run an IV test." << std::endl;
}

void Shell::SafeCommandExecute(boost::shared_ptr<Command> command)
{
    try
    {
        command->Execute();
    }
    catch(incorrect_command_exception& e)
    {
        psi::Log<psi::Error>(LOG_HEAD) << "ERROR: " << "Incorrect command format. " << e.what() << std::endl
                                       << "Please use 'help command_name' to see the command definition." << std::endl;
    }
    catch(psi::exception& e)
    {
        psi::Log<psi::Error>(LOG_HEAD) << "ERROR: " << e.what() << std::endl;
    }
    catch(boost::thread_interrupted&) {}

    {
        boost::lock_guard<boost::mutex> lock(mutex);
        commandRunning = false;
    }
    stateChange.notify_one();
}

void Shell::SafeReadLine(std::string* line)
{
    try
    {
        char* char_line = readline (prompt.c_str());
        *line = std::string(char_line);
        free(char_line);
    }
    catch(boost::thread_interrupted&) {}

    {
        boost::lock_guard<boost::mutex> lock(mutex);
        readLineRunning = false;
    }
    stateChange.notify_one();
}
