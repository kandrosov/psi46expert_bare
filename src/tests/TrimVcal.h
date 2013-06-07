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
    TrimVcal(TestRange *testRange, TBInterface *aTBInterface);
    virtual void ReadTestParameters();
    virtual void RocAction();
    void AddMap(TH2D* calMap);

protected:
    int nTrig;
    ThresholdMap *thresholdMap;
};
