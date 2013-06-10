/*!
 * \file Trim.h
 * \brief Definition of Trim class.
 */

#pragma once

#include "BasePixel/Test.h"
#include <TH2D.h>
#include "BasePixel/ThresholdMap.h"

/*!
 * \brief Trim functions
 */
class Trim : public Test {
public:
    Trim(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface);
    virtual void RocAction(TestRoc& roc);

private:
    int AdjustVtrim(TestPixel& pixel);
    void AddMap(TH2D* calMap);
    TH2D* TrimStep(TestRoc& roc, int correction, TH2D *calMapOld);

private:
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    int vthrComp, doubleWbc, nTrig, vcal;
    ThresholdMap thresholdMap;
    unsigned numberOfVcalThresholdMaps;
    unsigned numberOfTrimMaps;
};
