/*!
 * \file units.h
 * \brief Definition of the unit system for the psi namespace.
 *
 * \b Changelog
 * 06-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Added name and symbol strings for boost::units::si::electric_potential.
 * 25-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Added some SI scale prefixes.
 */

#pragma once

#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/electric_potential.hpp>
#include <boost/units/systems/si/current.hpp>
#include <boost/units/systems/si/time.hpp>
#include <boost/units/io.hpp>

namespace psi {

using boost::units::si::amperes;
using boost::units::si::volts;
using boost::units::si::seconds;

/// Type definition for the electric potential.
typedef boost::units::quantity<boost::units::si::electric_potential> ElectricPotential;

/// Type definition for the electric current.
typedef boost::units::quantity<boost::units::si::current> ElectricCurrent;

/// Type definition for the time.
typedef boost::units::quantity<boost::units::si::time> Time;

typedef boost::mpl::divides<boost::units::current_dimension, boost::units::time_dimension>::type current_per_time_type;
typedef boost::units::unit<current_per_time_type, boost::units::si::system> current_per_time;
typedef boost::units::quantity<current_per_time> CurrentPerTime;

static const double giga = 1e9;
static const double mega = 1e6;
static const double kilo = 1e3;
static const double milli = 1e-3;
static const double micro = 1e-6;
static const double nano = 1e-9;

/// abs from boost/units/cmath.hpp
template<class Unit,class Y>
inline boost::units::quantity<Unit,Y> abs(const boost::units::quantity<Unit,Y>& q)
{
    return boost::units::quantity<Unit,Y>::from_value(std::abs BOOST_PREVENT_MACRO_SUBSTITUTION (q.value()));
}

} // psi

namespace boost {
namespace units {
/// from boost/units/systems/si/io.hpp
inline std::string name_string(const reduce_unit<si::electric_potential>::type&) { return "volt"; }

/// from boost/units/systems/si/io.hpp
inline std::string symbol_string(const reduce_unit<si::electric_potential>::type&) { return "V"; }
} // units
} // boost
