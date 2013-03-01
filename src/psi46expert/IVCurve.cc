/*!
 * \file IVCurve.cc
 * \brief Implementation of IVCurve class.
 *
 * \b Changelog
 * 25-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using ThreadSafeVoltageSource::GradualSet method to safely increase/decrease voltage.
 *      - IVoltageSource and VoltageSourceFactory moved into psi namespace.
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using VoltageSourceFactory.
 *      - Now using definitions from PsiCommon.h.
 * 21-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using DataStorage class to save the results.
 * 18-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - IVCurve test algorithm changed for the bare module tests.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new ConfigParameters class definition.
 *      - Adaptation for the new TestParameters class definition.
 * 10-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - IVoltageSource interface was changed.
 * 30-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Changed to support IVoltageSource interface.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Changed to support IHighVoltageSource interface.
 */

#include <iostream>

#include <TGraph.h>

#include "IVCurve.h"
#include "BasePixel/Keithley237.h"
#include "interface/Log.h"
#include "BasePixel/ConfigParameters.h"
#include "TestParameters.h"
#include "DataStorage.h"

IVCurve::IVCurve(TestRange*, TBInterface*)
{
    psi::Log<psi::Info>() << "[IVCurve] Initialization." << std::endl;
    ReadTestParameters();
    hvSource = psi::VoltageSourceFactory::Get();
}

void IVCurve::ReadTestParameters()
{
    const TestParameters& testParameters = TestParameters::Singleton();
    voltStep = testParameters.IVStep();
    if(voltStep <= 0.0 * psi::volts)
        THROW_PSI_EXCEPTION("[IVCurve] Invalid voltage step = " << voltStep << ". The voltage step should be greater"
                            " then zero.");
    voltStart = testParameters.IVStart();
    voltStop = testParameters.IVStop();
    compliance = testParameters.IVCompliance();
    if(compliance <= 0.0 * psi::amperes)
        THROW_PSI_EXCEPTION("[IVCurve] Invalid compliance = " << compliance << ". The compliance should be greater then"
                            " zero.");
    rampStep = testParameters.IVRampStep();
    if(rampStep <= 0.0 * psi::volts)
        THROW_PSI_EXCEPTION("[IVCurve] Invalid ramp voltage step = " << rampStep << ". The ramp voltage"
                            " step should be greater then zero.");
    delay = testParameters.IVDelay();
    if(delay < 0.0 * psi::seconds)
        THROW_PSI_EXCEPTION("[IVCurve] Invalid delay between the voltage set and the measurement = " << delay <<
                            ". The delay should not be negative.");
    rampDelay = testParameters.IVRampDelay();
    if(rampDelay < 0.0 * psi::seconds)
        THROW_PSI_EXCEPTION("[IVCurve] Invalid ramp delay between the voltage switch = " << rampDelay <<
                            ". The ramp delay should not be negative.");
}

void IVCurve::StopTest()
{
    psi::Log<psi::Info>() << "[IVCurve] Ending IV curve test. Ramping down voltage." << std::endl;
    hvSource->GradualSet(psi::IVoltageSource::Value(0.0 * psi::volts, compliance), rampStep, rampDelay, false);
    hvSource->Off();
    psi::Log<psi::Info>() << "[IVCurve] High voltage source is turned off." << std::endl;
}

bool IVCurve::SafelyIncreaseVoltage(psi::ElectricPotential goalVoltage)
{
    if(voltStart != 0.0 * psi::volts)
        psi::Log<psi::Info>() << "[IVCurve] Safely increasing voltage to the starting voltage = " << voltStart << std::endl;

    const bool result = hvSource->GradualSet(psi::IVoltageSource::Value(voltStart, compliance), rampStep, rampDelay);
    if(!result)
    {
        psi::Log<psi::Info>() << "[IVCurve::SafelyIncreaseVoltage]  Compliance is reached while trying to achieve the"
                          " goal voltage = " << goalVoltage << ". Aborting the IV test." << std::endl;
        StopTest();
    }
    return result;
}

void IVCurve::ModuleAction()
{
    psi::Log<psi::Info>() << "[IVCurve] Starting IV test..." << std::endl;
    boost::lock_guard<psi::ThreadSafeVoltageSource> lock(*hvSource);
    std::vector<psi::IVoltageSource::Measurement> measurements;
    if(voltStart < voltStop)
        voltStep = boost::units::abs(voltStep);
    else
        voltStep = -boost::units::abs(voltStep);

    if(!SafelyIncreaseVoltage(voltStart))
        return;
    for (psi::ElectricPotential v = voltStart;;)
    {
        psi::Log<psi::Info>() << "[IVCurve] Setting on high voltage source " << v << " with " << compliance
                   << " compliance." << std::endl;
        const psi::IVoltageSource::Value setValue = hvSource->Set(psi::IVoltageSource::Value(v, compliance));
        psi::Log<psi::Info>() << "[IVCurve] High voltage source is set to " << setValue.Voltage << " with "
                       << setValue.Compliance << " compliance." << std::endl;

        psi::Log<psi::Info>() << "[IVCurve] Wait for " << delay << std::endl;
        psi::Sleep(delay);

        const psi::IVoltageSource::Measurement measurement = hvSource->Measure();
        psi::Log<psi::Info>() << "[IVCurve] Measured value is: " << measurement << std::endl;
        measurements.push_back(measurement);

        if(measurement.Compliance)
        {
            psi::Log<psi::Info>() << "[IVCurve] Compliance is reached. Stopping IV test." << std::endl;
            break;
        }
        const psi::ElectricPotential diff = boost::units::abs(v - voltStop);
        if(diff < hvSource->Accuracy(voltStop))
            break;
        if(diff < boost::units::abs(voltStep))
            v = voltStop;
        else
            v += voltStep;
    }
    StopTest();
    psi::DataStorage::Active().SaveGraph("IVCurve", measurements);
    psi::Log<psi::Info>() << "[IVCurve] IV test is done." << std::endl;
}
