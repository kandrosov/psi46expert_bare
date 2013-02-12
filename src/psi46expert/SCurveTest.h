/*!
 * \file SCurveTest.h
 * \brief Definition of SCurveTest class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - removed deprecated conversion from string constant to char*
 */

#ifndef SCURVETEST
#define SCURVETEST

#include "Test.h"
#include <TH2D.h>

/*!
 * \brief SCurve measurements
 */
class SCurveTest : public Test
{

public:
    SCurveTest(TestRange *testRange, TBInterface *aTBInterface);
	
    virtual void ReadTestParameters();
	virtual void ModuleAction();
	virtual void RocAction();
	virtual void DoubleColumnAction();
		
protected:

	int nTrig, mode, vthr, vcal, sCurve[16*ROCNUMROWS*256];
	int dacReg;
    std::string mapName;
	TH2D *map[MODULENUMROCS];
	bool testDone;
	FILE *file[MODULENUMROCS];

};


#endif

