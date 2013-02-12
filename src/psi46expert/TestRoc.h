/*!
 * \file TestRoc.h
 * \brief Definition of TestRoc class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#pragma once

#include "BasePixel/Roc.h"
#include "TestDoubleColumn.h"

class TestRange;
class Test;

/*!
 * \brief Implementation of the tests at ROC level
 */
class TestRoc : public Roc
{
public:
    TestRoc(TBInterface* const aTBInterface, const int aChipId, const int aHubId, const int aPortId, const int aoutChipPosition);
	void Execute(SysCommand &command);

	TestDoubleColumn* GetDoubleColumn(int column);
	TestPixel *GetPixel(int col, int row);
	TestPixel *GetTestPixel();
	TestRange *GetRange();
	
// == Setting DACS ======================================	
	
	int AdjustVana(double current0, double goalcurrent);
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
};
