/*!
 * \file BiasVoltageController.cc
 * \brief Implementation of BiasVoltageController class.
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 */

#include "psi/exception.h"
#include "psi/date_time.h"
#include "BasePixel/TestParameters.h"
#include "BasePixel/VoltageSourceFactory.h"
#include "BiasVoltageController.h"
#include "data/ElectricCurrentMeasurements.h"
#include "BasePixel/DataStorage.h"

psi::BiasVoltageController::BiasVoltageController(const OnComplianceCallback& onComplianceCallback,
        const OnErrorCallback& onErrorCallback)
    : onCompliance(onComplianceCallback), onError(onErrorCallback), controlEnabled(false), biasEnabled(false),
      canRun(true), isRunning(false), voltageSource(VoltageSourceFactory::Get()), currentCheckInterval(0)
{
}

psi::BiasVoltageController::~BiasVoltageController()
{
    boost::lock_guard<boost::recursive_mutex> lock(mutex);
    if(isRunning)
        THROW_PSI_EXCEPTION("Invalid usage. The object should not be destroyed while working thread is still running.");
}

bool psi::BiasVoltageController::ControlEnabled()
{
    boost::lock_guard<boost::recursive_mutex> lock(mutex);
    return controlEnabled;
}

bool psi::BiasVoltageController::BiasEnabled()
{
    boost::lock_guard<boost::recursive_mutex> lock(mutex);
    return biasEnabled;
}

void psi::BiasVoltageController::operator()()
{
    boost::unique_lock<boost::recursive_mutex> lock(mutex);
    try {
        isRunning = true;
        while(canRun) {
            if(!controlStateChange.timed_wait(lock, currentCheckInterval) && controlEnabled) {
                const IVoltageSource::Measurement measurement = voltageSource->Measure();
                if(measurement.Compliance)
                    onCompliance(measurement);
            }
        }
    } catch(psi::exception& e) {
        onError(e);
    } catch(boost::thread_resource_error& e) {
        onError(e);
    } catch(boost::thread_interrupted&) {
    }

    isRunning = false;
}

void psi::BiasVoltageController::EnableControl()
{
    {
        boost::lock_guard<boost::recursive_mutex> lock(mutex);
        if(controlEnabled)
            return;
        controlEnabled = true;
    }
    controlStateChange.notify_one();
}

void psi::BiasVoltageController::DisableControl()
{
    {
        boost::lock_guard<boost::recursive_mutex> lock(mutex);
        if(!controlEnabled)
            return;
        controlEnabled = false;
    }
    controlStateChange.notify_one();
}

void psi::BiasVoltageController::EnableBias()
{
    bool result;
    {
        boost::lock_guard<boost::recursive_mutex> lock(mutex);
        const TestParameters& testParameters = TestParameters::Singleton();
        const ElectricPotential voltage = testParameters.BiasVoltage();
        const ElectricCurrent compliance = testParameters.BiasCompliance();
        const ElectricPotential rampStep = testParameters.BiasRampStep();
        const Time rampDelay = testParameters.BiasRampDelay();
        currentCheckInterval = TimeToPosixTime(testParameters.BiasCurrentCheckInterval());
        biasEnabled = true;
        result = voltageSource->GradualSet(IVoltageSource::Value(voltage, compliance), rampStep, rampDelay);
    }
    if(!result) {
        onCompliance(voltageSource->Measure());
        THROW_PSI_EXCEPTION("Compliance is reached while enabling bias voltage.");
    }
}

void psi::BiasVoltageController::DisableBias()
{
    boost::lock_guard<boost::recursive_mutex> lock(mutex);
    const TestParameters& testParameters = TestParameters::Singleton();
    const ElectricCurrent compliance = testParameters.BiasCompliance();
    const ElectricPotential rampStep = testParameters.BiasRampStep();
    const Time delay = testParameters.BiasRampDelay();
    voltageSource->GradualSet(IVoltageSource::Value(0.0 * psi::volts, compliance), rampStep, delay);
    voltageSource->Off();
    biasEnabled = false;
}


void psi::BiasVoltageController::Stop()
{
    {
        boost::lock_guard<boost::recursive_mutex> lock(mutex);
        canRun = false;
    }
    controlStateChange.notify_one();
}

void psi::BiasVoltageController::SaveMeasurements() const
{
    psi::DataStorage::Active().EnterDirectory("/");
    psi::data::ElectricCurrentMeasurements measurementTree;
    {
        const boost::lock_guard<psi::ThreadSafeVoltageSource> lock(*voltageSource);
        const ThreadSafeVoltageSource::MeasurementCollection& measurements = voltageSource->Measurements();
        for(ThreadSafeVoltageSource::MeasurementCollection::const_iterator iter = measurements.begin();
            iter != measurements.end(); ++iter) {
            measurementTree.current() = psi::DataStorage::ToStorageUnits(iter->Current);
            measurementTree.voltage() = psi::DataStorage::ToStorageUnits(iter->Voltage);
            measurementTree.timestamp() = psi::DataStorage::ToStorageUnits(iter->Timestamp);
            measurementTree.Fill();
        }
    }
    measurementTree.RootTree().Write("", TObject::kWriteDelete);
    psi::DataStorage::Active().GoToPreviousDirectory();
}
