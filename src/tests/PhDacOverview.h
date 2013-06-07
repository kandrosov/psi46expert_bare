/*!
 * \file PhDacOverview.h
 * \brief Definition of PhDacOverview class.
 */

#pragma once

#include "BasePixel/Test.h"
#include "PhDacScan.h"
#include "BasePixel/TBMParameters.h"
#include <TH2D.h>
#include <TH1D.h>
#include <TArrayD.h>

/*!
 * \brief pulse height - Vcal overview for different DACs
 */
class PhDacOverview : public PhDacScan {
public:
    PhDacOverview(TestRange *testRange, TBInterface *aTBInterface);

    virtual void ReadTestParameters();
    virtual void RocAction();
    virtual void PixelAction();

private:
    void DoDacScan();
    void DoVsfScan();
    void DoTBMRegScan(TBMParameters::Register DacRegister, int offset);
private:
    int NumberOfSteps;
};
