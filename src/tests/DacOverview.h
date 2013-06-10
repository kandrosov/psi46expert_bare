/*!
 * \file DacOverview.h
 * \brief Definition of DacOverview class.
 */

#pragma once

#include "BasePixel/Test.h"
#include "PhDacScan.h"
#include <TH2D.h>
#include <TH1D.h>
#include <TArrayD.h>

class DacOverview : public Test {
public:

    DacOverview(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface);

    virtual void RocAction(TestRoc& roc);

private:
    void DoDacScan(TestPixel& pixel);

private:
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    int NumberOfSteps, DacType;
};
