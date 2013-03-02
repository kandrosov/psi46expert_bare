/*!
 * \file ThresholdMap.h
 * \brief Definition of ThresholdMap class.
 *
 * \b Changelog
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - removed deprecated conversion from string constant to char*
 */

#ifndef THRESHOLDMAP
#define THRESHOLDMAP

#include <TH2D.h>
#include "psi46expert/TestRoc.h"
#include "TestRange.h"

/*!
 * \brief Trim functions
 */
class ThresholdMap
{

public:
	ThresholdMap();
	
    TH2D* GetMap(const char* mapName, TestRoc *roc, TestRange *testRange, int nTrig);
// 	bool CheckMap();
    void MeasureMap(const char* mapName, TestRoc *roc, TestRange *testRange, int nTrig);
    void SetParameters(const char *mapName);
	void SetCals();
	void SetXTalk();
	void SetDoubleWbc();
	void SetSingleWbc();
	void SetReverseMode();
		
protected:

	TH2D *histo;
	int dacReg;
	bool cals, reverseMode, xtalk, doubleWbc;

};


#endif

