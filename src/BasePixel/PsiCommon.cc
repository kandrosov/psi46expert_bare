/*!
 * \file PsiCommon.cc
 * \brief Provides a minimal set of constants, type definitions and functions for the project.
 *
 * \b Changelog
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Code moved from GlobalConstants.h.
 */

#include <boost/thread.hpp>

#include "PsiCommon.h"

namespace psi
{

static boost::posix_time::milliseconds TimeToPosixTime(const psi::Time& time)
{
    const double delay_in_ms = time / (0.001 * psi::seconds);
    return boost::posix_time::milliseconds(delay_in_ms);
}

void Sleep(const psi::Time& time)
{
    boost::posix_time::milliseconds posix_time = TimeToPosixTime(time);
    boost::this_thread::sleep(posix_time);
}

}
