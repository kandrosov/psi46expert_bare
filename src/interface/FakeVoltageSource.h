/*!
 * \file FakeVoltageSource.h
 * \brief Definition of FakeVoltageSource class.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 06-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#pragma once

#include "IVoltageSource.h"
#include "psi/log.h"

namespace psi
{
class FakeVoltageSource : public psi::IVoltageSource
{
    /// \copydoc IVoltageSource::Set
    virtual Value Set(const Value& value)
    {
        LogDebug("FakeVoltageSource") << "Set value: " << value << "." << std::endl;
        return value;
    }

    /// \copydoc IVoltageSource::Accuracy
    virtual ElectricPotential Accuracy(const ElectricPotential& voltage)
    {
        static const ElectricPotential accuracy = 0.1 * psi::volts;
        LogDebug("FakeVoltageSource") << "Return accuracy = " << accuracy << " for " << voltage << "." << std::endl;
        return accuracy;
    }

    /// \copydoc IVoltageSource::Measure
    virtual IVoltageSource::Measurement Measure()
    {
        static const IVoltageSource::Measurement measurement(0.0 * psi::amperes, 0.0 * psi::volts, false);
        LogDebug("FakeVoltageSource") << "Return measurement: " << measurement << "." << std::endl;
        return measurement;
    }

    /// \copydoc IVoltageSource::Off
    virtual void Off()
    {
        LogDebug("FakeVoltageSource") << "Turn off." << std::endl;
    }
};

} // psi
