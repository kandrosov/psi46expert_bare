/*!
 * \file PhNoise.h
 * \brief Definition of PhNoise class.
 *
 * \b Changelog
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using definitions from PsiCommon.h.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#pragma once

#include <TH1D.h>
#include "Test.h"

/*!
 * \brief Measure the noise on the pulse height in the analog out signal
 */
class PhNoise : public Test
{
public:
    PhNoise(TestRange *testRange, TBInterface *aTBInterface);
    virtual void ModuleAction();
	virtual void RocAction();
	
protected:
	unsigned short count;
    short data[psi::FIFOSIZE];
    static const int nReadouts = 1000;
	static bool debug;
};
