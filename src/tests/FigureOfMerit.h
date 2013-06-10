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
class FigureOfMerit : public Test {
public:
    FigureOfMerit(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface, DACParameters::Register dac1,
                  DACParameters::Register dac2, int crit);

    virtual void RocAction(TestRoc& roc);
    virtual void PixelAction(TestPixel& pixel);

private:
    void DoDacDacScan(TestPixel& pixel);
    double Timewalk(TestRoc& roc, int i, int k);
    int LinearRange(TestRoc& roc, int i, int k);
    int PulseHeight(TestRoc& roc, int i, int k);
    double LowLinearRange(TestRoc& roc, int i, int k);
    int Threshold(TestRoc& roc, int i, int k);
    static int FindFirstValue(short *result);


private:
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    PhDacScan phDacScan;
    DACParameters::Register firstDac, secondDac;
    int dac1Start, dac1Stop, dac2Start, dac2Stop, dac1Step, dac2Step;
    int criterion, testVcal;
    int dacValue1, dacValue2, index1, index2, bestQuality;

    TH1D *nor;
};
