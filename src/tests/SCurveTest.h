/*!
 * \file SCurveTest.h
 * \brief Definition of SCurveTest class.
 */

#pragma once

#include "BasePixel/Test.h"
#include <TH2D.h>

/*!
 * \brief SCurve measurements
 */
class SCurveTest : public Test {

public:
    SCurveTest(TestRange *testRange, TBInterface *aTBInterface);

    virtual void ReadTestParameters();
    virtual void ModuleAction();
    virtual void RocAction();
    virtual void DoubleColumnAction();

protected:

    int nTrig, mode, vthr, vcal, sCurve[16 * psi::ROCNUMROWS * 256];
    int dacReg;
    std::string mapName;
    TH2D *map[psi::MODULENUMROCS];
    bool testDone;
    FILE *file[psi::MODULENUMROCS];

};
