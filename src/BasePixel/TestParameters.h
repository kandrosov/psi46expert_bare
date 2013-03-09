/*!
 * \file TestParameters.h
 * \brief Definition of TestParameters class.
 *
 * \b Changelog
 * 06-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Added PSI_CONFIG_NAME macros.
 *      - Macros CONFIG_PARAMETER renamed to PSI_CONFIG_PARAMETER.
 * 26-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Added parameters for continues bias control.
 * 18-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Reviewed parameters for the IVCurve test.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - TestParameters class now inherit psi::BaseConifg class.
 */

#pragma once

#include "BasePixel/BaseConfig.h"

/*!
 * \brief Central definition point of the parameters which control the test behaviour
 */
class TestParameters : public psi::BaseConfig
{
public:
    PSI_CONFIG_PARAMETER(int, PixelMapReadouts, 20)
    PSI_CONFIG_PARAMETER(int, PixelMapEfficiency, 60)
    PSI_CONFIG_PARAMETER(int, SCurveMode, 1)
    PSI_CONFIG_PARAMETER(int, SCurveNTrig, 0)
    PSI_CONFIG_PARAMETER(int, SCurveVcal, 150)
    PSI_CONFIG_PARAMETER(int, SCurveVthr, 80)
    PSI_CONFIG_PARAMETER(int, SCurveBeamNTrig, 5)
    PSI_CONFIG_PARAMETER(int, ThresholdMode, 1)
    PSI_CONFIG_PARAMETER(int, ThresholdNTrig, 0)
    PSI_CONFIG_PARAMETER(int, ThresholdVcal, 150)
    PSI_CONFIG_PARAMETER(int, ThresholdVthr, 80)
    PSI_CONFIG_PARAMETER(int, BumpBondingNTrig, 10)
    PSI_CONFIG_PARAMETER(int, TrimBitsVtrim14, 250)
    PSI_CONFIG_PARAMETER(int, TrimBitsVtrim13, 200)
    PSI_CONFIG_PARAMETER(int, TrimBitsVtrim11, 150)
    PSI_CONFIG_PARAMETER(int, TrimBitsVtrim7, 100)
    PSI_CONFIG_PARAMETER(int, TrimBitsNTrig, 10)
    PSI_CONFIG_PARAMETER(int, PHNTrig, 5)
    PSI_CONFIG_PARAMETER(int, PHMode, 25)
    PSI_CONFIG_PARAMETER(int, PHdac1Start, 100)
    PSI_CONFIG_PARAMETER(int, PHdac1Stop, 100)
    PSI_CONFIG_PARAMETER(int, PHdac1Step, 10)
    PSI_CONFIG_PARAMETER(int, PHdac2Start, 40)
    PSI_CONFIG_PARAMETER(int, PHdac2Stop, 40)
    PSI_CONFIG_PARAMETER(int, PHdac2Step, 10)
    PSI_CONFIG_PARAMETER(int, PHNumberOfSteps, 4)
    PSI_CONFIG_PARAMETER(int, PHtestVcal, 60)
    PSI_CONFIG_PARAMETER(int, PHDacType, 1)
    PSI_CONFIG_PARAMETER(int, PHSafety, 50)
    PSI_CONFIG_PARAMETER(int, TrimVcal, 50)
    PSI_CONFIG_PARAMETER(int, TrimNTrig, 10)
    PSI_CONFIG_PARAMETER(int, TrimDoubleWbc, 1)
    PSI_CONFIG_PARAMETER(int, Dac1, 12)
    PSI_CONFIG_PARAMETER(int, Dac2, 25)
    PSI_CONFIG_PARAMETER(int, DacRange1, 256)
    PSI_CONFIG_PARAMETER(int, DacRange2, 256)
    PSI_CONFIG_PARAMETER(int, DacNTrig, 10)
    PSI_CONFIG_PARAMETER(int, PHCalibrationNTrig, 1)
    PSI_CONFIG_PARAMETER(int, PHCalibrationMode, 0)
    PSI_CONFIG_PARAMETER(int, PHCalibrationNPixels, 4160)
    PSI_CONFIG_PARAMETER(int, PHCalibrationCalDelVthrComp, 1)

    PSI_CONFIG_PARAMETER(psi::ElectricPotential, IVStep, 5.0 * psi::volts)
    PSI_CONFIG_PARAMETER(psi::ElectricPotential, IVStart, 0.0 * psi::volts)
    PSI_CONFIG_PARAMETER(psi::ElectricPotential, IVStop, 600.0 * psi::volts)
    PSI_CONFIG_PARAMETER(psi::ElectricCurrent, IVCompliance, 1.0 * psi::micro * psi::amperes)
    PSI_CONFIG_PARAMETER(psi::Time, IVDelay, 3.0 * psi::seconds)
    PSI_CONFIG_PARAMETER(psi::ElectricPotential, IVRampStep, 20.0 * psi::volts)
    PSI_CONFIG_PARAMETER(psi::Time, IVRampDelay, 0.5 * psi::seconds)

    PSI_CONFIG_PARAMETER(psi::ElectricPotential, BiasVoltage, 200.0 * psi::volts)
    PSI_CONFIG_PARAMETER(psi::ElectricCurrent, BiasCompliance, 1.0 * psi::micro * psi::amperes)
    PSI_CONFIG_PARAMETER(psi::ElectricPotential, BiasRampStep, 20.0 * psi::volts)
    PSI_CONFIG_PARAMETER(psi::Time, BiasRampDelay, 0.5 * psi::seconds)
    PSI_CONFIG_PARAMETER(psi::Time, BiasCurrentCheckInterval, 1.0 * psi::seconds)

    PSI_CONFIG_PARAMETER(int, TempNTrig, 1)
    PSI_CONFIG_PARAMETER(int, TBMUbLevel, -700)
    PSI_CONFIG_PARAMETER(int, TWMeanShift, 20)
    PSI_CONFIG_PARAMETER(int, vsfStart, 120)
    PSI_CONFIG_PARAMETER(int, vsfStop, 180)
    PSI_CONFIG_PARAMETER(int, vsfSteps, 10)
    PSI_CONFIG_PARAMETER(int, vhldDelStart, 10)
    PSI_CONFIG_PARAMETER(int, vhldDelStop, 250)
    PSI_CONFIG_PARAMETER(int, vhldDelSteps, 10)
    PSI_CONFIG_PARAMETER(double, goalPar1, 1.4)
    PSI_CONFIG_PARAMETER(psi::ElectricCurrent, goalCurrent, 5.0 * psi::milli * psi::amperes)
    PSI_CONFIG_PARAMETER(int, XrayNTrig, 10000)
    PSI_CONFIG_PARAMETER(int, XrayVthrCompMin, 40)
    PSI_CONFIG_PARAMETER(int, XrayVthrCompMax, 110)
    PSI_CONFIG_PARAMETER(double, XrayMaxEff, 0.01)

public:
    static TestParameters& ModifiableSingleton()
    {
        static TestParameters instance;
        return instance;
    }

    static const TestParameters& Singleton()
    {
        return ModifiableSingleton();
    }

private:
    PSI_CONFIG_NAME("TestParameters")
    TestParameters() {}
};
