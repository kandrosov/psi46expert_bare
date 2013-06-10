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
    VhldDelOptimization(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface);
    virtual void RocAction(TestRoc& roc);

private:
    int AdjustVhldDel(TestRoc& roc, PTestRange pixelRange);

private:
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    int hldDelValue;
};
