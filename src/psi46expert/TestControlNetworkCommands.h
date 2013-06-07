/*!
 * \file TestControlNetworkCommands.h
 * \brief Command definition for the TestControlNetwork.
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 */

#pragma once

#include "Command.h"

namespace psi {
namespace control {

class TestControlNetwork;

namespace commands {

namespace detail {
class BiasData {
public:
    static BiasData Parse(const std::vector<std::string>& commandLineArguments) {
        typedef std::map<std::string, bool> StateMap;
        static StateMap stateMap;
        if(!stateMap.size()) {
            stateMap["enable"] = true;
            stateMap["disable"] = false;
        }
        if(commandLineArguments.size() != 2)
            throw incorrect_command_exception("bias command", "Usage: bias <state>, where state=enalbe|disable");
        const std::string& stateString = commandLineArguments[1];
        StateMap::const_iterator iter = stateMap.find(stateString);
        if(iter == stateMap.end())
            throw incorrect_command_exception("bias command", "Unknown bias state = '" + stateString + "'.");
        return BiasData(iter->second);
    }
    BiasData(bool _enable) : enable(_enable) {}
    bool Enable() const {
        return enable;
    }
private:
    bool enable;
};
} //detail
PSI_CONTROL_TARGETED_COMMAND(TestControlNetwork, Bias, BiasData)

PSI_CONTROL_SIMPLE_TARGETED_COMMAND(TestControlNetwork, FullTest)
PSI_CONTROL_SIMPLE_TARGETED_COMMAND(TestControlNetwork, IV)
PSI_CONTROL_SIMPLE_TARGETED_COMMAND(TestControlNetwork, TestDacProgramming)
} // commands

template<>
class CommandProvider<TestControlNetwork> {
public:
    typedef commands::detail::CommandDescriptor<TestControlNetwork> Descriptor;
    typedef std::map<std::string, commands::detail::CommandDescriptor<TestControlNetwork> > CommandMap;
    static const CommandMap& Commands() {
        using namespace commands::detail;
        static CommandMap map;
        if(!map.size()) {
            map["bias"] = Descriptor(new BiasPrototype(), "enable/disable bias voltage", "enable/disable bias voltage");
            map["full_test"] = Descriptor(new FullTestPrototype(), "run full test and calibration",
                                          "run full test and calibration");
            map["iv"] = Descriptor(new IVPrototype(), "do the IV test", "do the IV test");
            map["test_dac_prog"] = Descriptor(new TestDacProgrammingPrototype(), "test if DACs are programable",
                                              "test if DACs are programable");
        }
        return map;
    }
};

} // control
} // psi
