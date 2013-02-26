/*!
 * \file Keithley.h
 * \brief Definition of Keithley class.
 *
 * \b Changelog
 * 25-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Added method Accuracy.
 *      - IVoltageSource and Keithley moved into psi namespace.
 *      - Switched to ElectricPotential, ElectricCurrent and Time defined in PsiCommon.h.
 * 10-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - IVoltageSource interface was changed.
 * 30-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now Keithley inherits IVoltageSource interface.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now Keithley inherits IHighVoltageSource interface.
 */

#pragma once

#include "IVoltageSource.h"

namespace psi {
/*!
 * \brief To control the Keithley high voltage source (version is unknown!)
 */
class Keithley : public IVoltageSource
{

public:
	Keithley();
    virtual ~Keithley();

    /// \copydoc IVoltageSource::Set
    virtual Value Set(const Value& value);

    /// \copydoc IVoltageSource::Accuracy
    virtual ElectricPotential Accuracy(const ElectricPotential& voltage);

    /// \copydoc IVoltageSource::Measure
    virtual IVoltageSource::Measurement Measure();

    /// \copydoc IVoltageSource::Off
    virtual void Off();

private:
    void Open();
    void Init();
    void Write(const char *string);
    void Read();
    void Measure(int targetVoltage, double &voltage, double &current, int delay);
    void Command(char *commandString);
    void GoLocal();
    void ShutDown();
    void ReadCurrent(double &voltage, double &current);
    int  Tripped();

private:
	int port;
};

}
