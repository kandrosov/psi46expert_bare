/*!
 * \file TrimVcal.h
 * \brief Definition of TrimVcal class.
 */

#pragma once

#include "BasePixel/Test.h"
#include <TH2D.h>
#include "BasePixel/ThresholdMap.h"

/*!
 * \brief Trim functions
 */
class TrimVcal : public Test {
public:
    TrimVcal(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface);
    virtual void RocAction(TestRoc& roc);

private:
    void AddMap(TH2D* calMap);

private:
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    int nTrig;
    ThresholdMap thresholdMap;
};
