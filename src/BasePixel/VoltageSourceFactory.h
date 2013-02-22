/*!
 * \file VoltageSourceFactory.h
 * \brief Definition of VoltageSourceFactory class.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using ThreadSafeVoltageSource.
 * 11-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#pragma once

#include "ThreadSafeVoltageSource.h"

class VoltageSourceFactory
{
public:
    static boost::shared_ptr<ThreadSafeVoltageSource> Get();

private:
    VoltageSourceFactory() {}
};
