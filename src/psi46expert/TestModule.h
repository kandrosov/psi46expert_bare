/*!
 * \file TestModule.h
 * \brief Definition of TestModule class.
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
    TestRoc& GetRoc(unsigned iRoc) { return *rocs.at(iRoc); }
    void DoTest(boost::shared_ptr<Test> aTest);
    boost::shared_ptr<const TestRange> FullRange() const { return fullRange; }

    void FullTestAndCalibration();
    void Calibration();
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

    void IanaScan();

    unsigned NRocs();
    bool GetTBM(TBMParameters::Register reg, int& value);
    void SetTBM(int chipId, TBMParameters::Register reg, int value);
    TBM& GetTBM() { return *tbm; }
    void SetTBMSingle(int tbmChannel);
    void AdjustDTL();
    void Initialize();
    void WriteDACParameterFile( const char* filename);

private:
    std::vector< boost::shared_ptr<TestRoc> > rocs;
    boost::shared_ptr<TBM> tbm;
    int controlNetworkId;
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    boost::shared_ptr<TestRange> fullRange;
    int hubId;
};
