/*!
 * \file PsiCommon.cc
 * \brief Provides a minimal set of constants, type definitions and functions for the project.
 *
 * \b Changelog
 * 25-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Function TimeToPosixTime moved from PsiCommon.cc to PsiCommon.h.
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Code moved from GlobalConstants.h.
 */

#include <boost/thread.hpp>

#include "PsiCommon.h"

namespace psi
{

boost::posix_time::microseconds TimeToPosixTime(const Time& time)
{
    static const Time time_factor = 1.0 * micro * seconds;
    const double delay_in_micro_seconds = time / time_factor;
    return boost::posix_time::microseconds(delay_in_micro_seconds);
}

void Sleep(const Time& time)
{
    boost::posix_time::microseconds posix_time = TimeToPosixTime(time);
    boost::this_thread::sleep(posix_time);
}

}
