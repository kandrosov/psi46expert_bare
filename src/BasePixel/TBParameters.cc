/*!
 * \file TBParameters.cc
 * \brief Implementation of TBParameters class.
 */

#include <fstream>

#include "BasePixel/TBParameters.h"
#include "BasePixel/TBAnalogInterface.h"
#include "psi/exception.h"

const TBParameters::RegMap& TBParameters::Registers()
{
    static RegMap m;
    if(!m.size()) {
        m[clk] = "clk";
        m[sda] = "sda";
        m[ctr] = "ctr";
        m[tin] = "tin";
        m[rda] = "rda";
        m[trc] = "trc";
        m[tcc] = "tcc";
        m[tct] = "tct";
        m[ttk] = "ttk";
        m[trep] = "trep";
        m[cc] = "cc";
        m[spd] = "spd";
    }
    return m;
}

const std::string& TBParameters::GetRegisterName(Register reg)
{
    const RegMap::const_iterator iter = Registers().find(reg);
    if(iter == Registers().end())
        THROW_PSI_EXCEPTION("Unknown TB register = " << reg << ".");
    return iter->second;
}

void TBParameters::Apply(TBAnalogInterface& tbInterface)
{
    for(RegMap::const_iterator iter = Registers().begin(); iter != Registers().end(); ++iter) {
        int value = 0;
        if(BaseConfig::Get(iter->second, value))
            Set(tbInterface, iter->first, value);
    }
}

void TBParameters::Set(TBAnalogInterface& tbInterface, Register reg, int value)
{
    const std::string& regName = GetRegisterName(reg);
    BaseConfig::Set(regName, value);
    if (reg == spd) tbInterface.SetClock(value);
    else if (reg > 15)
        tbInterface.Set(reg, value);
    else
        tbInterface.SetDelay(reg, value);
}

bool TBParameters::Get(Register reg, int& value) const
{
    const std::string& regName = GetRegisterName(reg);
    return BaseConfig::Get(regName, value);
}

void TBParameters::Read(const std::string& fileName)
{
    std::ifstream f(fileName.c_str());
    if(!f.is_open())
        THROW_PSI_EXCEPTION("Unable to read the configuration file '" << fileName << "'.");
    while(f.good()) {
        std::string line;
        std::getline(f, line);
        if(!line.length() || line[0] == '#' || line[0] == '-')
            continue;

        std::istringstream istring( line);

        Register reg;
        std::string name;
        int value;

        istring >> reg >> name >> value;

        if(istring.fail() || !name.length())
            continue;
        const std::string expectedName = GetRegisterName(reg);
        if(name != expectedName)
            THROW_PSI_EXCEPTION("Configuration file contains invalid name '" << name << "' for register " << reg << "."
                                << " Expected name is '" << expectedName << "'.");
        BaseConfig::Set(name, value);
    }
}

void TBParameters::Write(const std::string& fileName) const
{
    std::ofstream f(fileName.c_str());
    if(!f.is_open())
        THROW_PSI_EXCEPTION("Unable to write the configuration into the file '" << fileName << "'.");
    for(RegMap::const_iterator iter = Registers().begin(); iter != Registers().end(); ++iter) {
        int value;
        if(Get(iter->first, value))
            f << iter->first << " " << iter->second << " " << value << std::endl;
    }
}

std::istream& operator>>(std::istream& s, TBParameters::Register& reg)
{
    int i;
    s >> i;
    reg = (TBParameters::Register) i;
    TBParameters::GetRegisterName(reg);
    return s;
}
