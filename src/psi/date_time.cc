/*!
 * \file date_time.cc
 * \brief Implementation of the time-related functionality for the psi namespace.
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 */

#include <boost/thread.hpp>

#include "date_time.h"

namespace psi {

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
