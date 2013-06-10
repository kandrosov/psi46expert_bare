/*!
 * \file TBParameters.h
 * \brief Definition of TBParameters class.
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

    static const std::string& GetRegisterName(Register reg);

public:
    void Apply(TBAnalogInterface& tbInterface);
    void Set(TBAnalogInterface& tbInterface, Register reg, int value);
    bool Get(Register reg, int& value) const;

    virtual void Read(const std::string& fileName);
    virtual void Write(const std::string& fileName) const;

private:
    typedef std::map<Register, std::string> RegMap;
    static const RegMap& Registers();
};

extern std::istream& operator>>(std::istream& s, TBParameters::Register& reg);
