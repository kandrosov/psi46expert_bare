/*!
 * \file VsfOptimization.h
 * \brief Definition of VsfOptimization class.
 */

#pragma once

#include "PhDacScan.h"
#include <TH2D.h>
#include <TH1D.h>
#include <TArrayD.h>
#include "psi/units.h"
#include "BasePixel/Test.h"

/*!
 * \brief Pulse height dependency on Vsf and VhldDel DACs
 */
class VsfOptimization : public Test {
public:
    VsfOptimization(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface);
    virtual void RocAction(TestRoc& roc);

private:
    void DoDacDacScan(TestRoc& roc);
    void VsfOpt(TestRoc& roc);
    int CurrentOpt(TestRoc& roc);
    int CurrentOpt2(TestRoc& roc);
    int Par1Opt(TestRoc& roc);
    int TestCol(TestRoc& roc);

private:
    // Group Input parameters
    struct Input {
        int start;
        int stop;
        int steps;
    };

    boost::shared_ptr<TBAnalogInterface> tbInterface;
    PhDacScan phDacScan;

    TArrayI bestVsf_pixel;
    TArrayI bestVhldDel_pixel;
    TArrayF bestQuality_pixel;
    int     bestVsf_ROC;
    int     bestVhldDel_ROC;

    int     par1Vsf;
    int     optVsf;

    Input  vsf;
    Input  vhldDel;

    psi::ElectricCurrent goalCurrent;
    double goalPar1;
};
