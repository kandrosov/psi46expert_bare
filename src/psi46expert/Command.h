/*!
 * \file Command.h
 * \brief Common definitions for psi::control commands.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 18-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Command tag added.
 * 06-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - psi::exception now have header and message.
 * 26-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>

#include "psi/exception.h"

#define PSI_CONTROL_SIMPLE_TARGETED_COMMAND(target,name) \
    namespace detail { \
    class name##Data {}; \
    struct name##Tag { static std::string Name() { return #name; } }; \
    typedef TargetedCommandPrototype< target, name##Data > name##Prototype; \
    } \
    typedef detail::TargetedCommand<target, detail::name##Data> name; \
     

#define PSI_CONTROL_TARGETED_COMMAND(target,name,data) \
    namespace detail { \
    struct name##Tag { static std::string Name() { return #name; } }; \
    typedef TargetedCommandPrototype<target, data, data > name##Prototype; \
    } \
    typedef detail::TargetedCommand<target, detail::data > name; \
     

namespace psi {
namespace control {

class Command {
public:
    virtual void Execute() const = 0;
    virtual ~Command() {}
};

class incorrect_command_exception : public psi::exception {
public:
    incorrect_command_exception(const std::string& header, const std::string& message) : exception(header, message) {}
    virtual ~incorrect_command_exception() throw() {}
};

template<typename Target>
class CommandProvider {
};

namespace commands {
namespace detail {

template<typename _Target, typename _Data>
class TargetedCommand : public Command {
public:
    typedef _Target Target;
    typedef _Data Data;

public:
    TargetedCommand(Target& aTarget, const Data& aData) : target(aTarget), data(aData) {}
    virtual void Execute() const {
        target.Execute(*this);
    }
    const Data& getData() const {
        return data;
    }

private:
    Target& target;
    Data data;
};

template<typename Data>
class CommandWithoutParametersParser {
public:
    static Data Parse(const std::vector<std::string>& commandLineParameters) {
        return Data();
    }
};

template<typename _Target>
class TargetedCommandPrototypeBase {
public:
    typedef _Target Target;

    virtual boost::shared_ptr<Command> Create(Target& target,
            const std::vector<std::string>& commandLineArguments) const = 0;
    virtual ~TargetedCommandPrototypeBase() {}
};

template<typename _Target, typename _Data, typename _Parser = CommandWithoutParametersParser<_Data> >
class TargetedCommandPrototype : public TargetedCommandPrototypeBase<_Target> {
public:
    typedef _Target Target;
    typedef _Data Data;
    typedef _Parser Parser;
    typedef TargetedCommand<Target, Data> CommandType;

    virtual boost::shared_ptr<Command> Create(Target& target,
            const std::vector<std::string>& commandLineArguments) const {
        const Data data = Parser::Parse(commandLineArguments);
        return boost::shared_ptr<Command>(new CommandType(target, data));
    }
};

template<typename _Target>
struct CommandDescriptor {
    typedef _Target Target;
    typedef TargetedCommandPrototypeBase<Target> CommandPrototype;

    boost::shared_ptr<CommandPrototype> prototype;
    std::string short_help;
    std::string long_help;

    CommandDescriptor() {}
    CommandDescriptor(CommandPrototype* _prototype, const std::string& _short_help,
                      const std::string& _long_help)
        : prototype(_prototype), short_help(_short_help), long_help(_long_help) {}
};

} // detail
} // commands
} // control
} // psi
