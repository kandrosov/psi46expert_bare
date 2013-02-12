/*!
 * \file TrimBits.h
 * \brief Definition of TrimBits class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#pragma once

#include "Test.h"

/*!
 * \brief Trim bit test
 */
class TrimBits : public Test
{
public:
    TrimBits(TestRange *testRange, TBInterface *aTBInterface);
    virtual void ReadTestParameters();
	virtual void RocAction();
	
protected:
	int nTrig, vtrim14, vtrim13, vtrim11, vtrim7;
};
