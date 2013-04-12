/*!
 * \file TBParameters.cc
 * \brief Implementation of TBParameters class.
 *
 * \b Changelog
 * 12-04-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Removed member - pointer to TBInterface.
 *      - TBParameters class now inherit psi::BaseConifg class.
 *      - Temporary joined with TBAnalogParameters.
 * 01-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using a new PSI Logging System.
 *      - Class SysCommand removed.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - removed deprecated conversion from string constant to char*
 */

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
    const RegMap::const_iterator iter = Registers().find(reg);
    if(iter == Registers().end())
        THROW_PSI_EXCEPTION("Unknown TB register = " << reg << ".");

    BaseConfig::Set(iter->second, value);
    if (reg == spd) tbInterface.SetClock(value);
    else if (reg > 15)
        tbInterface.Set(reg, value);
    else
        tbInterface.SetDelay(reg, value);
}

bool TBParameters::Get(Register reg, int& value)
{
    const RegMap::const_iterator iter = Registers().find(reg);
    if(iter == Registers().end())
        THROW_PSI_EXCEPTION("Unknown TB register = " << reg << ".");

    return BaseConfig::Get(iter->second, value);
}
