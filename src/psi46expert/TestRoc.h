/*!
 * \file TestRoc.h
 * \brief Definition of TestRoc class.
 *
 * \b Changelog
 * 01-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Class SysCommand removed.
 * 26-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Removed redundant dependency from Roc class.
 * 15-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using boost::units::quantity to represent physical values.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#pragma once

#include "TestDoubleColumn.h"
#include "BasePixel/DACParameters.h"
#include "BasePixel/TBInterface.h"
#include "BasePixel/TBAnalogInterface.h"

class TestRange;
class Test;

/*!
 * \brief Implementation of the tests at ROC level
 */
class TestRoc // : public Roc
{
public:
    TestRoc(TBInterface* const aTBInterface, const int aChipId, const int aHubId, const int aPortId,
            const int aoutChipPosition);
    ~TestRoc();

	TestDoubleColumn* GetDoubleColumn(int column);
	TestPixel *GetPixel(int col, int row);
	TestPixel *GetTestPixel();
	TestRange *GetRange();
	
// == Setting DACS ======================================	
	
    int AdjustVana(psi::ElectricCurrent current0, psi::ElectricCurrent goalcurrent);
	void AdjustCalDelVthrComp();
	void AdjustCalDelVthrComp(int column, int row, int vcal, int belowNoise);
	void AdjustUltraBlackLevel(int ubLevel);
	
// == Tests =============================================

    void ChipTest();
    void DoTest(Test *aTest);
    void DoIV(Test *aTest);
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
	void SendSignals(int start, int stop, int step, int nTrig, char *dacName);
	void ReadSignals(int start, int stop, int step, int nTrig, int sCurve[]);
	int fitLeft(TH2D *his, int line, int maxEntries);
	int fitRight(TH2D *his, int line, int maxEntries);

// == Histos =============================================
	
	TH1D *DACHisto();
	TH2D* TrimMap();


    void DoubleColumnADCData(int doubleColumn, short data[], int readoutStop[]);
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
    int GetDAC(const char* dacName);
    int GetDAC(int dacReg);
    void SetTrim(int iCol, int iRow, int trimBit);
    void ClrCal();
    int MaskTest(short nTriggers, short res[]);
    void EnablePixel(int col, int row);
    DACParameters* SaveDacParameters();
    void RestoreDacParameters(DACParameters *dacParameters = 0);
    void SetDAC(int reg, int value);
    void SetDAC(const char* dacName, int value);
    void DisableDoubleColumn(int col);
    void Mask();
    void EnableAllPixels();
    void SendADCTrigs(int nTrig);
    bool GetADC(short buffer[], unsigned short buffersize, unsigned short &wordsread, int nTrig, int startBuffer[],
                int &nReadouts);
    bool WriteDACParameterFile(const char *filename);
    void Initialize();
    void RocSetDAC(int reg, int value);
    void CDelay(int clocks);
    TBAnalogInterface* GetTBAnalogInterface() {return (TBAnalogInterface*)tbInterface;}
    TBInterface* GetTBInterface();
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
    bool ReadDACParameterFile( const char *filename);
    void ReadTrimConfiguration( const char *filename);
    void DisablePixel(int col, int row);
    void SetChip();

private:
    TBInterface* tbInterface;
    const int chipId, hubId, portId, aoutChipPosition;
    TestDoubleColumn *doubleColumn[psi::ROCNUMDCOLS];
    DACParameters *dacParameters, *savedDacParameters;
};
