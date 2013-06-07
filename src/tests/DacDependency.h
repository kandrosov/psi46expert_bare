/*!
 * \file DacDependency.h
 * \brief Definition of DacDependency class.
 */

#pragma once

#include "BasePixel/Test.h"
#include <TH2D.h>

class DacDependency : public Test {
public:
    DacDependency(TestRange *testRange, TBInterface *aTBInterface);

    virtual void ReadTestParameters();
    virtual void PixelAction();
    void SetDacs(DACParameters::Register d1, DACParameters::Register d2, int range1, int range2);
    void SetNTrig(int nTrig);

protected:
    DACParameters::Register dac1, dac2;
    int nTrig, dacRange1, dacRange2;


};
