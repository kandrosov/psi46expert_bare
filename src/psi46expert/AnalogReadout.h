/*!
 * \file AnalogReadout.h
 * \brief Definition of AnalogReadout class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#pragma once

#include "Test.h"

/*!
 * \brief The AnalogReadout class.
 */
class AnalogReadout : public Test
{
public:
    AnalogReadout(TestRange *testRange, TBInterface *aTBInterface);
	
	virtual void ModuleAction();
	
	void AddressLevelsTest();
	void DualModeTest();
};
