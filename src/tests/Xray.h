/*!
 * \file Xray.h
 * \brief Definition of Xray class.
 */

#pragma once

#include "BasePixel/constants.h"
#include "BasePixel/Test.h"

/*!
 * \brief Vcal calibration with xray
 */
class Xray : public Test {
public:
    Xray(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface);
    virtual void ModuleAction(TestModule& module);
    virtual void RocAction(TestRoc& roc);

private:
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    int nTrig, vthrCompMin, vthrCompMax;
    double maxEff;
    TH1F *histo[psi::MODULENUMROCS];
};
