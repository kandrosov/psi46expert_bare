/*!
 * \file BaseConfig.h
 * \brief Definition of BaseConfig class.
 *
 * \b Changelog
 * 01-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using a new PSI Logging System.
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using definitions from PsiCommon.h.
 * 18-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Added support to save/load psi::Time.
 * 15-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using boost::units::quantity to represent physical values.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#pragma once

#include <map>
#include <string>
#include <sstream>
#include "interface/Log.h"
#include "PsiCommon.h"

#define CONFIG_PARAMETER(type, name, default_value) \
    type name() const { \
        type result = default_value; \
        if(!Get(#name, result)) \
        psi::Log<psi::Info>("BaseConfig") << "Warning: Parameter '" << #name \
                        << "' is not set. Using default value = '" << default_value << "'." << std::endl; \
        return result; \
    } \
    void set##name(const type& value) { Set(#name, value); }

namespace psi {

namespace BaseConfigInternals
{

template<typename Value>
struct ConfigValue
{
    static bool Read(const std::string& str, Value& value)
    {
        std::istringstream s(str);
        s >> value;
        return !s.fail();
    }
};

template<>
struct ConfigValue<psi::ElectricCurrent>
{
    static const psi::ElectricCurrent& UnitsFactor()
    {
        static const psi::ElectricCurrent factor = 1.0 * psi::amperes;
        return factor;
    }

    static bool Read(const std::string& str, psi::ElectricCurrent& value)
    {
        double v;
        if(!ConfigValue<double>::Read(str, v))
            return false;
        value = v * UnitsFactor();
        return true;
    }
};

template<>
struct ConfigValue<psi::ElectricPotential>
{
    static const psi::ElectricPotential& UnitsFactor()
    {
        static const psi::ElectricPotential factor = 1.0 * psi::volts;
        return factor;
    }

    static bool Read(const std::string& str, psi::ElectricPotential& value)
    {
        double v;
        if(!ConfigValue<double>::Read(str, v))
            return false;
        value = v * UnitsFactor();
        return true;
    }
};

template<>
struct ConfigValue<psi::Time>
{
    static const psi::Time& UnitsFactor()
    {
        static const psi::Time factor = 1.0 * psi::seconds;
        return factor;
    }

    static bool Read(const std::string& str, psi::Time& value)
    {
        double v;
        if(!ConfigValue<double>::Read(str, v))
            return false;
        value = v * UnitsFactor();
        return true;
    }
};

}

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
        return BaseConfigInternals::ConfigValue<Value>::Read(iter->second, value);
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
