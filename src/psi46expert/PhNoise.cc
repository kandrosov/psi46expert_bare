/*!
 * \file PhNoise.cc
 * \brief Implementation of PhNoise class.
 *
 * \b Changelog
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using definitions from PsiCommon.h.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#include "interface/Log.h"

#include <TMath.h>

#include "PhNoise.h"
#include "TestRoc.h"
#include "TestModule.h"
#include "BasePixel/TBAnalogInterface.h"

bool PhNoise::debug = true;

PhNoise::PhNoise(TestRange *aTestRange, TBInterface *aTBInterface)
{
  psi::Log<psi::Debug>() << "[PhNoise] Initialization." << std::endl;

  testRange = aTestRange;
  tbInterface = aTBInterface;
}

void PhNoise::ModuleAction()
{
  int offset;
  if (((TBAnalogInterface*)tbInterface)->TBMPresent()) offset = 3; else offset = 2; //either tbm black or roc black
  TH1D *black = new TH1D("black", "black", 4000, -2000., 2000.);
  for (int i = 0; i < nReadouts; i++)
  {
    ((TBAnalogInterface*)tbInterface)->ADCRead(data, count, 1);
    if (count > offset) black->Fill(data[offset]);
  }
  
  if (debug) printf("Black %.1f +- %.2f\n", black->GetMean(), black->GetRMS());
  
  Test::ModuleAction();
}


void PhNoise::RocAction()
{
  int data[psi::ROCNUMROWS*psi::ROCNUMCOLS], offset;
  if (((TBAnalogInterface*)tbInterface)->TBMPresent()) offset = 16; else offset = 9;
  int phPosition = offset + aoutChipPosition*3;
  
  TH2D *phMean = GetMap("phMean");
  TH2D *phSquaredMean = GetMap("phSquaredMean");
  TH2D *phVariance =  GetMap("phVariance");

  for (int i = 0; i < nReadouts; i++)
  {
    roc->AoutLevelChip(phPosition, 1, data);
    for (int k = 0; k < psi::ROCNUMROWS*psi::ROCNUMCOLS; k++)
    {
      if (debug && k == 2393) printf("%i ph %i\n", k, data[k]);
      phMean->Fill(k/psi::ROCNUMROWS, k%psi::ROCNUMROWS, data[k]);
      phSquaredMean->Fill(k/psi::ROCNUMROWS, k%psi::ROCNUMROWS, data[k]*data[k]);
    }
  }
  double mean, squaredMean, variance;
  for (int k = 0; k < psi::ROCNUMROWS*psi::ROCNUMCOLS; k++)
  {
    mean = phMean->GetBinContent(k/psi::ROCNUMROWS+1, k%psi::ROCNUMROWS+1);
    squaredMean = phSquaredMean->GetBinContent(k/psi::ROCNUMROWS+1, k%psi::ROCNUMROWS+1);
    variance = TMath::Sqrt((squaredMean - mean*mean/nReadouts)/(nReadouts-1));
    phVariance->Fill(k/psi::ROCNUMROWS, k%psi::ROCNUMROWS, variance);
    if (debug && k == 2393) printf("phMean %e phSquaredMean %e variance %e\n", mean/nReadouts, squaredMean/nReadouts, variance);
  }
  
  histograms->Add(phMean);
  histograms->Add(phSquaredMean);
  histograms->Add(phVariance);
}
