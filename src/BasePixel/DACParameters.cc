/*!
 * \file DACParameters.cc
 * \brief Implementation of DACParameters class.
 */

#include <fstream>
#include "BasePixel/DACParameters.h"
#include "psi/exception.h"
#include "psi46expert/TestRoc.h"
#include "BasePixel/CalibrationTable.h"

const DACParameters::DescriptorMap& DACParameters::Descriptors()
{
    static DescriptorMap m;
    if(!m.size()) {
        m[Vdig] = Descriptor("Vdig");
        m[Vana] = Descriptor("Vana", 4000);
        m[Vsf] = Descriptor("Vsf");
        m[Vcomp] = Descriptor("Vcomp");
        m[Vleak_comp] = Descriptor("Vleak_comp");
        m[VrgPr] = Descriptor("VrgPr");
        m[VwllPr] = Descriptor("VwllPr");
        m[VrgSh] = Descriptor("VrgSh");
        m[VwllSh] = Descriptor("VwllSh");
        m[VhldDel] = Descriptor("VhldDel");
        m[Vtrim] = Descriptor("Vtrim", 6000);
        m[VthrComp] = Descriptor("VthrComp", 3000);
        m[VIBias_Bus] = Descriptor("VIBias_Bus");
        m[Vbias_sf] = Descriptor("Vbias_sf");
        m[VoffsetOp] = Descriptor("VoffsetOp");
        m[VIbiasOp] = Descriptor("VIbiasOp");
        m[VOffsetR0] = Descriptor("VOffsetR0");
        m[VIon] = Descriptor("VIon");
        m[VIbias_PH] = Descriptor("VIbias_PH");
        m[Ibias_DAC] = Descriptor("Ibias_DAC");
        m[VIbias_roc] = Descriptor("VIbias_roc");
        m[VIColOr] = Descriptor("VIColOr");
        m[Vnpix] = Descriptor("Vnpix");
        m[VSumCol] = Descriptor("VSumCol");
        m[Vcal] = Descriptor("Vcal", 3000, true);
        m[CalDel] = Descriptor("CalDel");
        m[RangeTemp] = Descriptor("RangeTemp");
        m[CtrlReg] = Descriptor("CtrlReg");
        m[WBC] = Descriptor("WBC", 3000, false, true);
    }
    return m;
}

const DACParameters::Descriptor& DACParameters::FindDescriptor(Register reg)
{
    const DescriptorMap::const_iterator iter = Descriptors().find(reg);
    if(iter == Descriptors().end())
        THROW_PSI_EXCEPTION("Unknown DAC register = " << reg << ".");
    return iter->second;
}

const std::string& DACParameters::GetRegisterName(Register reg)
{
    const Descriptor& d = FindDescriptor(reg);
    return d.name;
}

void DACParameters::Apply(TestRoc& roc, bool correction)
{
    for(DescriptorMap::const_iterator iter = Descriptors().begin(); iter != Descriptors().end(); ++iter) {
        int value = 0;
        if(BaseConfig::Get(iter->second.name, value))
            Set(roc, iter->first, value, correction);
    }
}

void DACParameters::Set(TestRoc& roc, Register reg, int value, bool correction)
{
    const Descriptor& d = FindDescriptor(reg);
    const int valueToSet = d.hasCalibrationTable && correction ? CalibrationTable::CorrectedVcalDAC(value) : value;
    BaseConfig::Set(d.name, valueToSet);
    roc.RocSetDAC(reg, valueToSet);
    roc.CDelay(d.delay);
    if(d.resetRequired)
        roc.SendReset(); //        roc.GetTBAnalogInterface()->Single(0x08); //send a reset to set a DAC
    psi::LogDebug("DACParameters") << "Register " << reg << " is set to " << valueToSet << ".\n";
}

int DACParameters::Get(Register reg) const
{
    const Descriptor& d = FindDescriptor(reg);
    int value;
    if(!BaseConfig::Get(d.name, value))
        THROW_PSI_EXCEPTION("Register " << reg << "is not configured.");
    return value;
}

void DACParameters::Read(const std::string& fileName)
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

void DACParameters::Write(const std::string& fileName) const
{
    std::ofstream f(fileName.c_str());
    if(!f.is_open())
        THROW_PSI_EXCEPTION("Unable to write the configuration into the file '" << fileName << "'.");
    for(DescriptorMap::const_iterator iter = Descriptors().begin(); iter != Descriptors().end(); ++iter) {
        const int value = Get(iter->first);
        f << iter->first << " " << iter->second.name << " " << value << std::endl;
    }
}

std::istream& operator>>(std::istream& s, DACParameters::Register& reg)
{
    int i;
    s >> i;
    reg = (DACParameters::Register) i;
    DACParameters::GetRegisterName(reg);
    return s;
}
