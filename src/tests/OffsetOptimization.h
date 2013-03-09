/*!
 * \file OffsetOptimization.h
 * \brief Definition of OffsetOptimization class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
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
class OffsetOptimization : public PhDacScan
{
public:
    OffsetOptimization(TestRange *testRange, TBInterface *aTBInterface);
    virtual void ReadTestParameters();
    virtual void RocAction();
    virtual void PixelAction();
    void DoDacDacScan();

protected:
    int dac1Start, dac1Stop, dac1Step, dac2Start, dac2Stop, dac2Step;
};
