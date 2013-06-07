/*!
 * \file Xray.h
 * \brief Definition of Xray class.
 */

#pragma once

#include "BasePixel/Test.h"

/*!
 * \brief Vcal calibration with xray
 */
class Xray : public Test {
public:
    Xray(TestRange *testRange, TBInterface *aTBInterface);
    virtual void ReadTestParameters();
    virtual void ModuleAction();
    virtual void RocAction();

protected:
    int nTrig, vthrCompMin, vthrCompMax;
    double maxEff;
    TH1F *histo[psi::MODULENUMROCS];
};
