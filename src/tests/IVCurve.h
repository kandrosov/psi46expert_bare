/*!
 * \file IVCurve.h
 * \brief Definition of IVCurve class.
 */

#pragma once

#include "BasePixel/Test.h"
#include "BasePixel/VoltageSourceFactory.h"

class IVCurve : public Test {
public:
    IVCurve();
    virtual void ReadTestParameters();
    virtual void ModuleAction(TestModule& module);

private:
    void StopTest();
    bool SafelyIncreaseVoltage(psi::ElectricPotential goalVoltage);

private:
    psi::ElectricPotential voltStep, voltStart, voltStop, rampStep;
    psi::ElectricCurrent compliance;
    psi::Time delay, rampDelay;
    boost::shared_ptr<psi::ThreadSafeVoltageSource> hvSource;
    psi::ElectricPotential measuredVoltage;
    psi::ElectricCurrent measuredCurrent;
};
