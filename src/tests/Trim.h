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
    Trim(TestRange *testRange, TBInterface *aTBInterface);
    virtual void ReadTestParameters();
    virtual void RocAction();
    int AdjustVtrim();
    void AddMap(TH2D* calMap);
    TH2D* TrimStep(int correction, TH2D *calMapOld, TestRange* aTestRange);

protected:
    int vthrComp, doubleWbc, nTrig, vcal;
    ThresholdMap *thresholdMap;
};
