/*!
 * \file PsiShell.h
 * \brief Definition of psi::control::Shell class.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 28-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#pragma once

#include "ShellCommands.h"
#include "BasePixel/constants.h"

namespace psi {
namespace control {

class Shell : boost::noncopyable
{
public:
    Shell(const std::string& aHistoryFileName);
    ~Shell();
    void Run();

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

private:
    std::string historyFileName;
    std::string prompt;
    bool runNext;
};

} // psi
} // control
