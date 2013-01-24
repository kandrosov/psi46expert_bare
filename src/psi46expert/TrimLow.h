/*!
 * \file TrimLow.h
 * \brief Definition of TrimLow class.
 *
 * \b Changelog
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - removed deprecated conversion from string constant to char*
 */

#ifndef TRIMLOW
#define TRIMLOW

#include "Test.h"
#include <TH2D.h>
#include "ThresholdMap.h"

/*!
 * \brief Trim functions
 */
class TrimLow : public Test
{

public:
	TrimLow(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface);
	
	virtual void ReadTestParameters(TestParameters *testParameters);
	virtual void RocAction();
    double MinVthrComp(const char *mapName);
	int AdjustVtrim();
	void AddMap(TH2D* calMap);
	TH2D* TrimStep(int correction, TH2D *calMapOld, TestRange* aTestRange);
        void NoTrimBits(bool aBool);
        void SetVcal(int vcal);

		
protected:

	int vthrComp, doubleWbc, nTrig, vcal;
        bool noTrimBits;
	ThresholdMap *thresholdMap;

};


#endif

