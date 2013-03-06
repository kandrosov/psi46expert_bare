/*!
 * \file PsiShell.cc
 * \brief Implementation of psi::Shell class.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 06-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Each command will be executed in a separate thread.
 * 28-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>
#include <readline/readline.h>
#include <readline/history.h>

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

void Shell::Run()
{
    Log<Info>() << "Please enter a command or 'help' to see a list of the available commands." << std::endl;

    while(runNext)
    {
        const std::string p = ReadLine();
        Log<Debug>() << prompt << p << std::endl;
        std::vector<std::string> commandLineArguments;
        boost::algorithm::split(commandLineArguments, p, boost::algorithm::is_any_of(" "),
                                boost::algorithm::token_compress_on);
        boost::shared_ptr<Command> command;
        const bool result = FindAndCreateCommand(*this, commandLineArguments, command);
        if(result)
        {
            commandRunning = true;
            interruptionRequested = false;
            boost::thread commandThread(boost::bind(&Shell::SafeCommandExecute, this, command));
            boost::unique_lock<boost::mutex> lock(mutex);
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
}
void Shell::InterruptCommandExecution()
{
    {
        boost::lock_guard<boost::mutex> lock(mutex);
        interruptionRequested = true;
    }
    stateChange.notify_one();
}

std::string Shell::ReadLine()
{
    char* line = readline (prompt.c_str());
    if (line && *line)
        add_history (line);
    const std::string str = std::string(line);
    free(line);
    return str;
}

void Shell::Execute(const commands::Exit&)
{
    Log<Info>() << "Exiting..." << std::endl;
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
    catch(boost::thread_interrupted&)
    {
    }

    {
        boost::lock_guard<boost::mutex> lock(mutex);
        commandRunning = false;
    }
    stateChange.notify_one();
}
