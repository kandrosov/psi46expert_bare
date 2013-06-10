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
class PhDacOverview : public Test {
public:
    PhDacOverview(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface);
    virtual void RocAction(TestRoc& roc);
    virtual void PixelAction(TestPixel& pixel);

private:
    void DoDacScan(TestPixel& pixel);
    void DoVsfScan(TestRoc& roc);
    void DoTBMRegScan(TestRoc& roc, TBMParameters::Register DacRegister, int offset);

private:
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    PhDacScan phDacScan;
    int NumberOfSteps;
};
