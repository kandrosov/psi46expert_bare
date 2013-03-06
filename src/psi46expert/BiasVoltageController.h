/*!
 * \file BiasVoltageController.h
 * \brief Definition of BiasVoltageController class.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 26-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#pragma once

#include "BasePixel/constants.h"
#include "interface/ThreadSafeVoltageSource.h"
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

namespace psi {

/*!
 * \brief Set bias voltage for the test and control that the current value is not reach the compliance.
 */
class BiasVoltageController : boost::noncopyable
{
public:
    typedef boost::function<void (const IVoltageSource::Measurement&)> OnComplianceCallback;
    typedef boost::function<void (const std::exception&)> OnErrorCallback;
    typedef boost::shared_ptr<ThreadSafeVoltageSource> VoltageSourcePtr;

public:
    BiasVoltageController(const OnComplianceCallback& onComplianceCallback, const OnErrorCallback& onErrorCallback);
    ~BiasVoltageController();
    void operator()();
    void Enable();
    void Disable();
    void Stop();

    bool IsEnabled() const { return enabled; }

private:
    boost::mutex mutex;
    boost::condition_variable stateChange;
    OnComplianceCallback onCompliance;
    OnErrorCallback onError;
    bool enabled, canRun, isRunning;
    VoltageSourcePtr voltageSource;
    boost::posix_time::microseconds currentCheckInterval;
};

} // psi
