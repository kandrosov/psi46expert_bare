/*!
 * \file ThresholdTest.h
 * \brief Definition of ThresholdTest class.
 */

#pragma once

#include "BasePixel/Test.h"
#include <TH2D.h>

/*!
 * \brief Trim functions
 */
class ThresholdTest : public Test {
public:
    ThresholdTest(TestRange *testRange, TBInterface *aTBInterface);
    virtual void ReadTestParameters();
    virtual void RocAction();

protected:
    int nTrig, mode, vthr, vcal;
    char *dacName;
    TH2D *map;
};
