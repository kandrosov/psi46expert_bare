/*!
 * \file TBMParameters.h
 * \brief Definition of TBMParameters class.
 */

#pragma once

#include "BaseConfig.h"

class TBM;

/*!
 * \brief The class represents the settings of a token bit manager (TBM)
 */
class TBMParameters : public psi::BaseConfig {
public:
    enum Register {
        Single = 0, Speed = 1, Inputbias = 2, Outputbias = 3, Dacgain = 4, Triggers = 5, Mode = 6
    };

    static const std::string& GetRegisterName(Register reg);

public:
    /// Sets all the current DAC parameters.
    void Apply(TBM& tbm) const;

    void Set(TBM& tbm, Register reg, int value);
    bool Get(Register reg, int& value) const;

    virtual void Read(const std::string& fileName);
    virtual void Write(const std::string& fileName) const;

protected:
    typedef void (*Action)(TBM& tbm, int value);
    struct Descriptor {
        std::string name;
        Action action;

        Descriptor() : name(""), action(0) {}
        Descriptor(const std::string& aName, Action anAction)
            : name(aName), action(anAction) {}
    };

    typedef std::map<Register, Descriptor> DescriptorMap;
    static const DescriptorMap& Descriptors();
    static const Descriptor& FindDescriptor(Register reg);
};

extern std::istream& operator>>(std::istream& s, TBMParameters::Register& reg);
