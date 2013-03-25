/*!
 * \file TestModule.h
 * \brief Definition of TestModule class.
 *
 * \b Changelog
 * 13-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Using TBAnalogInterface instead TBInterface.
 *      - TBMParameters class now inherit psi::BaseConifg class.
 * 01-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Class SysCommand removed.
 * 26-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Removed redundant dependency from Module class.
 * 15-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using boost::units::quantity to represent physical values.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new ConfigParameters class definition.
 *      - Adaptation for the new TestParameters class definition.
 */

#pragma once

#include "BasePixel/TBM.h"
#include "TestRoc.h"
#include "BasePixel/Test.h"
#include "BasePixel/TestRange.h"

/*!
 * \brief This class provides support for the tests on the Module level
 */
class TestModule {

public:
    TestModule(int aCNId, boost::shared_ptr<TBAnalogInterface> aTBInterface);
    boost::shared_ptr<TestRoc> GetRoc(int iRoc);
    void DoTest(Test *aTest);
    TestRange *FullRange();

    void FullTestAndCalibration();
    void ShortTestAndCalibration();
    void ShortCalibration();
    void TestM();
    void DigiCurrent();
    void DoTBMTest();
    void AnaReadout();
    void DumpParameters();
    void DataTriggerLevelScan();
    void AdjustVana(psi::ElectricCurrent goalCurrent = 0.024 * psi::amperes);
    void AdjustAllDACParameters();
    void AdjustDACParameters();
    void AdjustUltraBlackLevel();
    void AdjustTBMUltraBlack();
    void AdjustSamplingPoint();
    void AdjustVOffsetOp();
    void AdjustPHRange();
    void CalibrateDecoder();
    double GetTemperature();
    void ADCHisto();
    void AdjustCalDelVthrComp();
    void MeasureCurrents();
    void VanaVariation();
    void Scurves();

    bool TestDACProgramming(int dacReg, int max);
    void TestDACProgramming();
    void IanaScan();

    unsigned NRocs();
    bool GetTBM(unsigned reg, int& value);
    void SetTBM(int chipId, unsigned reg, int value);
    TBM* GetTBM();
    void SetTBMSingle(int tbmChannel);
    void AdjustDTL();
    void Initialize();
    void WriteDACParameterFile( const char* filename);

private:
    std::vector< boost::shared_ptr<TestRoc> > rocs;

    TBM *tbm;
    int controlNetworkId;
    boost::shared_ptr<TBAnalogInterface> tbInterface;

    int hubId;
};
