/*!
 * \file FakeVoltageSource.h
 * \brief Definition of FakeVoltageSource class.
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 */

#pragma once

#include "IVoltageSource.h"
#include "psi/log.h"
#include "psi/date_time.h"

namespace psi {

class FakeVoltageSource : public IVoltageSource {
public:
    FakeVoltageSource(const Resistance& resistance)
        : r(resistance), v(0.0 * volts) { }

    /// \copydoc IVoltageSource::Set
    virtual Value Set(const Value& value) {
        LogDebug("FakeVoltageSource") << "Set value: " << value << "." << std::endl;
        v = value.Voltage;
        compliance = value.Compliance;
        return value;
    }

    /// \copydoc IVoltageSource::Accuracy
    virtual ElectricPotential Accuracy(const ElectricPotential& voltage) {
        static const ElectricPotential accuracy = 0.1 * volts;
        LogDebug("FakeVoltageSource") << "Return accuracy = " << accuracy << " for " << voltage << "." << std::endl;
        return accuracy;
    }

    /// \copydoc IVoltageSource::Measure
    virtual IVoltageSource::Measurement Measure() {
        const ElectricCurrent i = v / r;
        const bool inCompliance = i >= compliance;
        const IVoltageSource::Measurement measurement(i, v, DateTimeProvider::ElapsedTime(), inCompliance);
        LogDebug("FakeVoltageSource") << "Return measurement: " << measurement << "." << std::endl;
        return measurement;
    }

    /// \copydoc IVoltageSource::Off
    virtual void Off() {
        LogDebug("FakeVoltageSource") << "Turn off." << std::endl;
    }

private:
    psi::Resistance r;
    psi::ElectricPotential v;
    psi::ElectricCurrent compliance;
};

} // psi
