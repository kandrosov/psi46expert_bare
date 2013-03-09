/*!
 * \file PhDacScan.cc
 * \brief Implementation of PhDacScan class.
 *
 * \b Changelog
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using definitions from PsiCommon.h.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#include "PhDacScan.h"
#include "psi46expert/TestRoc.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/constants.h"
#include <TMath.h>
#include "BasePixel/TestParameters.h"
#include <TF1.h>
PhDacScan::PhDacScan(TestRange *aTestRange, TBInterface *aTBInterface)
{
  testRange = aTestRange;
  tbInterface = aTBInterface;
  ReadTestParameters();

  fit = new TF1("Fit", "pol4");
  linFit = new TF1("linFit", "pol1");
  pol2Fit = new TF1("pol2Fit", "pol2");

  debug = false;
}

void PhDacScan::ReadTestParameters()
{
    const TestParameters& testParameters = TestParameters::Singleton();
    mode = testParameters.PHMode();
    nTrig = testParameters.PHNTrig();
}

int PhDacScan::FitStartPoint(TH1D *histo)
{
  int bin = 255;
  while ((histo->GetBinContent(bin+1) != 7777) && (bin > 0)){
    bin--;
  }
  bin += 2;
  
  while ( histo->GetBinContent(bin) == 0 && (bin < 255)) bin++;
  
  if (bin > 255) return 255;
  return bin;
}

int PhDacScan::FitStopPoint(TH1D *histo, int fitStart)
{
  int ibin = 256;
  while ( histo->GetBinContent(ibin) == 0 && (ibin > 0)) ibin--;
  double saturation = histo->GetBinContent(ibin);
  double saturationLow = histo->GetBinContent(fitStart);
  double diff = 0.1*(saturation - saturationLow);
  if (diff < 8) diff = 8;
  int bin = 255;
  
  //while (((histo->GetBinContent(bin+1) > saturation - diff) || histo->GetBinContent(bin+1) == 0) && (bin > 0)) bin--;
  while (((histo->GetBinContent(bin) > saturation - diff) || histo->GetBinContent(bin) == 0) && (bin > 0)) bin--;
  bin += 40;
  
  if (bin > 255) return 255;
  return bin;
}

double PhDacScan::FindLinearRange(TH1D *histo)
{
  // start and stop point for fit

  int fitStart = FitStartPoint(histo);
  int fitStop = FitStopPoint(histo, fitStart);

  if (debug)
      psi::LogInfo() << "fitStart " << fitStart << " fitStop " << fitStop << std::endl;

  if (histo->GetBinContent(fitStop+1) - histo->GetBinContent(fitStart+1) < 40) return 0;
  
  fit->SetRange(fitStart, fitStop);
  double slope = (histo->GetBinContent(fitStop+1)-histo->GetBinContent(fitStart+1))/(fitStop - fitStart);
  fit->SetParameter(0, histo->GetBinContent(fitStart+1)-slope*fitStart);
  fit->SetParameter(1, -3.);
  if (debug) histo->Fit("Fit","R"); else histo->Fit("Fit","RQN");
  
  // find inflection point
  
  double a = fit->GetParameter(4);
  double b = fit->GetParameter(3);
  double c = fit->GetParameter(2);
  double d = fit->GetParameter(1);
  
  int minOffset = (fitStop - fitStart) / 5;
  if (debug) psi::LogInfo() << "minOffset " << minOffset << std::endl;
 
  double derivative;
  int x, x2;
  for (x = fitStop - 40; x > fitStart + minOffset; x--)
    {
      derivative = 12*a*x*x + 6*b*x + 2*c;
      if (derivative > 0) break;
    }

  for (x2 = fitStart + minOffset; x2 < fitStop - 40; x2++)
    {
      derivative = 12*a*x2*x2 + 6*b*x2 + 2*c;
      if (derivative < 0) break;
    }
  x2--;
  
  if ((x2 != fitStop - 2) && (x != x2) && (x != fitStart + minOffset - 1)) 
    {
      if (debug)
          psi::LogInfo() << "Second inflection point found " << x << " " << x2 << std::endl;
      double mean = (histo->GetBinContent(fitStop+1) + histo->GetBinContent(fitStart+1))/2;
      if (TMath::Abs(histo->GetBinContent(x2) - mean) < TMath::Abs(histo->GetBinContent(x) - mean)) x = x2;	
    }
  
  if (debug)
      psi::LogInfo() << "inflection point " << x << std::endl;

  if (x < fitStart + minOffset) x = fitStart+minOffset;
  if (debug)
      psi::LogInfo() << "x " << x << std::endl;
  
  // create linear fit in inflection point
  
  slope = 4*a*x*x*x + 3*b*x*x + 2*c*x + d;
  linFit->SetRange(fitStart, fitStop);
  linFit->FixParameter(1, slope);
  //double phx = (histo->GetBinContent(x) + histo->GetBinContent(x+1) + histo->GetBinContent(x+2))/3;
  int ibin = x;
  while ( histo->GetBinContent(ibin) == 0 && (ibin < 255)) ibin++;
  double phx = histo->GetBinContent(ibin);
  linFit->FixParameter(0, phx - slope*x);
  
  if (debug)
    {
      TH1D *histo2 = (TH1D*)histo->Clone();
      histo2->Fit("linFit","QRB");
      histograms->Add(histo2);
    }
  minPh = fit->Eval(20);

  
  // find beginning and end of the linear range
    
  //double diff = (histo->GetBinContent(fitStop+1) - histo->GetBinContent(fitStart+1)) / 10;
  double diff = (histo->GetBinContent(fitStop) - histo->GetBinContent(fitStart)) / 10;
  if (debug) psi::LogInfo() << "bin-content(fitStart) = " << histo->GetBinContent(fitStart) << std::endl;
  if (debug) psi::LogInfo() << "bin-content(fitStop) = " << histo->GetBinContent(fitStop) << std::endl;
  int stopVcal = x, stopPh = PH(stopVcal, histo, fit);
  while((TMath::Abs(stopPh - linFit->Eval(stopVcal)) < diff) && (stopVcal < 255))
    {
      stopVcal++;
      stopPh = PH(stopVcal, histo, fit);
    }

  int startVcal = x;
  int startPh = PH(startVcal, histo, fit);
  while(TMath::Abs(startPh - linFit->Eval(startVcal)) < diff)
    {
      startVcal--;
      startPh = PH(startVcal, histo, fit);
    }
  startVcal++;
  startPh = PH(startVcal, histo, fit);

  if (debug)
      psi::LogInfo() << "linear range in Vcal direction from " << startVcal << " to " << stopVcal << ", distance "
                     << (stopVcal - startVcal) << std::endl;
  if (debug)
      psi::LogInfo() << "linear range in PH direction from " << startPh << " to " << stopPh << ", distance "
                     << (stopPh - startPh) << std::endl;

  return sqrt((double)((stopVcal-startVcal)*(stopVcal-startVcal) + (stopPh-startPh)*(stopPh-startPh)));  
  //  return stopVcal-startVcal;  

}


int PhDacScan::PH(int vcal, TH1D *histo, TF1* fit)
{
  int binHigh = vcal;
  while ( histo->GetBinContent(binHigh) == 0 && binHigh < 255 ) binHigh++;
  int binLow = vcal;
  while ( histo->GetBinContent(binLow) == 0 && binLow > 0 ) binLow--;

  int bin = 0;
  if ( (binHigh - vcal) < (vcal - binLow) ) bin = binHigh;
  else bin = binLow;

  //if (TMath::Abs(histo->GetBinContent(vcal+1) - fit->Eval(vcal)) < 10) return (int)fit->Eval(vcal);
  //else return TMath::Nint(histo->GetBinContent(vcal+1));

  if (TMath::Abs(histo->GetBinContent(bin) - fit->Eval(bin)) < 10) return (int)fit->Eval(vcal);
  else return TMath::Nint(histo->GetBinContent(bin));
}

double PhDacScan::QualityLowRange(TH1D *histo)
{
  int fitStart = FitStartPoint(histo), fitStop = 250;	
  pol2Fit->SetRange(fitStart, fitStop);
  pol2Fit->SetParLimits(2,0,1e6);
  pol2Fit->SetParLimits(1,0,1e6);
  TH1D *pol2FitHisto = (TH1D*)histo->Clone();
  pol2FitHisto->Fit("pol2Fit","RQB");

  double aoverb = pol2Fit->GetParameter(2)/pol2Fit->GetParameter(1);

  histograms->Add(pol2FitHisto);

  return -TMath::Abs(aoverb);
}

double PhDacScan::Quality(TH1D *histoLowRange, TH1D *histoHighRange)
{
  const double scale = 1e5; 
  return -TMath::Abs(QualityLowRange(histoLowRange))*scale + FindLinearRange(histoHighRange);
}

double PhDacScan::FindLowLinearRange(TH1D *histo)
{
  double aoverb =  QualityLowRange(histo);
  
  TH1D *linFitHisto = (TH1D*)histo->Clone();
  
  //  psi::LogInfo() << "aoverb = " << aoverb << endl;
  
  if (TMath::Abs(aoverb) > 1) return 0;
  int fitStart = FitStartPoint(linFitHisto), fitStop = 250;
  
  linFit->SetRange(fitStart, fitStop);
  linFitHisto->Fit("linFit","RQ");
  histograms->Add(linFitHisto);
  
  if(linFit->GetParameter(1) < 0) return 0;

  //double diffPh = (linFit->Eval(fitStop)-linFit->Eval(fitStart));
  //double diffVcal = fitStop - fitStart;
  //double linearRange = TMath::Sqrt(diffPh * diffPh + diffVcal * diffVcal);

  return TMath::Abs(aoverb);

}
