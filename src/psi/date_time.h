/*!
 * \file date_time.h
 * \brief Definition of the time-related functionality for the psi namespace.
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 */

#pragma once

#include <boost/date_time/posix_time/posix_time_duration.hpp>

#include "units.h"

namespace psi {
extern boost::posix_time::microseconds TimeToPosixTime(const psi::Time& time);
extern void Sleep(const psi::Time& time);
}
