/*!
 * \file ShellCommands.h
 * \brief Command definition for the psi shell.
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 */

#pragma once

#include "Command.h"

namespace psi {
namespace control {

class Shell;

namespace commands {
PSI_CONTROL_SIMPLE_TARGETED_COMMAND(Shell, Exit)

namespace detail {
class HelpData {
public:
    static HelpData Parse(const std::vector<std::string>& commandLineArguments) {
        if(commandLineArguments.size() > 1)
            return HelpData(commandLineArguments[1]);
        return HelpData();
    }
    HelpData() : commandName("") {}
    HelpData(const std::string& aCommandName) : commandName(aCommandName) {}
    bool DetailedHelpForOneCommand() const {
        return commandName != "";
    }
    const std::string& CommandName() const {
        return commandName;
    }
private:
    std::string commandName;
};
} // detail
PSI_CONTROL_TARGETED_COMMAND(Shell, Help, HelpData)

namespace detail {
template<typename CommandTag>
class StringParameter {
public:
    static StringParameter Parse(const std::vector<std::string>& commandLineArguments) {
        if(commandLineArguments.size() > 1)
            return StringParameter(commandLineArguments[1]);
        throw incorrect_command_exception(CommandTag::Name(), "Expecting one string parameter.");
    }
    StringParameter() : parameterValue("") {}
    StringParameter(const std::string& value) : parameterValue(value) {}
    const std::string& ParameterValue() const {
        return parameterValue;
    }
private:
    std::string parameterValue;

};
}
} // commands

template<>
class CommandProvider<Shell> {
public:
    typedef commands::detail::CommandDescriptor<Shell> Descriptor;
    typedef std::map<std::string, commands::detail::CommandDescriptor<Shell> > CommandMap;
    static const CommandMap& Commands() {
        using namespace commands::detail;
        static CommandMap map;
        if(!map.size()) {
            map["exit"] = Descriptor(new ExitPrototype(), "exit from the program",
                                     "Usage: exit\n\n"
                                     "Actions:\n"
                                     "- turn off bias voltage if it is on;\n"
                                     "- disconnect from the testboard;\n"
                                     "- free used resources;\n"
                                     "- exit from the program.");
            map["help"] = Descriptor(new HelpPrototype(), "print help",
                                     "Usage: help [command_name]\n\n"
                                     "Description:\n"
                                     "If run without arguments, the list of all available commands will be shown.\n"
                                     "If command_name specified, a detailed help about 'command_name' will be shown.");
        }
        return map;
    }
};

} // control
} // psi
