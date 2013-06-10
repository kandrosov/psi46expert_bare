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
    SCurveTest(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface);

    virtual void ModuleAction(TestModule& module);
    virtual void RocAction(TestRoc& roc);
    virtual void DoubleColumnAction(TestDoubleColumn& doubleColumn);

private:
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    int nTrig, mode, vthr, vcal, sCurve[16 * psi::ROCNUMROWS * 256];
    int dacReg;
    std::string mapName;
    TH2D *map[psi::MODULENUMROCS];
    bool testDone;
    FILE *file[psi::MODULENUMROCS];

};
