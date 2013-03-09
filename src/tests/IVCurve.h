/*!
 * \file IVCurve.h
 * \brief Definition of IVCurve class.
 *
 * \b Changelog
 * 25-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using ThreadSafeVoltageSource::GradualSet method to safely increase/decrease voltage.
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using VoltageSourceFactory.
 * 21-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using DataStorage class to save the results.
 * 18-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - IVCurve test algorithm changed for the bare module tests.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Changed to support IHighVoltageSource interface.
 */

#pragma once

#include "BasePixel/Test.h"
#include "BasePixel/VoltageSourceFactory.h"

class IVCurve : public Test
{
public:
    IVCurve(TestRange *testRange, TBInterface *aTBInterface);
    virtual void ReadTestParameters();
    virtual void ModuleAction();

private:
    void StopTest();
    bool SafelyIncreaseVoltage(psi::ElectricPotential goalVoltage);

private:
    psi::ElectricPotential voltStep, voltStart, voltStop, rampStep;
    psi::ElectricCurrent compliance;
    psi::Time delay, rampDelay;
    boost::shared_ptr<psi::ThreadSafeVoltageSource> hvSource;
};
