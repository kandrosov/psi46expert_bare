/*!
 * \file PHTest.h
 * \brief Definition of DacDependency class.
 *
 * \b Changelog
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - removed deprecated conversion from string constant to char*
 */

#ifndef PHTEST
#define PHTEST

#include "Test.h"
#include <TH2D.h>
#include <TH1D.h>

class PHTest : public Test
{

public:
	PHTest(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface);
	
	virtual void ReadTestParameters(TestParameters *testParameters);
	virtual void RocAction();
	virtual void PixelAction();
	
    void PhDac(const char *dacName);

	
	
protected:

	int nTrig, mode;
	TH2D *map;

};


#endif

