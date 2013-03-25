/*!
 * \file Trim.h
 * \brief Definition of Trim class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
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
