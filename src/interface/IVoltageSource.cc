/*!
 * \file IVoltageSource.cc
 * \brief Implementation of IVoltageSource related functionality.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 09-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 *      - std::ostrem output operators moved here from the header file.
 */

#include "IVoltageSource.h"

std::ostream& operator << (std::ostream& s, const psi::IVoltageSource::Measurement& m)
{
    s << "Voltage = " << m.Voltage << ", Current = " << m.Current << ", In compliance = "
      << std::boolalpha << m.Compliance;
    return s;
}

std::ostream& operator << (std::ostream& s, const psi::IVoltageSource::Value& v)
{
    s << "Voltage = " << v.Voltage << ", Compliance = " << v.Compliance;
    return s;
}
