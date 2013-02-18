/*!
 * \file IVoltageSource.h
 * \brief Definition of IVoltageSource interface.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 18-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Added operator << for IVoltageSource::Measurement class.
 * 07-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - All physical values now represented using boost::units::quantity.
 * 31-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Changed Set method signature. Now it takes IVoltageSource::Value as parameter and returns value that was
 *        really set on the device.
 * 28-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Removed delay parameter from Set method.
 *      - Added method Off.
 *      - Measure now returns a dedicated structure.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#pragma once

#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/electric_potential.hpp>
#include <boost/units/systems/si/current.hpp>
#include <boost/units/cmath.hpp>
#include <boost/units/systems/si/io.hpp>

/*!
 * \brief Interface to operate with voltage source that is also capable to measure set voltage and current.
 */
class IVoltageSource
{
public:
    /// Type definition for the electric potential.
    typedef boost::units::quantity<boost::units::si::electric_potential> ElectricPotential;

    /// Type definition for the electric current.
    typedef boost::units::quantity<boost::units::si::current> ElectricCurrent;

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
        Measurement() : Current(0*boost::units::si::amperes), Voltage(0*boost::units::si::volts), Compliance(false) {}

        /// Constructor.
        Measurement(ElectricCurrent current, ElectricPotential voltage, bool compliance)
            : Current(current), Voltage(voltage), Compliance(compliance) {}
    };

    /*!
     * \brief Represents a value with setup parameters that can be generated by the IVoltageSource
     */
    struct Value
    {
        /// Voltage in Volts.
        ElectricPotential Voltage;

        /// Compliance value in Amperes.
        ElectricCurrent Compliance;

        /// Default constructor.
        Value() : Voltage(0*boost::units::si::volts), Compliance(0*boost::units::si::amperes) {}

        /// Constructor.
        Value(ElectricPotential voltage, ElectricCurrent compliance) : Voltage(voltage), Compliance(compliance) {}
    };

public:
    /*!
     * \brief Set voltage on the source using default compliance value.
     * \param value - desired voltage and setup parameters
     * \return Voltage and setup parameters that were really set on the device. It can be different from 'value' \
     *         due to the device limitations.
     */
    virtual Value Set(const Value& value) = 0;

    /*!
     * \brief Perform measurement of voltage and current.
     * \return Measuremnet result.
     */
    virtual Measurement Measure() = 0;

    /// Turn the voltage off.
    virtual void Off() = 0;

    /// IHighVoltageSource virtual destructor
    virtual ~IVoltageSource() {}
};

static std::ostream& operator << (std::ostream& s, const IVoltageSource::Measurement& m)
{
    s << "Voltage = " << m.Voltage << ", Current = " << m.Current << ", In compliance = "
      << std::boolalpha << m.Compliance << ".";
    return s;
}
