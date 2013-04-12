/*!
 * \file PhDacOverview.h
 * \brief Definition of PhDacOverview class.
 *
 * \b Changelog
 * 12-04-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Defined enum TBMParameters::Register.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
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
