/*!
 * \file IVCurve.cc
 * \brief Implementation of IVCurve class.
 */

#include <TGraph.h>

#include "IVCurve.h"
#include "interface/Keithley237.h"
#include "psi/log.h"
#include "psi/date_time.h"
#include "BasePixel/ConfigParameters.h"
#include "BasePixel/TestParameters.h"
#include "BasePixel/DataStorage.h"

static const std::string LOG_HEAD = "IVCurve";

IVCurve::IVCurve()
    : Test("IVCurve")
{
    psi::LogInfo(LOG_HEAD) << "Initialization." << std::endl;
    ReadTestParameters();
    hvSource = psi::VoltageSourceFactory::Get();
    params->Branch("IVStep", const_cast<double*>(&voltStep.value()));
    params->Branch("IVStart", const_cast<double*>(&voltStart.value()));
    params->Branch("IVStop", const_cast<double*>(&voltStop.value()));
    params->Branch("IVDelay", const_cast<double*>(&delay.value()));
    params->Branch("IVCompliance", const_cast<double*>(&compliance.value()));
    params->Branch("IVRampStep", const_cast<double*>(&rampStep.value()));
    params->Branch("IVRampDelay", const_cast<double*>(&rampDelay.value()));
    params->Fill();
    results->Branch("Current", const_cast<double*>(&measuredCurrent.value()));
    results->Branch("Voltage", const_cast<double*>(&measuredVoltage.value()));
}

void IVCurve::ReadTestParameters()
{
    const TestParameters& testParameters = TestParameters::Singleton();
    voltStep = testParameters.IVStep();
    if(voltStep <= 0.0 * psi::volts)
        THROW_PSI_EXCEPTION("Invalid voltage step = " << voltStep << ". The voltage step should be greater then zero.");
    voltStart = testParameters.IVStart();
    voltStop = testParameters.IVStop();
    compliance = testParameters.IVCompliance();
    if(compliance <= 0.0 * psi::amperes)
        THROW_PSI_EXCEPTION("Invalid compliance = " << compliance << ". The compliance should be greater then zero.");
    rampStep = testParameters.IVRampStep();
    if(rampStep <= 0.0 * psi::volts)
        THROW_PSI_EXCEPTION("Invalid ramp voltage step = " << rampStep
                            << ". The ramp voltage step should be greater then zero.");
    delay = testParameters.IVDelay();
    if(delay < 0.0 * psi::seconds)
        THROW_PSI_EXCEPTION("Invalid delay between the voltage set and the measurement = " << delay
                            << ". The delay should not be negative.");
    rampDelay = testParameters.IVRampDelay();
    if(rampDelay < 0.0 * psi::seconds)
        THROW_PSI_EXCEPTION("Invalid ramp delay between the voltage switch = " << rampDelay
                            << ". The ramp delay should not be negative.");
}

void IVCurve::StopTest()
{
    psi::LogInfo(LOG_HEAD) << "Ending IV curve test. Ramping down voltage." << std::endl;
    hvSource->GradualSet(psi::IVoltageSource::Value(0.0 * psi::volts, compliance), rampStep, rampDelay, false);
    hvSource->Off();
    psi::LogInfo(LOG_HEAD) << "High voltage source is turned off." << std::endl;
}

bool IVCurve::SafelyIncreaseVoltage(psi::ElectricPotential goalVoltage)
{
    if(voltStart != 0.0 * psi::volts)
        psi::LogInfo(LOG_HEAD) << "Safely increasing voltage to the starting voltage = " << voltStart
                               << std::endl;

    const bool result = hvSource->GradualSet(psi::IVoltageSource::Value(voltStart, compliance), rampStep, rampDelay);
    if(!result) {
        psi::LogInfo(LOG_HEAD) << "Compliance is reached while trying to achieve the goal voltage = "
                               << goalVoltage << ". Aborting the IV test." << std::endl;
        StopTest();
    }
    return result;
}

void IVCurve::ModuleAction(TestModule&)
{
    psi::LogInfo(LOG_HEAD) << "Starting IV test..." << std::endl;
    boost::lock_guard<psi::ThreadSafeVoltageSource> lock(*hvSource);
    //std::vector<psi::IVoltageSource::Measurement> measurements;
    if(voltStart < voltStop)
        voltStep = psi::abs(voltStep);
    else
        voltStep = -psi::abs(voltStep);

    if(!SafelyIncreaseVoltage(voltStart))
        return;
    for (psi::ElectricPotential v = voltStart;;) {
        psi::LogInfo(LOG_HEAD) << "Setting on high voltage source " << v << " with " << compliance
                               << " compliance." << std::endl;
        const psi::IVoltageSource::Value setValue = hvSource->Set(psi::IVoltageSource::Value(v, compliance));
        psi::LogInfo(LOG_HEAD) << "High voltage source is set to " << setValue.Voltage << " with "
                               << setValue.Compliance << " compliance." << std::endl;

        psi::LogInfo(LOG_HEAD) << "Wait for " << delay << std::endl;
        psi::Sleep(delay);

        const psi::IVoltageSource::Measurement measurement = hvSource->Measure();
        psi::LogInfo(LOG_HEAD) << "Measured value is: " << measurement << std::endl;
        measuredVoltage = measurement.Voltage;
        measuredCurrent = measurement.Current;
        results->Fill();
        //measurements.push_back(measurement);

        if(measurement.Compliance) {
            record.result = 1;
            psi::LogInfo(LOG_HEAD) << "Compliance is reached. Stopping IV test." << std::endl;
            break;
        }
        const psi::ElectricPotential diff = psi::abs(v - voltStop);
        if(diff < hvSource->Accuracy(voltStop))
            break;
        if(diff < psi::abs(voltStep))
            v = voltStop;
        else
            v += voltStep;
    }
    StopTest();
//    psi::DataStorage::Active().SaveGraph("IVCurve", measurements);
    psi::LogInfo(LOG_HEAD) << "IV test is done." << std::endl;
}
