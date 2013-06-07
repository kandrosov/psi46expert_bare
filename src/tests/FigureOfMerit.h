/*!
 * \file FigureOfMerit.h
 * \brief Definition of FigureOfMerit class.
 */

#pragma once

#include "BasePixel/Test.h"
#include "PhDacScan.h"
#include <TH2D.h>
#include <TH1D.h>
#include <TArrayD.h>

/*!
 * \brief 2 DACs can be scanned as a function of various criteria
 */
class FigureOfMerit : public PhDacScan {
public:
    FigureOfMerit(TestRange *testRange, TBInterface *aTBInterface, DACParameters::Register dac1,
                  DACParameters::Register dac2, int crit);

    virtual void ReadTestParameters();
    virtual void RocAction();
    virtual void PixelAction();

    void DoDacDacScan();
    double Timewalk(int i, int k);
    int LinearRange(int i, int k);
    int PulseHeight(int i, int k);
    double LowLinearRange(int i, int k);
    double DacDacDependency();
    int FindFirstValue(short *result);
    int Threshold(int i, int k);

protected:
    DACParameters::Register firstDac, secondDac;
    int dac1Start, dac1Stop, dac2Start, dac2Stop, dac1Step, dac2Step;
    int criterion, testVcal;
    int dacValue1, dacValue2, index1, index2, bestQuality;

    TH1D *nor;
};
