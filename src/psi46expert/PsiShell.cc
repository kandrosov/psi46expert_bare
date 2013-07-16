/*!
 * \file PsiShell.cc
 * \brief Implementation of psi::Shell class.
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 */

#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>
#include <readline/readline.h>
#include <readline/history.h>
#include <pthread.h>

#include "psi/log.h"
#include "PsiShell.h"
#include "BasePixel/DataStorage.h"

static const std::string LOG_HEAD = "PsiShell";

using namespace psi::control;

Shell::Shell(const std::string& aHistoryFileName, boost::shared_ptr<TestControlNetwork> aTestControlNetwork)
    : historyFileName(aHistoryFileName), prompt("psi46expert> "), runNext(true), commandRunning(false),
      readLineRunning(false), interruptionRequested(false), testControlNetwork(aTestControlNetwork)
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
        LogInfo() << "Please enter a command or 'help' to see a list of the available commands." << std::endl;

    while(RunNext()) {
        LogInfo() << "\a";
        const std::string p = ReadLine();
        LogDebug() << prompt << p << std::endl;
        {
            boost::lock_guard<boost::mutex> lock(mutex);
            if(p.size() == 0 || interruptionRequested)
                continue;
        }
        std::vector<std::string> commandLineArguments;
        boost::algorithm::split(commandLineArguments, p, boost::algorithm::is_any_of(" "),
                                boost::algorithm::token_compress_on);
        boost::shared_ptr<Command> command;
        bool result = false;
        try {
            result = FindAndCreateCommand(*this, commandLineArguments, command);
            if(!result)
                result = FindAndCreateCommand(*testControlNetwork, commandLineArguments, command);
        } catch(incorrect_command_exception& e) {
            LogInfo(LOG_HEAD) << "Incorrect usage of '" << e.header() << "'." << std::endl << e.what() << std::endl;
        }

        if(result) {
            boost::unique_lock<boost::mutex> lock(mutex);
            commandRunning = true;
            interruptionRequested = false;
            boost::thread commandThread(boost::bind(&Shell::SafeCommandExecute, this, command));
            while(commandRunning) {
                stateChange.wait(lock);
                if(interruptionRequested) {
                    commandThread.interrupt();
                    lock.mutex()->unlock();
                    commandThread.join();
                    lock.mutex()->lock();
                    commandRunning = false;
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
    while(readLineRunning) {
        stateChange.wait(lock);
        if(interruptionRequested) {
            boost::thread::native_handle_type nativeReadThread = readThread.native_handle();
            if(!pthread_cancel(nativeReadThread)) {
                void* result;
                if(pthread_join(nativeReadThread, &result))
                    THROW_PSI_EXCEPTION("Unable to join the console input thread.");

                if(result != PTHREAD_CANCELED)
                    THROW_PSI_EXCEPTION("The console input thread is not canceled after a successful join.");
            }
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
    LogInfo(LOG_HEAD) << "Exiting..." << std::endl;
    boost::lock_guard<boost::mutex> lock(mutex);
    runNext = false;
}

void Shell::Execute(const commands::Help& cmd)
{
    if(cmd.getData().DetailedHelpForOneCommand()) {
        const std::string& commandName = cmd.getData().CommandName();
        bool result = PrintDetailedCommandHelp<Shell>(commandName);
        if(!result)
            result = PrintDetailedCommandHelp<TestControlNetwork>(commandName);
        if(!result)
            LogInfo() << "Command '" << commandName << "' not found. To see the availabe commands use 'help' without"
                      " arguments." << std::endl;
    } else {
        PrintCommandList<Shell>("Available shell commands:");
        PrintCommandList<TestControlNetwork>("Available test control commands:");
        LogInfo() << "Use 'help command_name' to see a detailed command description.\n\n";
    }
}

void Shell::Execute(const commands::OperatorName& operatorNameCommand)
{
    DataStorage::Active().SetOperatorName(operatorNameCommand.getData().ParameterValue());
}

void Shell::Execute(const commands::DetectorName& detectorNameCommand)
{
    DataStorage::Active().SetDetectorName(detectorNameCommand.getData().ParameterValue());
}

void Shell::SafeCommandExecute(boost::shared_ptr<Command> command)
{
    try {
        try {
            command->Execute();
        } catch(incorrect_command_exception& e) {
            psi::LogError(e.header()) << "ERROR: " << "Incorrect command format. " << e.message() << std::endl
                                      << "Please use 'help command_name' to see the command definition." << std::endl;
        } catch(psi::exception& e) {
            psi::LogError(e.header()) << "ERROR: " << e.message() << std::endl;
        }

        {
            boost::lock_guard<boost::mutex> lock(mutex);
            commandRunning = false;
        }
        stateChange.notify_one();

    } catch(boost::thread_interrupted&) {}
}

void Shell::SafeReadLine(std::string* line)
{
    try {
        char* char_line = readline (prompt.c_str());
        *line = std::string(char_line);
        free(char_line);
    } catch(boost::thread_interrupted&) {}

    {
        boost::lock_guard<boost::mutex> lock(mutex);
        readLineRunning = false;
    }
    stateChange.notify_one();
}
