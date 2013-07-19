/*!
 * \file date_time.cc
 * \brief Implementation of the time-related functionality for the psi namespace.
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 */

#include <boost/thread.hpp>
#include <boost/date_time.hpp>

#include "date_time.h"

namespace {
boost::posix_time::ptime _Now()
{
    return boost::date_time::microsec_clock<boost::posix_time::ptime>::universal_time();
}

const boost::posix_time::ptime& _StartTime()
{
    static boost::posix_time::ptime startTime = _Now();
    return startTime;
}
}

namespace psi {

boost::posix_time::microseconds TimeToPosixTime(const Time& time)
{
    static const Time time_factor = 1.0 * micro * seconds;
    const double delay_in_micro_seconds = time / time_factor;
    return boost::posix_time::microseconds(delay_in_micro_seconds);
}

void Sleep(const Time& time)
{
    const boost::posix_time::microseconds posix_time = TimeToPosixTime(time);
    boost::this_thread::sleep(posix_time);
}

std::string DateTimeProvider::Now()
{
    const boost::posix_time::ptime now = _Now();
    return boost::posix_time::to_iso_extended_string(now);
}

std::string DateTimeProvider::TimeNow()
{
    const boost::posix_time::ptime now = _Now();
    return boost::posix_time::to_simple_string(now.time_of_day());
}

std::string DateTimeProvider::StartTime()
{
    const boost::posix_time::ptime& startTime = _StartTime();
    return boost::posix_time::to_iso_extended_string(startTime);
}

psi::Time ElapsedTime()
{
    const boost::posix_time::time_duration deltaT = _Now() - _StartTime();
    return deltaT.total_seconds() * psi::seconds;
}

}
