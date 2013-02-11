/*!
 * \file VoltageSourceFactory.h
 * \brief Definition of VoltageSourceFactory class.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 11-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#pragma once

#include "IVoltageSource.h"

class VoltageSourceFactory
{
public:
    static boost::shared_ptr<IVoltageSource> Get();

private:
    VoltageSourceFactory() {}
};
