/*!
 * \file ShellCommands.h
 * \brief Command definition for the psi shell.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 26-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#pragma once

#include "Command.h"

namespace psi {
namespace control {

class Shell;

namespace commands {
PSI_CONTROL_SIMPLE_TARGETED_COMMAND(Shell, Exit)
PSI_CONTROL_SIMPLE_TARGETED_COMMAND(Shell, Help)
} // commands

template<>
class CommandProvider<Shell>
{
public:
    typedef commands::detail::CommandDescriptor<Shell> Descriptor;
    typedef std::map<std::string, commands::detail::CommandDescriptor<Shell> > CommandMap;
    static const CommandMap& Commands()
    {
        using namespace commands::detail;
        static CommandMap map;
        if(!map.size())
        {
            map["exit"] = Descriptor(new ExitPrototype(), "exit from the program", "exit from the program");
            map["help"] = Descriptor(new HelpPrototype(), "print help", "print help");
        }
        return map;
    }
};

} // control
} // psi
