/*!
 * \file BiasVoltageController.h
 * \brief Definition of BiasVoltageController class.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 06-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Method Enable/Disable separated for control and bias.
 *      - Switched to boost::recursive_mutex.
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
class BiasVoltageController : boost::noncopyable {
public:
    typedef boost::function<void (const IVoltageSource::Measurement&)> OnComplianceCallback;
    typedef boost::function<void (const std::exception&)> OnErrorCallback;
    typedef boost::shared_ptr<ThreadSafeVoltageSource> VoltageSourcePtr;

public:
    BiasVoltageController(const OnComplianceCallback& onComplianceCallback, const OnErrorCallback& onErrorCallback);
    ~BiasVoltageController();
    void operator()();
    void EnableControl();
    void DisableControl();
    void EnableBias();
    void DisableBias();

    void Stop();

    bool ControlEnabled();
    bool BiasEnabled();

private:
    boost::recursive_mutex mutex;
    boost::condition_variable_any controlStateChange;
    OnComplianceCallback onCompliance;
    OnErrorCallback onError;
    bool controlEnabled, biasEnabled, canRun, isRunning;
    VoltageSourcePtr voltageSource;
    boost::posix_time::microseconds currentCheckInterval;
};

} // psi
