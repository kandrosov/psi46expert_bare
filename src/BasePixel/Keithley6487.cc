/*!
 * \file Keithley6487.cc
 * \brief Implementation of Keithley6487 class.
 *
 * \b Changelog
 * 28-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Added verification of Keithley ID and verification of module change.
 *      - Destructor now returns Keithley to the default conditions and switches it to the local mode.
 *      - Added methods Off, Send, Read, ReadString and LastOperationIsCompleted.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#include "Keithley6487.h"
#include "psi_exception.h"

#include <iostream>

static const unsigned DEFAULT_TIMEOUT = 3;
static const unsigned OPERATION_IS_COMPLETE_INDICATOR = 1;
static const unsigned MAX_VOLTAGE_RANGE = 500; // V
static const std::string MAX_CURRENT_LIMIT = "2.5e-3"; // A
static const std::string IDENTIFICATION_STRING_PREFIX = "KEITHLEY INSTRUMENTS INC.,MODEL 6487";

const double Keithley6487::MAX_VOLTAGE = 500;

Keithley6487::Keithley6487(const std::string& deviceName, unsigned baudrate, SerialOptions::FlowControl flowControl,
                           SerialOptions::Parity parity, unsigned char characterSize)
{
    SerialOptions options;
    options.setDevice(deviceName);
    options.setTimeout(boost::posix_time::seconds(DEFAULT_TIMEOUT));

    options.setBaudrate(baudrate);
    options.setFlowControl(flowControl);
    options.setParity(parity);
    options.setCsize(characterSize);

    try
    {
        serialStream = boost::shared_ptr<SerialStream>(new SerialStream(options));
        serialStream->exceptions(std::ios::badbit | std::ios::failbit);
        Send("*RST");
        Send("*IDN?");
        const std::string identificationString = ReadString();
        if(identificationString.find(IDENTIFICATION_STRING_PREFIX) != 0)
            THROW_PSI_EXCEPTION("[Keithley6487::Keithley6487] ERROR: Device connected to '" << deviceName
                                << "' is not supported. Device identified it self as '"
                                << identificationString << "'.");
        Send("FUNC 'CURR'");
        Send("FORM:ELEM READ,VSO");
    }
    catch(TimeoutException&)
    {
        THROW_PSI_EXCEPTION("[Keithley6487::Keithley6487] ERROR: Unable to connect to the Keithley on '"
                            << deviceName << "'.");
    }
    catch(std::ios_base::failure&)
    {
        THROW_PSI_EXCEPTION("[Keithley6487::Keithley6487] ERROR: Unable to connect to the Keithley on '"
                            << deviceName << "'.");
    }
}

Keithley6487::~Keithley6487()
{
    Send("*RST");
    Send("SYST:LOC");
}

void Keithley6487::Set(double voltage)
{
    if(voltage > MAX_VOLTAGE)
        THROW_PSI_EXCEPTION("[Keithley6487::Set] ERROR: voltage " << voltage << " V is too high."
                            << "Maximal allowed voltage is " << MAX_VOLTAGE << " V.");

    try
    {
        Send("SOUR:VOLT:RANG", MAX_VOLTAGE_RANGE);
        Send("SOUR:VOLT", voltage);
        Send("SOUR:VOLT:ILIM", MAX_CURRENT_LIMIT);
        Send("SOUR:VOLT:STAT ON");

        if(!LastOperationIsCompleted())
            THROW_PSI_EXCEPTION("[Keithley6487::Set] ERROR: Voltage was not set.");
    }
    catch(TimeoutException&)
    {
        THROW_PSI_EXCEPTION("[Keithley6487::Set] ERROR: Unable to connect to the Keithley to set a voltage.");
    }
    catch(std::ios_base::failure&)
    {
        THROW_PSI_EXCEPTION("[Keithley6487::Set] ERROR: Unable to connect to the Keithley to set a voltage.");
    }
}

IVoltageSource::Measurement Keithley6487::Measure()
{
    Send("READ?");
    return Read<Measurement>();
}

void Keithley6487::Off()
{
    Send("SOUR:VOLT:STAT OFF");
    if(!LastOperationIsCompleted())
        THROW_PSI_EXCEPTION("[Keithley6487::Off] ERROR: Voltage is not turned off.");
}

std::string Keithley6487::ReadString()
{
    std::string s;
    std::getline(*serialStream, s);
    return s;
}

bool Keithley6487::LastOperationIsCompleted()
{
    Send("*OPC?");
    const unsigned operationStatus = Read<unsigned>();
    return operationStatus == OPERATION_IS_COMPLETE_INDICATOR;
}

std::istream& operator >>(std::istream& s, Keithley6487::Measurement& m)
{
    char c;
    s >> m.Current;
    s >> c;
    if(c != ',')
        THROW_PSI_EXCEPTION("[Keithley6487::Measurement] ERROR: Keithley replay has an incorrect format.");
    s >> m.Voltage;
}
