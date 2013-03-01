/*!
 * \file Keithley6487.h
 * \brief Definition of Keithley6487 class.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 25-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Added method Accuracy.
 *      - IVoltageSource and Keithley6487 moved into psi namespace.
 *      - Switched to ElectricPotential, ElectricCurrent and Time defined in PsiCommon.h.
 * 10-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - IVoltageSource interface was changed.
 * 28-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Destructor now returns Keithley to the default conditions and switches it to the local mode.
 *      - Added methods Off, Send, Read, ReadString and LastOperationIsCompleted.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#pragma once

#include <string>
#include <boost/shared_ptr.hpp>
#include "IVoltageSource.h"
#include "serialstream.h"

namespace psi {
/*!
 * \brief To control the Keithley 6487 high voltage source over RS232
 */
class Keithley6487 : public IVoltageSource
{
public:
    /// Maximal voltage that can be set on the Keithley 6487.
    static const double MAX_VOLTAGE;

    /// The accuracy of the Keithley.
    static const ElectricPotential ACCURACY;

    /*!
     * \brief Measurement result container.
     */
    struct Measurement
    {
        /// Current in Amperes.
        ElectricCurrent Current;

        /// Voltage in Volts.
        ElectricPotential Voltage;

        /// Indicates if device is in compliance mode.
        bool Compliance;

        /// Default constructor.
        Measurement() : Compliance(false) {}

        /// Constructor.
        Measurement(ElectricCurrent current, ElectricPotential voltage, bool compliance)
            : Current(current), Voltage(voltage), Compliance(compliance) {}
    };

public:
    /*!
     * \brief Keithley6487 constructor
     * Attention! All RS232 parameters should be synchronized with Keithley 6487 device configuration.
     * Please set line terminator on the Keithley RS232 configuration to LR.
     * \param deviceName - name of the device for which Keithley connected
     * \param baudrate - baudrate of the RS232 interface
     * \param flowControl - RS232 flow control. Keithley 6487 supports two modes: no control or software flow control
     * \param parity - RS232 parity check
     * \param characterSize - size of the character (can be 7 or 8 bit).
     */
    Keithley6487(const std::string& deviceName, unsigned baudrate = 9600,
                 SerialOptions::FlowControl flowControl = SerialOptions::noflow,
                 SerialOptions::Parity parity = SerialOptions::noparity,
                 unsigned char characterSize = 8);

    /*!
     * \brief Keithley6487 destructor.
     * It returns Keithley to the default conditions and switches it to the local mode.
     */
    virtual ~Keithley6487();

    /// \copydoc IHighVoltageSource::Set
    virtual Value Set(const Value& value);

    /// \copydoc IVoltageSource::Accuracy
    virtual ElectricPotential Accuracy(const ElectricPotential& voltage);

    /// \copydoc IHighVoltageSource::Measure
    virtual IVoltageSource::Measurement Measure();

    /// \copydoc IHighVoltageSource::Off
    virtual void Off();

private:
    /*!
     * \brief Send a command to the Keithley.
     * \param command - a command string to send
     */
    inline void Send(const std::string& command)
    {
        (*serialStream) << command << std::endl;
    }

    /*!
     * \brief Send a command to the Keithley with one argument.
     * \param command - a command name
     * \param argument - a command argument
     */
    template<typename Argument>
    void Send(const std::string& command, const Argument& argument)
    {
        (*serialStream) << command << " " << argument << std::endl;
    }

    /*!
     * \brief Read a quantity from the Keithley.
     * \return readed quantity
     */
    template<typename Result>
    Result Read()
    {
        std::string s = ReadString();
        std::stringstream s_stream;
        s_stream << s;
        Result r;
        s_stream >> r;
        return r;
    }

    /*!
     * \brief Read a string from the keithley
     * \return readed string
     */
    std::string ReadString();

    /*!
     * \brief Check if last operation is completed
     * \return true when operation is successfully completed; false - otherwise.
     */
    bool LastOperationIsCompleted();

private:
    /// A pointer to the object that provides stream access to the serial port.
    boost::shared_ptr<SerialStream> serialStream;
};

}

std::istream& operator >>(std::istream& s, psi::Keithley6487::Measurement& m);
