/*!
 * \file PHTest.h
 * \brief Definition of PHTest class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - removed deprecated conversion from string constant to char*
 */

#pragma once

#include "BasePixel/Test.h"
#include <TH2D.h>
#include <TH1D.h>

class PHTest : public Test {
public:
    PHTest(TestRange *testRange, TBInterface *aTBInterface);
    virtual void ReadTestParameters();
    virtual void RocAction();
    virtual void PixelAction();
    void PhDac(const char *dacName);

protected:
    int nTrig, mode;
    TH2D *map;
};
