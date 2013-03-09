/*!
 * \file PsiShell.h
 * \brief Definition of psi::control::Shell class.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 09-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Command 'help' improved.
 * 07-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Console input moved in the separate thread.
 *      - Added TestControlNetwork as supported target.
 * 06-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Each command will be executed in a separate thread.
 * 28-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#pragma once


#include <iomanip>

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/function.hpp>

#include "BasePixel/constants.h"
#include "ShellCommands.h"
#include "TestControlNetwork.h"

namespace psi
{
namespace control
{

class Shell : boost::noncopyable
{
public:
    Shell(const std::string& aHistoryFileName, boost::shared_ptr<TestControlNetwork> aTestControlNetwork);
    ~Shell();
    void Run(bool printHelpLine = true);
    void InterruptExecution();
    void Exit();

    void Execute(const commands::Exit& exitCommand);
    void Execute(const commands::Help& helpCommand);
private:
    std::string ReadLine();

    template<typename Target>
    const commands::detail::CommandDescriptor<Target>* FindCommandDescriptor(const std::string& commandName) const
    {
        typedef typename CommandProvider<Target>::CommandMap CommandMap;
        CommandMap map = CommandProvider<Target>::Commands();
        typename CommandMap::const_iterator iter = map.find(commandName);
        if(iter == map.end())
            return 0;
        return &iter->second;
    }

    template<typename Target>
    bool FindAndCreateCommand(Target& target, const std::vector<std::string>& commandLineArguments,
                              boost::shared_ptr<Command>& command) const
    {
        const commands::detail::CommandDescriptor<Target>* descriptor =
            FindCommandDescriptor<Target>(commandLineArguments[0]);
        if(!descriptor)
            return false;
        command = descriptor->prototype->Create(target, commandLineArguments);
        return true;
    }

    template<typename Target>
    void PrintCommandList(const std::string& header) const
    {
        static const unsigned COMMAND_NAME_COLUMN_WIDTH = 20;
        typedef typename CommandProvider<Target>::CommandMap CommandMap;
        CommandMap map = CommandProvider<Target>::Commands();

        LogInfo() << header << std::endl;
        for(typename CommandMap::const_iterator iter = map.begin(); iter != map.end(); ++iter)
        {
            LogInfo() << "\t" << std::setw(COMMAND_NAME_COLUMN_WIDTH) << std::left << iter->first
                      << iter->second.short_help << std::endl;
        }
        LogInfo() << std::endl;
    }

    template<typename Target>
    bool PrintDetailedCommandHelp(const std::string& commandName)
    {
        const commands::detail::CommandDescriptor<Target>* descriptor = FindCommandDescriptor<Target>(commandName);
        if(!descriptor)
            return false;
        LogInfo() << commandName << ": " << descriptor->short_help << std::endl << std::endl;
        LogInfo() << descriptor->long_help << std::endl << std::endl;
        return true;
    }

    void SafeCommandExecute(boost::shared_ptr<Command> command);
    void SafeReadLine(std::string* line);

    bool RunNext();

private:
    boost::mutex mutex;
    boost::condition_variable stateChange;
    std::string historyFileName;
    std::string prompt;
    bool runNext, commandRunning, readLineRunning, interruptionRequested;
    boost::shared_ptr<TestControlNetwork> testControlNetwork;
};

} // control
} // psi
