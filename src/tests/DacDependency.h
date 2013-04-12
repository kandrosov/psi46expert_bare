/*!
 * \file DacDependency.h
 * \brief Definition of DacDependency class.
 *
 * \b Changelog
 * 12-04-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - DACParameters class now inherit psi::BaseConifg class.
 *      - Defined enum DacParameters::Register.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
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
