/*!
 * \file PsiShell.h
 * \brief Definition of psi::control::Shell class.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 06-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Each command will be executed in a separate thread.
 * 28-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#pragma once

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/function.hpp>

#include "BasePixel/constants.h"
#include "ShellCommands.h"

namespace psi {
namespace control {

class Shell : boost::noncopyable
{
public:
    typedef boost::function<void ()> AfterCommandExecutionCallback;
    typedef boost::function<void (const std::exception&)> OnErrorCallback;


    Shell(const std::string& aHistoryFileName);
    ~Shell();
    void Run(bool printHelpLine = true);
    void InterruptExecution();
    void Exit();

    void Execute(const commands::Exit& exitCommand);
    void Execute(const commands::Help& helpCommand);
private:
    std::string ReadLine();

    template<typename Target>
    bool FindAndCreateCommand(Target& target, const std::vector<std::string>& commandLineArguments,
                          boost::shared_ptr<Command>& command)
    {
        typedef typename CommandProvider<Target>::CommandMap CommandMap;
        CommandMap map = CommandProvider<Target>::Commands();
        typename CommandMap::const_iterator iter = map.find(commandLineArguments[0]);
        if(iter == map.end())
            return false;
        command = iter->second.prototype->Create(target, commandLineArguments);
        return true;
    }

    void SafeCommandExecute(boost::shared_ptr<Command> command);

    bool RunNext();

private:
    boost::mutex mutex;
    boost::condition_variable stateChange;
    std::string historyFileName;
    std::string prompt;
    bool runNext;
    bool commandRunning;
    bool interruptionRequested;
};

} // psi
} // control
