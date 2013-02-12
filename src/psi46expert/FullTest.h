/*!
 * \file FullTest.h
 * \brief Definition of FullTest class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new ConfigParameters class definition.
 */

#include "Test.h"

/*!
 * \brief Full test of a module
 */
class FullTest : public Test
{
public:
    FullTest(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface,int opt);
	int Scurve;
	virtual void RocAction();
	virtual void ModuleAction();
	void DoTemperatureTest();
		
protected:
	TestParameters *testParameters;
};
