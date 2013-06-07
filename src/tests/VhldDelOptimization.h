/*!
 * \file VhldDelOptimization.h
 * \brief Definition of VhldDelOptimization class.
 */

#pragma once

#include "BasePixel/Test.h"

/*!
 * \brief Adjust VhldDel by optimizing the Linearity in the low range
 */
class VhldDelOptimization : public Test {
public:
    VhldDelOptimization();
    VhldDelOptimization(TestRange *testRange, TBInterface *aTBInterface);

    void Initialize();
    virtual void ReadTestParameters();
    virtual void RocAction();
    int AdjustVhldDel(TestRange *pixelRange);
    void PixelLoop();

private:
    int hldDelValue;
};
