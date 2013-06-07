/*!
 * \file VoltageSourceFactory.h
 * \brief Definition of VoltageSourceFactory class.
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
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
