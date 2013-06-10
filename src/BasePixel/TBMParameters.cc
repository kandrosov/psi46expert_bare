/*!
 * \file TBMParameters.cc
 * \brief Implementation of TBMParameters class.
 */

#include <fstream>

#include "BasePixel/TBMParameters.h"
#include "BasePixel/TBM.h"
#include "psi/log.h"
#include "psi/exception.h"

static const std::string LOG_HEAD = "TBMParameters";

void TBMParameters::Apply(TBM& tbm) const
{
    for(DescriptorMap::const_iterator iter = Descriptors().begin(); iter != Descriptors().end(); ++iter) {
        int value = 0;
        if(BaseConfig::Get(iter->second.name, value))
            iter->second.action(tbm, value);
    }
}

void TBMParameters::Set(TBM& tbm, Register reg, int value)
{
    const Descriptor& d = FindDescriptor(reg);
    BaseConfig::Set(d.name, value);
    d.action(tbm, value);
}

bool TBMParameters::Get(Register reg, int& value) const
{
    const Descriptor& d = FindDescriptor(reg);
    return BaseConfig::Get(d.name, value);
}

static void SetSingle(TBM& tbm, int value)
{
    if (value == 0) tbm.setSingleMode();
    else if (value == 1) tbm.setDualMode();
    else if (value == 2) tbm.setSingleMode2();
}

static void SetSpeed(TBM& tbm, int value)
{
    tbm.set2040Readout(value);
}

static void SetInputBias(TBM& tbm, int value)
{
    tbm.setTBMDAC(0, value);
}

static void SetOutputBias(TBM& tbm, int value)
{
    tbm.setTBMDAC(1, value);
}

static void SetDacGain(TBM& tbm, int value)
{
    tbm.setTBMDAC(2, value);
}

static void SetTriggers(TBM& tbm, int value)
{
    if (value == 0) tbm.setIgnoreTriggers(0x00);
    else if (value == 1) tbm.setIgnoreTriggers(0x10);
    else if (value == 2) tbm.setDisableTriggers(0x40);
}

static void SetMode(TBM& tbm, int value)
{
    if (value == 0) tbm.setMode(0x00);  //sync
    else if (value == 1) tbm.setMode(0xC0); //cal
    else if (value == 2) tbm.setMode(0x80); //clear
}

const std::string& TBMParameters::GetRegisterName(Register reg)
{
    const Descriptor& d = FindDescriptor(reg);
    return d.name;
}

const TBMParameters::Descriptor& TBMParameters::FindDescriptor(Register reg)
{
    const DescriptorMap::const_iterator iter = Descriptors().find(reg);
    if(iter == Descriptors().end())
        THROW_PSI_EXCEPTION("Unknown TBM register = " << reg << ".");
    return iter->second;
}

const TBMParameters::DescriptorMap& TBMParameters::Descriptors()
{
    static DescriptorMap d;
    if(!d.size()) {
        d[Single] = Descriptor("Single",  &SetSingle);
        d[Speed] = Descriptor("Speed",  &SetSpeed);
        d[Inputbias] = Descriptor("Inputbias",  &SetInputBias);
        d[Outputbias] = Descriptor("Outputbias",  &SetOutputBias);
        d[Dacgain] = Descriptor("Dacgain",  &SetDacGain);
        d[Triggers] = Descriptor("Triggers",  &SetTriggers);
        d[Mode] = Descriptor("Mode",  &SetMode);
    }
    return d;
}

void TBMParameters::Read(const std::string& fileName)
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

void TBMParameters::Write(const std::string& fileName) const
{
    std::ofstream f(fileName.c_str());
    if(!f.is_open())
        THROW_PSI_EXCEPTION("Unable to write the configuration into the file '" << fileName << "'.");
    for(DescriptorMap::const_iterator iter = Descriptors().begin(); iter != Descriptors().end(); ++iter) {
        int value;
        if(Get(iter->first, value))
            f << iter->first << " " << iter->second.name << " " << value << std::endl;
    }
}

std::istream& operator>>(std::istream& s, TBMParameters::Register& reg)
{
    int i;
    s >> i;
    reg = (TBMParameters::Register) i;
    TBMParameters::GetRegisterName(reg);
    return s;
}
