/*!
 * \file ThreadSafeVoltageSource.cc
 * \brief Implementation of ThreadSafeVoltageSource class.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 25-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Added method GradualSet.
 *      - IVoltageSource and ThreadSafeVoltageSource moved into psi namespace.
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#include "ThreadSafeVoltageSource.h"
#include "psi/exception.h"
#include "psi/date_time.h"

psi::ThreadSafeVoltageSource::ThreadSafeVoltageSource(IVoltageSource* aVoltageSource)
    : voltageSource(aVoltageSource), isOn(false)
{
    if(!aVoltageSource)
        THROW_PSI_EXCEPTION("Voltage source can't be null.")
}

psi::IVoltageSource::Value psi::ThreadSafeVoltageSource::Set(const Value& value)
{
    const boost::lock_guard<boost::recursive_mutex> lock(mutex);
    if(!isOn || currentValue != value)
    {
        currentValue = voltageSource->Set(value);
        isOn = true;
    }
    return currentValue;
}

psi::ElectricPotential psi::ThreadSafeVoltageSource::Accuracy(const psi::ElectricPotential& voltage)
{
    const boost::lock_guard<boost::recursive_mutex> lock(mutex);
    return voltageSource->Accuracy(voltage);
}

psi::IVoltageSource::Measurement psi::ThreadSafeVoltageSource::Measure()
{
    const boost::lock_guard<boost::recursive_mutex> lock(mutex);
    return voltageSource->Measure();
}

bool psi::ThreadSafeVoltageSource::GradualSet(const Value& value, const psi::ElectricPotential& step,
                                              const psi::Time& delayBetweenSteps, bool checkForCompliance)
{
    if(step <= 0.0 * psi::volts)
        THROW_PSI_EXCEPTION("Invalid voltage step = " << step << ". The voltage step should be greater then zero.");
    if(delayBetweenSteps < 0.0 * psi::seconds)
        THROW_PSI_EXCEPTION("Invalid delay between the voltage switch = " << delayBetweenSteps
                            << ". The delay should be positive or zero.");

    const boost::lock_guard<boost::recursive_mutex> lock(mutex);
    for(bool makeNextStep = true; makeNextStep;)
    {
        const psi::ElectricPotential deltaV = value.Voltage - currentValue.Voltage;
        const psi::ElectricPotential absDeltaV = psi::abs(deltaV);
        psi::ElectricPotential voltageToSet;
        if(absDeltaV < Accuracy(value.Voltage))
            break;
        if(absDeltaV < psi::abs(step))
        {
            makeNextStep = false;
            voltageToSet = value.Voltage;
        }
        else
        {
            voltageToSet = currentValue.Voltage + (deltaV > 0.0 * psi::volts ? step : -step);
        }

        Set(Value(voltageToSet, value.Compliance));
        psi::Sleep(delayBetweenSteps);

        if(checkForCompliance)
        {
            const Measurement measurement = Measure();
            if(measurement.Compliance)
                return false;
        }
    }
    return true;
}

void psi::ThreadSafeVoltageSource::Off()
{
    const boost::lock_guard<boost::recursive_mutex> lock(mutex);
    voltageSource->Off();
    isOn = false;
}

void psi::ThreadSafeVoltageSource::lock()
{
    mutex.lock();
}

void psi::ThreadSafeVoltageSource::unlock()
{
    mutex.unlock();
}
