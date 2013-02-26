/*!
 * \file PsiCommon.h
 * \brief Provides a minimal set of constants, type definitions and functions for the project.
 *
 * \b Changelog
 * 25-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Function TimeToPosixTime moved from PsiCommon.cc to PsiCommon.h.
 *      - Added some SI scale prefixes.
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Code moved from GlobalConstants.h.
 */

#pragma once

#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/electric_potential.hpp>
#include <boost/units/systems/si/current.hpp>
#include <boost/units/cmath.hpp>
#include <boost/units/systems/si/io.hpp>
#include <boost/mpl/divides.hpp>
#include <boost/date_time.hpp>

namespace psi
{
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

boost::posix_time::microseconds TimeToPosixTime(const psi::Time& time);
void Sleep(const psi::Time& time);

static const double giga = 1e9;
static const double mega = 1e6;
static const double kilo = 1e3;
static const double milli = 1e-3;
static const double micro = 1e-6;
static const double nano = 1e-9;

static const unsigned ROCNUMROWS = 80;  // # rows
static const unsigned ROCNUMCOLS = 52;  // # columns
static const unsigned ROCNUMDCOLS = 26;  // # double columns (= columns/2)
static const unsigned MODULENUMROCS = 16; // # max. number of rocs on a module
static const unsigned CONTROLNETWORKNUMMODULES = 16; // # max. number of modules in a detector setup
static const unsigned FIFOSIZE = 4096; // size of the fifo buffer on the analog testboard
}
