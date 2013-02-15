/*!
 * \file GlobalConstants.h
 * \brief Provides a minimal set of basic constants and type definitions of the project.
 *
 * \b Changelog
 * 15-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Added type definitions to represent physical values using boost::units::quantity.
 */

#pragma once

#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/electric_potential.hpp>
#include <boost/units/systems/si/current.hpp>
#include <boost/units/cmath.hpp>
#include <boost/units/systems/si/io.hpp>
#include <boost/mpl/divides.hpp>

namespace psi {
  const int COLS       = 52;
  const int ROWS       = 80;
  const int DCOLS      = 26;
  const int DCOLROWS   = 160;
  const int MODULEROCS = 16;
  const int DAC8       = 256; // Max 8-bit dacs
  const int DAC4       = 128; // Max 4-bit dacs

  using boost::units::si::amperes;
  using boost::units::si::volts;
  using boost::units::si::seconds;

  /// Type definition for the electric potential.
  typedef boost::units::quantity<boost::units::si::electric_potential> ElectricPotential;

  /// Type definition for the electric current.
  typedef boost::units::quantity<boost::units::si::current> ElectricCurrent;

  /// Type definition for the time.
  typedef boost::units::quantity<boost::units::si::time> Time;

  typedef boost::mpl::divides<boost::units::current_dimension, boost::units::time_dimension>::type
                                                                                                current_per_time_type;
  typedef boost::units::unit<current_per_time_type, boost::units::si::system> current_per_time;
  typedef boost::units::quantity<current_per_time> CurrentPerTime;

}

#define DCOLNUMPIX 160 // # pixels in a double column
#define ROCNUMROWS  80  // # rows
#define ROCNUMCOLS  52  // # columns
#define ROCNUMDCOLS 26  // # double columns (= columns/2)
#define MODULENUMROCS 16 // # max. number of rocs on a module
#define CONTROLNETWORKNUMMODULES 16 // # max. number of modules in a detector setup
#define FIFOSIZE 4096 // size of the fifo buffer on the analog testboard

static const char *const red = "\033[0;31m";
static const char *const blue = "\033[0;35m";
static const char *const purple = "\033[0;36m";
static const char *const normal = "\033[0m";
