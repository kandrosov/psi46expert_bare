/*!
 * \file date_time.h
 * \brief Definition of the time-related functionality for the psi namespace.
 *
 * \b Changelog
 * 25-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Function TimeToPosixTime moved from PsiCommon.cc to PsiCommon.h.
 */

#pragma once

#include <boost/date_time.hpp>

#include "units.h"

namespace psi {
extern boost::posix_time::microseconds TimeToPosixTime(const psi::Time& time);
extern void Sleep(const psi::Time& time);
}
