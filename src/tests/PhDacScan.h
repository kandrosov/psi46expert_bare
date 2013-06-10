/*!
 * \file PhDacScan.h
 * \brief Definition of PhDacScan class.
 */

#pragma once

#include <TH2D.h>
#include <TH1D.h>
#include <TArrayD.h>

/*!
 * \brief Pulse height dependency on different DACs.
 */
class PhDacScan {
public:
    PhDacScan();

    int FitStartPoint(TH1D *histo);
    int FitStopPoint(TH1D *histo, int fitStart);
    double FindLinearRange(TH1D *histo);
    double FindLowLinearRange(TH1D *histo);
    double QualityLowRange(TH1D *histo);
    double Quality(TH1D *histoLowRange, TH1D *histoHighRange);
    int PH(int vcal, TH1D *histo, TF1* fit);

    int GetNTrig() const { return nTrig; }
    double GetMinPh() const { return minPh; }

private:
    bool debug;
    int nTrig, mode, NumberOfSteps;
    double minPh;
    TF1 *fit;
    TF1 *linFit;
    TF1 *pol2Fit;
};
