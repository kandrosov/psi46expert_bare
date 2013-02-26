/*!
 * \file BiasVoltageController.cc
 * \brief Implementation of BiasVoltageController class.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 26-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#include "TestParameters.h"
#include "BasePixel/VoltageSourceFactory.h"
#include "BasePixel/psi_exception.h"

#include "BiasVoltageController.h"

psi::BiasVoltageController::BiasVoltageController(const OnComplianceCallback& onComplianceCallback,
                                                  const OnErrorCallback& onErrorCallback)
    : onCompliance(onComplianceCallback), onError(onErrorCallback), enabled(false), canRun(true), isRunning(false),
      voltageSource(VoltageSourceFactory::Get()), currentCheckInterval(0)
{
}

psi::BiasVoltageController::~BiasVoltageController()
{
    if(isRunning)
        THROW_PSI_EXCEPTION("[BiasVoltageController::~BiasVoltageController] Invalid usage. The object should not be"
                            " destroyed while working thread is still running.");
}

void psi::BiasVoltageController::operator()()
{
    try
    {
        boost::unique_lock<boost::mutex> lock(mutex);
        isRunning = true;
        while(canRun)
        {
            if(!stateChange.timed_wait(lock, currentCheckInterval) && enabled)
            {
                const IVoltageSource::Measurement measurement = voltageSource->Measure();
                if(measurement.Compliance)
                    onCompliance(measurement);
            }
        }
    }
    catch(psi::exception& e)
    {
        onError(e);
    }
    catch(boost::thread_resource_error& e)
    {
        onError(e);
    }
    catch(boost::thread_interrupted&)
    {
    }

    isRunning = false;
}

void psi::BiasVoltageController::Enable()
{
    {
        boost::lock_guard<boost::mutex> lock(mutex);
        if(enabled)
            return;
        const TestParameters& testParameters = TestParameters::Singleton();
        const ElectricPotential voltage = testParameters.BiasVoltage();
        const ElectricCurrent compliance = testParameters.BiasCompliance();
        const ElectricPotential rampStep = testParameters.BiasRampStep();
        const Time rampDelay = testParameters.BiasRampDelay();
        currentCheckInterval = TimeToPosixTime(testParameters.BiasCurrentCheckInterval());
        voltageSource->GradualSet(IVoltageSource::Value(voltage, compliance), rampStep, rampDelay);
        enabled = true;
    }
    stateChange.notify_one();
}

void psi::BiasVoltageController::Disable()
{
    {
        boost::lock_guard<boost::mutex> lock(mutex);
        if(!enabled)
            return;
        enabled = false;
        const TestParameters& testParameters = TestParameters::Singleton();
        const ElectricCurrent compliance = testParameters.BiasCompliance();
        const ElectricPotential rampStep = testParameters.BiasRampStep();
        const Time delay = testParameters.BiasRampDelay();
        voltageSource->GradualSet(IVoltageSource::Value(0.0 * psi::volts, compliance), rampStep, delay);
        voltageSource->Off();
    }
    stateChange.notify_one();
}

void psi::BiasVoltageController::Stop()
{
    {
        boost::lock_guard<boost::mutex> lock(mutex);
        canRun = false;
    }
    stateChange.notify_one();
}
