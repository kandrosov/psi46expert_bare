/*!
 * \file PHRange.h
 * \brief Definition of PHRange class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#pragma once

#include "BasePixel/Test.h"

/*!
 * \brief Sets the VIbias_roc and the VOffsetOp DAC so that PH is within the range -tbmublevel+safety ... +tbmublevel
 */
class PHRange : public Test
{
public:
    PHRange();
    PHRange(TestRange *testRange, TBInterface *aTBInterface);
    void Init();
    int PHMin();
    int PHMax();
    int PH(int ctrlReg, int vcal, int calDel, int vthrComp, int vtrim, int pixel);

    virtual void ReadTestParameters();
    virtual void RocAction();
    void ValidationPlot();

private:
    int phSafety, tbmUbLevel, minPixel, maxPixel, phPosition;
    int calDelMin, vthrCompMin, calDelMax, vthrCompMax, vcalMin, vcalMax, ctrlRegMin, ctrlRegMax, vtrimMin, vtrimMax;
};
