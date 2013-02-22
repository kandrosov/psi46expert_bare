/*!
 * \file ThrComp.cc
 * \brief Implementation of ThrComp class.
 *
 * \b Changelog
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
#include "interface/Delay.h"
#include "interface/Log.h"
#include "BasePixel/Roc.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/ConfigParameters.h"
#include "TestRoc.h"
#include "ThrComp.h"
#include <TMath.h>

ThrComp::ThrComp(TestRange *aTestRange, TBInterface *aTBInterface)
{
  testRange = aTestRange;
  tbInterface = aTBInterface;
  ReadTestParameters();
}

void ThrComp::ReadTestParameters()
{
    debug = false;
}

void ThrComp::RocAction()
{
  Float_t vcal = 200.;

  double data[psi::ROCNUMROWS*psi::ROCNUMCOLS];
  double dataMax[psi::ROCNUMROWS*psi::ROCNUMCOLS];
  double efficiency, lastEfficiency = 0.;

  printf("VthrComp roc %i\n", chipId);
  
  SetDAC("Vcal", TMath::Nint(vcal));
  //SetDAC("CtrlReg", 4);
  SetDAC("CtrlReg", 0);

  TGraph* graph = new TGraph();
  TString name = Form("VthrComp_Vcal%i", vcal);
  graph->SetName(name);
  graph->SetLineColor(2);
  graph->SetLineStyle(1);
  graph->SetLineWidth(2);
  graph->SetMarkerColor(2);

  int nPoints = 0;

  for ( Int_t ithrComp = 0; ithrComp < 255; ithrComp += 10 ){
    std::cout << "VthrComp = " << ithrComp << " : ";

    SetDAC("VthrComp", ithrComp);

    this->RocActionAuxiliary(data, dataMax);

    std::cout << std::endl;

    efficiency = 0.;
    for ( int ipixel = 0; ipixel < psi::ROCNUMROWS*psi::ROCNUMCOLS; ipixel++ ) efficiency += dataMax[ipixel];
    efficiency /= psi::ROCNUMROWS*psi::ROCNUMCOLS;
    std::cout << " efficiency = " << efficiency << std::endl;
		
    if ( TMath::Abs(lastEfficiency - efficiency) > 0.1 ){
      for ( int jthrComp = -9; jthrComp <= 0; jthrComp++ ){
    std::cout << "VthrComp = " << ithrComp + jthrComp << " : ";

	SetDAC("VthrComp", ithrComp + jthrComp);

	this->RocActionAuxiliary(data, dataMax);

    std::cout << std::endl;

	efficiency = 0.;
    for ( int ipixel = 0; ipixel < psi::ROCNUMROWS*psi::ROCNUMCOLS; ipixel++ ) efficiency += dataMax[ipixel];
    efficiency /= psi::ROCNUMROWS*psi::ROCNUMCOLS;
    std::cout << " efficiency = " << efficiency << std::endl;
	
	graph->SetPoint(nPoints, ithrComp + jthrComp, efficiency);
	nPoints++;
      }
    } else {
      graph->SetPoint(nPoints, ithrComp, efficiency);
      nPoints++;
    }

    lastEfficiency = efficiency;
  }
  
  histograms->Add(graph);
  graph->Write();
}

void ThrComp::RocActionAuxiliary(double data[], double dataMax[])
{
  for ( int ipixel = 0; ipixel < psi::ROCNUMROWS*psi::ROCNUMCOLS; ipixel++ ){
    dataMax[ipixel] = -1e6;
  }

  for ( Int_t icalDel = 0; icalDel < 255; icalDel += 25 ){
    printf(".");
    std::cout.flush();
    
    SetDAC("CalDel", icalDel);
    Flush();
    roc->ChipEfficiency(10, data);
    
    for ( int ipixel = 0; ipixel < psi::ROCNUMROWS*psi::ROCNUMCOLS; ipixel++ ){
      if ( data[ipixel] > dataMax[ipixel] ) dataMax[ipixel] = data[ipixel];
    }
  }

  //for ( int ipixel = 0; ipixel < ROC_NUMROWS*ROC_NUMCOLS; ipixel++ ){
  //  cout << "dataMax[" << ipixel << "] = " << dataMax[ipixel] << endl;
  //}
}


