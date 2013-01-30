/*!
 * \file IVoltageSource.h
 * \brief Definition of IVoltageSource interface.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 28-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Removed delay parameter from Set method.
 *      - Added method Off.
 *      - Measure now returns a dedicated structure.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#pragma once

/*!
 * \brief Interface to operate with voltage source that is also capable to measure set voltage and current.
 */
class IVoltageSource
{
public:
    /*!
     * \brief Measurement result container.
     */
    struct Measurement
    {
        /// Current in Ampere.
        double Current;

        /// Voltage in Volt.
        double Voltage;

        /// Default constructor.
        Measurement() : Current(0), Voltage(0) {}

        /// Constructor.
        Measurement(double current, double voltage) : Current(current), Voltage(voltage) {}
    };

public:
    /*!
     * \brief Set voltage on the source.
     * \param voltage - voltage to set in Volt
     */
    virtual void Set(double voltage) = 0;

    /*!
     * \brief Perform measurement of voltage and current.
     * \return Measuremnet result.
     */
    virtual Measurement Measure() = 0;

    /*!
     * \brief Turn the voltage off.
     */
    virtual void Off() = 0;

    /*!
     * \brief IHighVoltageSource virtual destructor
     */
    virtual ~IVoltageSource() {}
};
