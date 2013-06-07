/*!
 * \file PHTest.h
 * \brief Definition of PHTest class.
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
