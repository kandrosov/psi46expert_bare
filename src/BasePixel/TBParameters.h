/*!
 * \file TBParameters.h
 * \brief Definition of TBParameters class.
 *
 * \b Changelog
 * 12-04-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Removed member - pointer to TBInterface.
 *      - TBParameters class now inherit psi::BaseConifg class.
 *      - Temporary joined with TBAnalogParameters.
 * 01-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Class SysCommand removed.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - removed deprecated conversion from string constant to char*
 */

#pragma once

#include "BaseConfig.h"

class TBAnalogInterface;

/*!
 * \brief The class represents the settings of a testboard
 */
class TBParameters : public psi::BaseConfig {
public:
    enum Register {
        clk = 8, sda = 9, ctr = 10, tin = 11, rda = 12, trc = 17, tcc = 18, tct = 19, ttk = 20, trep = 21, cc = 22,
        spd = 77 // dummy register for clock frequency
    };

public:
    void Apply(TBAnalogInterface& tbInterface);
    void Set(TBAnalogInterface& tbInterface, Register reg, int value);
    bool Get(Register reg, int& value);

private:
    typedef std::map<Register, std::string> RegMap;
    static const RegMap& Registers();
};
