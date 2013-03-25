/*!
 * \file TBMParameters.h
 * \brief Definition of TBMParameters class.
 *
 * \b Changelog
 * 13-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Removed member - pointer to TBM.
 *      - TBMParameters class now inherit psi::BaseConifg class.
 * 01-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Class SysCommand removed.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - removed deprecated conversion from string constant to char*
 */

#pragma once

#include <vector>

#include "BaseConfig.h"

class TBM;

/*!
 * \brief The class represents the settings of a token bit manager (TBM)
 */
class TBMParameters : public psi::BaseConfig {
public:
    /// Sets all the current DAC parameters.
    void Apply(TBM& tbm) const;

    void Set(TBM& tbm, unsigned reg, int value);
    bool Get(unsigned reg, int& value) const;

protected:
    typedef void (*Action)(TBM& tbm, int value);
    struct Descriptor {
        std::string name;
        Action action;

        Descriptor() : name(""), action(0) {}
        Descriptor(const std::string& aName, Action anAction)
            : name(aName), action(anAction) {}
    };

    typedef std::vector<Descriptor> DescriptorVector;
    static const DescriptorVector& Descriptors();
};
