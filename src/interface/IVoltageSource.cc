/*!
 * \file IVoltageSource.cc
 * \brief Implementation of IVoltageSource related functionality.
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
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
