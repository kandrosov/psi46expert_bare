/*!
 * \file PHRange.h
 * \brief Definition of PHRange class.
 */

#pragma once

#include "BasePixel/Test.h"

/*!
 * \brief Sets the VIbias_roc and the VOffsetOp DAC so that PH is within the range -tbmublevel+safety ... +tbmublevel
 */
class PHRange : public Test {
public:
    PHRange(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface);

    virtual void RocAction(TestRoc& roc);

private:
    void Init(TestRoc& roc);
    int PHMin(TestRoc& roc);
    int PHMax(TestRoc& roc);
    int PH(TestRoc& roc, int ctrlReg, int vcal, int calDel, int vthrComp, int vtrim, int pixel);
    void ValidationPlot(TestRoc& roc);

private:
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    int phSafety, tbmUbLevel, minPixel, maxPixel, phPosition;
    int calDelMin, vthrCompMin, calDelMax, vthrCompMax, vcalMin, vcalMax, ctrlRegMin, ctrlRegMax, vtrimMin, vtrimMax;
};
