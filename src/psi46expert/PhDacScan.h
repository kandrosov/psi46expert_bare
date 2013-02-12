/*!
 * \file PhDacScan.h
 * \brief Definition of PhDacScan class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#pragma once

#include "Test.h"
#include <TH2D.h>
#include <TH1D.h>
#include <TArrayD.h>

/*!
 * \brief Pulse height dependency on different DACs.
 */
class PhDacScan : public Test
{
 public:
  PhDacScan() {}

  PhDacScan(TestRange *testRange, TBInterface *aTBInterface);
	
  virtual void ReadTestParameters();

  int FitStartPoint(TH1D *histo);
  int FitStopPoint(TH1D *histo, int fitStart);
  double FindLinearRange(TH1D *histo);
  double FindLowLinearRange(TH1D *histo);
  double QualityLowRange(TH1D *histo);
  double Quality(TH1D *histoLowRange, TH1D *histoHighRange);
  void DoDacScan();
  int PH(int vcal, TH1D *histo, TF1* fit);
	
 protected:

  int nTrig, mode, NumberOfSteps;
  double minPh;
  TF1 *fit; 
  TF1 *linFit;
  TF1 *pol2Fit;
};
