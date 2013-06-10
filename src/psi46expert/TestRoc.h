/*!
 * \file TestRoc.h
 * \brief Definition of TestRoc class.
 */

#pragma once

#include "TestDoubleColumn.h"
#include "BasePixel/DACParameters.h"
#include "BasePixel/TBInterface.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/TestRange.h"

class Test;
class TestModule;

/*!
 * \brief Implementation of the tests at ROC level
 */
class TestRoc {
public:
    TestRoc(boost::shared_ptr<TBAnalogInterface> aTBInterface, TestModule& testModule, int aChipId, int aHubId,
            int aPortId, int aoutChipPosition);

    TestDoubleColumn& GetDoubleColumnById(unsigned doubleColumn) { return *doubleColumns.at(doubleColumn); }
    TestDoubleColumn& GetDoubleColumnByColumnId(unsigned column) { return GetDoubleColumnById(column / 2); }
    TestPixel& GetPixel(unsigned col, unsigned row) { return GetDoubleColumnByColumnId(col).GetPixel(col, row); }
    TestPixel& GetTestPixel();
    boost::shared_ptr<const TestRange> GetRange() const { return fullRange; }

// == Setting DACS ======================================

    int AdjustVana(psi::ElectricCurrent current0, psi::ElectricCurrent goalcurrent);
    void AdjustCalDelVthrComp();
    void AdjustCalDelVthrComp(int column, int row, int vcal, int belowNoise);
    void AdjustUltraBlackLevel(int ubLevel);

// == Tests =============================================

    void ChipTest();
    void DoTest(boost::shared_ptr<Test> aTest);
    void Test1();

    void PhError();
    void Rainbow();
    void Rainbow2();

    void TestM();
    void PowerOnTest(int nTests);
    void ADCSamplingTest();
    //	void DataTriggerLevelScan();
    double GetTemperature();
    void TrimVerification();

    void DoTrim();
    void DoTrimVcal();
    void DoTrimLow();
    void DoPhCalibration();
    void ThrMaps();
    void DoPulseShape();
    double DoPulseShape(int column, int row, int vcal);

// == Helper routines ====================================

    int CountReadouts(int count);
    double Threshold(int sCurve[], int start, int sign, int step, double thrLevel);
    void SendSignals(int start, int stop, int step, int nTrig, DACParameters::Register dacReg);
    void ReadSignals(int start, int stop, int step, int nTrig, int sCurve[]);
    int fitLeft(TH2D *his, int line, int maxEntries);
    int fitRight(TH2D *his, int line, int maxEntries);

// == Histos =============================================

    TH2D* TrimMap();

    int GetChipId();
    int GetAoutChipPosition();
    void AddressLevelsTest(int result[]);
    int ChipEfficiency(int nTriggers, double res[]);
    void DacDac(int dac1, int dacRange1, int dac2, int dacRange2, int nTrig, int result[]);
    void EnableDoubleColumn(int col);
    void ArmPixel(int column, int row);
    void DisarmPixel(int column, int row);
    int AoutLevelChip(int position, int nTriggers, int res[]);
    int AoutLevelPartOfChip(int position, int nTriggers, int res[], bool pxlFlags[]);
    void GetTrimValues(int buffer[]);
    void SetTrim(int trim);
    int GetDAC(DACParameters::Register dacReg);
    void SetTrim(int iCol, int iRow, int trimBit);
    void ClrCal();
    int MaskTest(short nTriggers, short res[]);
    void EnablePixel(int col, int row);
    boost::shared_ptr<DACParameters> SaveDacParameters();
    void RestoreDacParameters(boost::shared_ptr<DACParameters> dacParameters = boost::shared_ptr<DACParameters>());
    void SetDAC(DACParameters::Register reg, int value);
    void DisableDoubleColumn(int col);
    void Mask();
    void EnableAllPixels();
    void SendADCTrigs(int nTrig);
    bool GetADC(short buffer[], unsigned short buffersize, unsigned short &wordsread, int nTrig, int startBuffer[],
                int &nReadouts);
    void WriteDACParameterFile(const std::string& filename);
    void Initialize();
    void RocSetDAC(int reg, int value);
    void CDelay(int clocks);
    void SingleCal();
    int GetRoCnt();
    void ColEnable(int col, int on);
    void PixTrim(int col, int row, int value);
    void PixMask(int col, int row);
    void PixCal(int col, int row, int sensorcal);
    void Cals(int col, int row);
    void Cal(int col, int row);
    void Flush();
    int PixelThreshold(int col, int row, int start, int step, int thrLevel, int nTrig, int dacReg, int xtalk, int cals,
                       int trim);
    void SendCal(int nTrig);
    int RecvRoCnt();
    int ChipThreshold(int start, int step, int thrLevel, int nTrig, int dacReg, int xtalk, int cals, int data[]);
    void WriteTrimConfiguration(const char* filename);
    void TrimAboveNoise(short nTrigs, short thr, short mode, short result[]);
    void ReadDACParameterFile(const std::string& filename);
    void ReadTrimConfiguration( const char *filename);
    void DisablePixel(int col, int row);
    void SetChip();

    bool IsIncluded(boost::shared_ptr<const TestRange> testRange) const;
    void SendReset();
    TestModule& GetModule() const { return *testModule; }

private:
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    TestModule* testModule;
    const int chipId, hubId, portId, aoutChipPosition;
    std::vector< boost::shared_ptr<TestDoubleColumn> > doubleColumns;
    boost::shared_ptr<DACParameters> dacParameters, savedDacParameters;
    boost::shared_ptr<TestRange> fullRange;
};
