/*!
 * \file OffsetOptimization.h
 * \brief Definition of OffsetOptimization class.
 */

#pragma once

#include "BasePixel/Test.h"
#include "PhDacScan.h"
#include <TH2D.h>
#include <TH1D.h>
#include <TArrayD.h>

/*!
 * \brief Optimization of the Dynamic Range for VoffsetOp and VOffsetR0.
 */
class OffsetOptimization : public Test {
public:
    OffsetOptimization(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface);
    virtual void RocAction(TestRoc& roc);
    virtual void PixelAction(TestPixel& pixel);

private:
    void DoDacDacScan(TestPixel& pixel);

private:
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    PhDacScan phDacScan;
    int dac1Start, dac1Stop, dac1Step, dac2Start, dac2Stop, dac2Step;
};
