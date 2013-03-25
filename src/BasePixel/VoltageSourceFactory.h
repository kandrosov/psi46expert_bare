/*!
 * \file VoltageSourceFactory.h
 * \brief Definition of VoltageSourceFactory class.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using ThreadSafeVoltageSource.
 *      - IVoltageSource, ThreadSafeVoltageSource and VoltageSourceFactory moved into psi namespace.
 * 11-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#pragma once

#include "interface/ThreadSafeVoltageSource.h"

namespace psi {
class VoltageSourceFactory {
public:
    typedef boost::shared_ptr<ThreadSafeVoltageSource> VoltageSourcePtr;
    static VoltageSourcePtr Get();

private:
    VoltageSourceFactory() {}
};

}
