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
    boost::shared_ptr<TestRoc> GetRoc(int iRoc);
    void DoTest(boost::shared_ptr<Test> aTest);
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

    void IanaScan();

    unsigned NRocs();
    bool GetTBM(TBMParameters::Register reg, int& value);
    void SetTBM(int chipId, TBMParameters::Register reg, int value);
    TBM* GetTBM();
    void SetTBMSingle(int tbmChannel);
    void AdjustDTL();
    void Initialize();
    void WriteDACParameterFile( const char* filename);

    const std::vector< boost::shared_ptr<TestRoc> >& Rocs() const {
        return rocs;
    }

private:
    std::vector< boost::shared_ptr<TestRoc> > rocs;

    TBM *tbm;
    int controlNetworkId;
    boost::shared_ptr<TBAnalogInterface> tbInterface;

    int hubId;
};
