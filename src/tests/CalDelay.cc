/*!
 * \file CalDelay.cc
 * \brief Implementation of CalDelay class.
 *
 * \b Changelog
 * 02-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using psi::Sleep instead interface/Delay.
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using definitions from PsiCommon.h.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#include <iostream>
#include <stdio.h>

#include "TF1.h"
#include "TH1D.h"
#include "TH2.h"
#include "TObjArray.h"
#include "TMath.h"

#include "psi/log.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/ConfigParameters.h"
#include "psi46expert/TestRoc.h"
#include "CalDelay.h"

namespace
{
  double vcal[13];
  int vwllsh[4];
}

CalDelay::CalDelay(TestRange *aTestRange, TBInterface *aTBInterface)
{
  psi::Log<psi::Debug>() << "[CalDelay] Initialization." << std::endl;

  testRange = aTestRange;
  tbInterface = aTBInterface;
  ReadTestParameters();
}


void CalDelay::ReadTestParameters()
{
    debug = false;
}

void CalDelay::RocAction()
{
  printf("CalDelay roc %i\n", chipId);

  vcal[1]  =  60.;
  vcal[2]  =  70.;
  vcal[3]  =  80.;
  vcal[4]  =  90.;
  vcal[5]  = 100.;
  vcal[6]  = 150.;
  vcal[7]  = 200.;
  vcal[8]  = 250.;
  vcal[9]  =  50.;
  vcal[10] =  70.;
  vcal[11] =  90.;
  vcal[12] = 200.;
  
  vwllsh[0] =   0;
  vwllsh[1] =  35;
  vwllsh[2] =  70;
  vwllsh[3] = 200; 
  
  TestRange* testRange_allPixels = new TestRange();
  TestRange* testRange_edgePixels = new TestRange();
  TestRange* testRange_cornerPixels = new TestRange();
  for ( int iColumn = 0; iColumn < psi::ROCNUMCOLS; iColumn++ ){
    for ( int iRow = 0; iRow < psi::ROCNUMROWS; iRow++ ){
      testRange_allPixels->AddPixel(roc->GetChipId(), iColumn, iRow);
      if ( (iColumn == 0 || iColumn == (psi::ROCNUMCOLS - 1)) ||
       (iRow    == 0 || iRow    == (psi::ROCNUMROWS - 1)) ){
    if ( (iColumn == 0 || iColumn == (psi::ROCNUMCOLS - 1)) &&
         (iRow    == 0 || iRow    == (psi::ROCNUMROWS - 1)) ){
	  testRange_cornerPixels->AddPixel(roc->GetChipId(), iColumn, iRow);
	} else {
	  testRange_edgePixels->AddPixel(roc->GetChipId(), iColumn, iRow);
	}
      }
    }
  }

  //for (int iVwllSh = 0; iVwllSh < 4; iVwllSh++){
  for (int iVwllSh = 1; iVwllSh < 2; iVwllSh++){
    SetDAC("VwllSh", vwllsh[iVwllSh]);
  
    printf("VwllSh = %i : ", vwllsh[iVwllSh]);

    TObjArray* graphArray_allPixels = GetEfficiency(Form("allPixels_VwllSh%i", vwllsh[iVwllSh]), testRange_allPixels);
    histograms->AddAll(graphArray_allPixels);

    TObjArray* graphArray_edgePixels = GetEfficiency(Form("edgePixels_VwllSh%i", vwllsh[iVwllSh]), testRange_edgePixels);
    histograms->AddAll(graphArray_edgePixels);
    
    TObjArray* graphArray_cornerPixels = GetEfficiency(Form("cornerPixels_VwllSh%i", vwllsh[iVwllSh]), testRange_cornerPixels);
    histograms->AddAll(graphArray_cornerPixels);
  }
}

TObjArray* CalDelay::GetEfficiency(const char* testName, TestRange* testRange)
{
  TObjArray* graphArray = new TObjArray();

  double dataBuffer[psi::ROCNUMROWS*psi::ROCNUMCOLS], lastEfficiency = 0.;

  for ( int iVcal = 0; iVcal < 13; iVcal++ ){
    double vCalRangeFactor = (iVcal > 8) ? 7. : 1.;
    printf(" Vcal = %f : ", vcal[iVcal]*vCalRangeFactor);
    
    SetDAC("Vcal", TMath::Nint(vcal[iVcal]));
    if ( iVcal > 8 ) 
      SetDAC("CtrlReg", 4);
    else 
      SetDAC("CtrlReg", 0);
    
    TGraph* graph = new TGraph();
    TString name = Form("CalDelay_%s_Vcal%i", testName, iVcal);
    graph->SetName(name);
    int nPoints = 0;
    for ( int iCalDel = 0; iCalDel < 255; iCalDel+=10 ){
      printf(".");
      std::cout.flush();
      
      SetDAC("CalDel", iCalDel);
      Flush();
      roc->ChipEfficiency(10, dataBuffer);
      
      double efficiency = 0.;
      int numPixels = 0;
      for ( int iColumn = 0; iColumn < psi::ROCNUMCOLS; iColumn++ ){
    for ( int iRow = 0; iRow < psi::ROCNUMROWS; iRow++ ){
	  if ( testRange->IncludesPixel(roc->GetChipId(), iColumn, iRow) ){
        efficiency += dataBuffer[iColumn*psi::ROCNUMROWS + iRow];
	    numPixels++;
	  }
	}
      }
      efficiency /= numPixels;
      
      if ( TMath::Abs(lastEfficiency - efficiency) > 0.05 ){
	for ( int jCalDel = -9; jCalDel <= 0; jCalDel++ ){
	  printf(".");
      std::cout.flush();
	  
	  SetDAC("CalDel", iCalDel + jCalDel);
	  //printf("caldel %i\n", iCalDel + jCalDel);
	  Flush();
	  roc->ChipEfficiency(10, dataBuffer);
	  
	  efficiency = 0.;
	  numPixels = 0;
      for ( int iColumn = 0; iColumn < psi::ROCNUMCOLS; iColumn++ ){
        for ( int iRow = 0; iRow < psi::ROCNUMROWS; iRow++ ){
	      if ( testRange->IncludesPixel(roc->GetChipId(), iColumn, iRow) ){
        efficiency += dataBuffer[iColumn*psi::ROCNUMROWS + iRow];
		numPixels++;
	      }
	    }
	  }	  
	  efficiency /= numPixels;
	  
	  graph->SetPoint(nPoints, iCalDel + jCalDel, efficiency);
	  nPoints++;
	}
      } else {
	graph->SetPoint(nPoints, iCalDel, efficiency);
	nPoints++;
      }
      lastEfficiency = efficiency;
    }
    
    std::cout << std::endl;
    
    int color = (iVcal % 7) + 1;
    int style = (iVcal / 7) + 1;
    graph->SetLineColor(color);
    graph->SetLineStyle(style);
    graph->SetMarkerColor(color);
    graphArray->AddLast(graph);
  }
  
  return graphArray;
}
