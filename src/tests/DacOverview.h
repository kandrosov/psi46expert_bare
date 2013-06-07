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

class DacOverview : public PhDacScan {
public:

    DacOverview(TestRange *testRange, TBInterface *aTBInterface);

    virtual void ReadTestParameters();
    virtual void RocAction();

    void DoDacScan();

protected:
    int NumberOfSteps, DacType;
};
