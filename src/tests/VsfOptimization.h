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

/*!
 * \brief Pulse height dependency on Vsf and VhldDel DACs
 */
class VsfOptimization : public PhDacScan {
public:
    VsfOptimization() {}
    VsfOptimization(TestRange *testRange, TBInterface *aTBInterface);

    virtual void ReadTestParameters();
    virtual void RocAction();

    void DoDacDacScan();
    void VsfOpt();
    int CurrentOpt();
    int CurrentOpt2();
    int Par1Opt();

    int TestCol();

private:
    // Group Input parameters
    struct Input {
        int start;
        int stop;
        int steps;
    };

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
