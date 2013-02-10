/*!
 * \file Keithley.h
 * \brief Definition of Keithley class.
 *
 * \b Changelog
 * 10-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - IVoltageSource interface was changed.
 * 30-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now Keithley inherits IVoltageSource interface.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now Keithley inherits IHighVoltageSource interface.
 */

#ifndef KEITHLEY
#define KEITHLEY

#include "IVoltageSource.h"

/*!
 * \brief To control the Keithley high voltage source (version is unknown!)
 */
class Keithley : public IVoltageSource
{

public:
	Keithley();
    virtual ~Keithley();

    virtual Value Set(const Value& value);
    virtual Measurement Measure();
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

#endif
