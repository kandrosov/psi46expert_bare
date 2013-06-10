/*!
 * \file ThresholdTest.h
 * \brief Definition of ThresholdTest class.
 */

#pragma once

#include "BasePixel/Test.h"
#include <TH2D.h>

/*!
 * \brief Trim functions
 */
class ThresholdTest : public Test {
public:
    ThresholdTest(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface);
    virtual void RocAction(TestRoc& roc);

private:
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    int nTrig, mode, vthr, vcal;
    char *dacName;
    TH2D *map;
};
