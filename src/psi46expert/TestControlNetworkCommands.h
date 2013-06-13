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

class AddressDecodingData {
public:
    static AddressDecodingData Parse(const std::vector<std::string>& commandLineArguments) {
        typedef std::map<std::string, bool> StateMap;
        static const std::string debugParam = "debug";
        static const std::string exceptionHeader = "address_decoding command";
        static const std::string exceptionMessage = "Usage: address_decoding <debug> <try_count>";
        if(commandLineArguments.size() > 3)
            throw incorrect_command_exception(exceptionHeader, exceptionMessage);
        bool debug = false;
        unsigned countIndex = 0;
        if(commandLineArguments.size() == 3) {
            countIndex = 2;
            if(commandLineArguments[1] == debugParam)
                debug = true;
            else
                throw incorrect_command_exception(exceptionHeader, exceptionMessage);
        }
        if(commandLineArguments.size() == 2)
            countIndex = 1;
        unsigned maxTryCount = 1;
        if(countIndex && ! detail::Parse(commandLineArguments[countIndex], maxTryCount))
            throw incorrect_command_exception(exceptionHeader, exceptionMessage);
        return AddressDecodingData(debug, maxTryCount);
    }
    AddressDecodingData(bool _debug, unsigned _maxTryCount) : debug(_debug), maxTryCount(_maxTryCount) {}
    bool Debug() const {
        return debug;
    }
    unsigned MaxTryCount() const {
        return maxTryCount;
    }
private:
    bool debug;
    unsigned maxTryCount;
};

} //detail
PSI_CONTROL_TARGETED_COMMAND(TestControlNetwork, Bias, BiasData)
PSI_CONTROL_TARGETED_COMMAND(TestControlNetwork, AddressDecoding, AddressDecodingData)

PSI_CONTROL_SIMPLE_TARGETED_COMMAND(TestControlNetwork, PreTest)
PSI_CONTROL_SIMPLE_TARGETED_COMMAND(TestControlNetwork, FullTest)
PSI_CONTROL_SIMPLE_TARGETED_COMMAND(TestControlNetwork, Calibration)
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
            map["full_test"] = Descriptor(new FullTestPrototype(), "run full test",
                                          "run full test");
            map["iv"] = Descriptor(new IVPrototype(), "do the IV test", "do the IV test");
            map["test_dac_prog"] = Descriptor(new TestDacProgrammingPrototype(), "test if DACs are programable",
                                              "test if DACs are programable");
            map["address_decoding"] = Descriptor(new AddressDecodingPrototype(), "run address decoding test",
                                                 "Usage: address_decoding <debug> <try_count>");
            map["pre_test"] = Descriptor(new PreTestPrototype(), "run pre-test", "run pre-test");
            map["calibration"] = Descriptor(new CalibrationPrototype(), "run calibration", "run calibration");
        }
        return map;
    }
};

} // control
} // psi
