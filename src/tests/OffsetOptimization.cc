/*!
 * \file OffsetOptimization.cc
 * \brief Implementation of OffsetOptimization class.
 *
 * \b Changelog
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using definitions from PsiCommon.h.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#include "TCanvas.h"
#include <TF1.h>
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/constants.h"
#include "psi/log.h"
#include "OffsetOptimization.h"
#include "psi46expert/TestRoc.h"
#include "PhDacScan.h"
#include "BasePixel/TestParameters.h"

OffsetOptimization::OffsetOptimization(TestRange *aTestRange, TBInterface *aTBInterface)
  : PhDacScan(aTestRange, aTBInterface)
{
  testRange = aTestRange;
  tbInterface = aTBInterface;
  ReadTestParameters();
  fit = new TF1("Fit", "pol4");
  debug = false;
}


void OffsetOptimization::ReadTestParameters()
{
    PhDacScan::ReadTestParameters();
    const TestParameters& testParameters = TestParameters::Singleton();
    dac1Start = testParameters.PHdac1Start();
    dac1Stop  = testParameters.PHdac1Stop();
    dac1Step  = testParameters.PHdac1Step();
    dac2Start = testParameters.PHdac2Start();
    dac2Stop  = testParameters.PHdac2Stop();
    dac2Step  = testParameters.PHdac2Step();
}


void OffsetOptimization::RocAction()
{
  SaveDacParameters();
  SetDAC("CtrlReg",4);	
  Test::RocAction();
  RestoreDacParameters();
}


void OffsetOptimization::PixelAction()
{
  ArmPixel();
  Flush();
  DoDacDacScan();
  DisarmPixel();
}


void OffsetOptimization::DoDacDacScan()
{
  psi::Log<psi::Debug>() << "[OffsetOptimization] DAC DAC Scan" << std::endl;

  int dacValue1Size = (dac1Stop - dac1Start)/dac1Step; // VOffsetR0
  int dacValue2Size = (dac2Stop - dac2Start)/dac2Step; // VOffsetOp
  int linearRange;
  short result[256];
  int optimalOp, optimalR0;
  int VcalRangeMax = -99;
  int index1, index2;
    
  TH2D *histo2 = new TH2D( Form( "Linear_Range_of_Vcal_c%ir%i_C%i", column, row, chipId),
                           Form("Linear_Range_of_Vcal_c%ir%i_C%i", column, row, chipId),
                           dacValue1Size + 1, dac1Start, dac1Stop + dac1Step,
                           dacValue2Size + 1, dac2Start, dac2Stop + dac2Step);
  histo2->GetXaxis()->SetTitle("VOffsetR0 [DAC units]");
  histo2->GetYaxis()->SetTitle("VoffsetOp [DAC units]");
  histo2->GetZaxis()->SetTitle("linear range");

  TH2D *minPhHisto = new TH2D( Form( "Min_PH_c%ir%i_C%i", column, row, chipId),
                               Form( "Min_PH_c%ir%i_C%i", column, row, chipId),
                               dacValue1Size + 1, dac1Start, dac1Stop + dac1Step,
                               dacValue2Size + 1, dac2Start, dac2Stop + dac2Step);
  minPhHisto->GetXaxis()->SetTitle("VOffsetR0 [DAC units]");
  minPhHisto->GetYaxis()->SetTitle("VoffsetOp [DAC units]");
  minPhHisto->GetZaxis()->SetTitle("starting PH");

  int r0, op;
  for (int i = 0; i <= dacValue1Size; i++)
  {
    r0 = dac1Start + i*dac1Step;
    SetDAC("VOffsetR0", r0);

    for (int k = 0; k <= dacValue2Size; k++)  
    {
      op = dac2Start + k*dac2Step;
      SetDAC("VoffsetOp", op);

      TH1D *histo = new TH1D( Form( "PHVcal_VoffsetOp%d_VOffsetR0%d_C%i", op, r0, chipId),
                              Form( "PHVcal_VoffsetOp%d_VOffsetR0%d_C%i", op, r0, chipId),
                              256, 0, 256);

      // PHDac( dac, dacRange, Trig, position, output)
      dynamic_cast<TBAnalogInterface *>( tbInterface)->PHDac( 25, 256, nTrig,
                                                              16 + aoutChipPosition * 3, result);
        
      for (int dac = 0; dac < 256; dac++) histo->SetBinContent( dac + 1, result[dac]);

      histo->SetMaximum( result[255] + 100);
        
      linearRange = static_cast<int>( FindLinearRange(histo) );
        
      if (linearRange > VcalRangeMax)
      {
        VcalRangeMax = linearRange;
        index1 = i;
        index2 = k;
      }
      psi::Log<psi::Debug>() << "[OffsetOptimization] Linear Range: " << linearRange
                      << std::endl;
      
      histo2->SetBinContent(i+1, k+1, linearRange);
      minPhHisto->SetBinContent(i+1, k+1, minPh);

      histograms->Add(histo);

      psi::Log<psi::Debug>() << "[OffsetOptimization] VOffsetR0: " << r0
                      << " VOffsetOp: " << op << std::endl;
    }
  }

  histograms->Add(minPhHisto);
  histograms->Add(histo2);

  optimalOp = dac2Start + index2 * dac2Step;
  optimalR0 = dac1Start + index1 * dac1Step;

  psi::Log<psi::Debug>() << "[OffsetOptimization] Vcal Range Max: " << VcalRangeMax
                  << " @ VOffsetR0: " << optimalR0
                  << " @ VOffsetOp: " << optimalOp << std::endl;

  psi::Log<psi::Debug>() << "[OffsetOptimization] Pixel Column: " << pixel->GetColumn()
                  << " Row: " << pixel->GetRow() << std::endl;
}