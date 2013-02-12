/*!
 * \file BaseConfig.h
 * \brief Definition of BaseConfig class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#pragma once

#include <map>
#include <string>
#include <sstream>
#include "interface/Log.h"

#define CONFIG_PARAMETER(type, name, default_value) \
    type name() const { \
        type result = default_value; \
        if(!Get(#name, result)) \
            psi::LogInfo() << "[BaseConfig] Warning! Parameter '" << #name << "' is not set. Using default value = '" \
                           << default_value << "'."; \
        return result; \
    } \
    void set##name(const type& value) { Set(#name, value); }

namespace psi {

class BaseConfig
{
private:
    typedef std::map<std::string, std::string> Map;
public:
    virtual ~BaseConfig() {}
    virtual void Read(const std::string& fileName);
    virtual void Write(const std::string& fileName) const;

protected:
    template<typename Value>
    bool Get(const std::string& name, Value& value) const
    {
        const Map::const_iterator iter = parameters.find(name);
        if(iter == parameters.end())
            return false;
        std::istringstream s(iter->second);
        s >> value;
        return !s.fail();
    }

    template<typename Value>
    void Set(const std::string& name, const Value& value)
    {
        std::ostringstream s;
        s << value;
        parameters[name] = s.str();
    }

private:
    Map parameters;
};

}
