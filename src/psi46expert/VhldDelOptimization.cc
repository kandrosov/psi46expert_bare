/*!
 * \file VhldDelOptimization.cc
 * \brief Implementation of VhldDelOptimization class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#include <iostream>
#include <stdio.h>
#include "TF1.h"
#include "TH1D.h"
#include "TH2.h"
#include "TCanvas.h"
#include "interface/Delay.h"
#include "interface/Log.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/ConfigParameters.h"
#include "TestRoc.h"
#include "PhDacScan.h"
#include "VhldDelOptimization.h"
#include "OffsetOptimization.h"
#include "FigureOfMerit.h"
#include "TestParameters.h"

VhldDelOptimization::VhldDelOptimization(TestRange *aTestRange, TBInterface *aTBInterface)
{
    testRange = aTestRange;
    tbInterface = aTBInterface;
    ReadTestParameters();
}

void VhldDelOptimization::ReadTestParameters()
{
    debug = true;
}

void VhldDelOptimization::RocAction()
{
printf("VhldDelOptimization roc %i\n", chipId);
PixelLoop();
}

void VhldDelOptimization::PixelLoop()
{

  TH1D *VhldDelHist = new TH1D("VhldDel","VhldDel",26,0,260);
  TestRange *pixelRange = new TestRange();
  int bestHldDel;  
  
  for (int col = 0; col < 5; col++)
    {
      for (int row = 0; row < 5; row++)
	{
      pixelRange->AddPixel(chipId, col,row);
      bestHldDel = AdjustVhldDel(pixelRange);
      VhldDelHist->Fill(bestHldDel);
	}
    }
  histograms->Add(VhldDelHist);
}


int VhldDelOptimization::AdjustVhldDel(TestRange *pixelRange) 
{
  
  SetDAC("CtrlReg", 4);
  Flush();

  const int vsfValue = 150, hldDelMin = 0, hldDelMax = 200, hldDelStep = 10;

  TestParameters& testParameters = TestParameters::ModifiableSingleton();
  testParameters.setPHdac1Start(vsfValue);   // Vsf
  testParameters.setPHdac1Stop(vsfValue);
  testParameters.setPHdac1Step(10);
  testParameters.setPHdac2Start(hldDelMin);     // VhldDel
  testParameters.setPHdac2Stop(hldDelMax);
  testParameters.setPHdac2Step(hldDelStep);
  
  SaveDacParameters();
  
  Test *fom = new FigureOfMerit(pixelRange, tbInterface, 3, 10, 3);
  fom->RocAction(roc);
  TList *histos = fom->GetHistos();
  TIter next(histos);
  if (debug) while (TH1 *histo = (TH1*)next()) histograms->Add(histo);
  delete fom;

  std::cout << "dac1 = " << GetDAC(3) << " DAC1 = " << GetDAC(10) << std::endl;

  TH2D *qualityHist2D = (TH2D*)(histos->Last());
  int nBins = (hldDelMax-hldDelMin)/hldDelStep;
  TH1D *qualityHist1D = new TH1D("VhldDel","VhldDel",nBins,hldDelMin,hldDelMax);

  double maxLinearity = 0;
  double maxBin = -1;

  for (int n = 0; n <= nBins; n++) 
    {
      double linearRange = qualityHist2D->GetBinContent(1,n+1);
      qualityHist1D->SetBinContent(n+1,linearRange);
      if (linearRange > maxLinearity)
	{
	  maxLinearity = linearRange;
	  maxBin = hldDelMin + n * hldDelStep;
	}
    }
  std::cout << "max Linearity = " << maxLinearity << " @ VhldDel = " << maxBin <<  std::endl;
  histograms->Add(qualityHist1D);
  RestoreDacParameters();  
  
  hldDelValue = static_cast<int>( maxBin);

  SetDAC("VhldDel", hldDelValue);
  std::cout << "VhldDel set to " << hldDelValue << std::endl;

  return hldDelValue;
}
