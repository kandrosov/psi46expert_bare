/*!
 * \file TBMParameters.cc
 * \brief Implementation of TBMParameters class.
 *
 * \b Changelog
 * 13-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Removed member - pointer to TBM.
 *      - TBMParameters class now inherit psi::BaseConifg class.
 * 01-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using a new PSI Logging System.
 *      - Class SysCommand removed.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - removed deprecated conversion from string constant to char*
 */

#include "BasePixel/TBMParameters.h"
#include "BasePixel/TBM.h"
#include "psi/log.h"
#include "psi/exception.h"

static const std::string LOG_HEAD = "TBMParameters";

void TBMParameters::Apply(TBM& tbm) const
{
    for(DescriptorVector::const_iterator iter = Descriptors().begin(); iter != Descriptors().end(); ++iter) {
        int value = 0;
        if(BaseConfig::Get(iter->name, value))
            iter->action(tbm, value);
    }
}

void TBMParameters::Set(TBM& tbm, unsigned reg, int value)
{
    if(reg >= Descriptors().size())
        THROW_PSI_EXCEPTION("Unknown TBP register = " << reg << ".");

    const Descriptor& d = Descriptors().at(reg);
    BaseConfig::Set(d.name, value);
    d.action(tbm, value);
}

bool TBMParameters::Get(unsigned reg, int& value) const
{
    if(reg >= Descriptors().size())
        THROW_PSI_EXCEPTION("Unknown TBP register = " << reg << ".");

    return BaseConfig::Get(Descriptors().at(reg).name, value);
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

const TBMParameters::DescriptorVector& TBMParameters::Descriptors()
{
    static const unsigned NTBMParameters = 7;
    static DescriptorVector d;
    if(!d.size()) {
        d.assign(NTBMParameters, Descriptor());
        d[0] = Descriptor("Single",  &SetSingle);
        d[1] = Descriptor("Speed",  &SetSpeed);
        d[2] = Descriptor("Inputbias",  &SetInputBias);
        d[3] = Descriptor("Outputbias",  &SetOutputBias);
        d[4] = Descriptor("Dacgain",  &SetDacGain);
        d[5] = Descriptor("Triggers",  &SetTriggers);
        d[6] = Descriptor("Mode",  &SetMode);
    }
    return d;
}
