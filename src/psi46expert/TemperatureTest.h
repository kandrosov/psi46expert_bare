/*!
 * \file TemperatureTest.h
 * \brief Definition of TemperatureTest class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#pragma once

#include "Test.h"

class TemperatureTest : public Test
{
public:
    TemperatureTest(TestRange *testRange, TBInterface *aTBInterface);
    virtual void ReadTestParameters();
	virtual void RocAction();
	
protected:
	int nTrig;
};
