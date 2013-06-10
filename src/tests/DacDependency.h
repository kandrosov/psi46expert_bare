/*!
 * \file DacDependency.h
 * \brief Definition of DacDependency class.
 */

#pragma once

#include "BasePixel/Test.h"
#include <TH2D.h>

class DacDependency : public Test {
public:
    DacDependency(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface);

    virtual void PixelAction(TestPixel& pixel);

    void SetDacs(DACParameters::Register d1, DACParameters::Register d2, int range1, int range2);
    void SetNTrig(int _nTrig) { nTrig = _nTrig; }

private:
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    DACParameters::Register dac1, dac2;
    int nTrig, dacRange1, dacRange2;
};
