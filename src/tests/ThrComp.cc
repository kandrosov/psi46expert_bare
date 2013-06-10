/*!
 * \file ThrComp.cc
 * \brief Implementation of ThrComp class.
 */

#include "TF1.h"
#include "TH1D.h"
#include "TH2.h"
#include "psi/log.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/ConfigParameters.h"
#include "psi46expert/TestRoc.h"
#include "ThrComp.h"
#include <TMath.h>

ThrComp::ThrComp(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface)
    : Test("ThrComp", testRange), tbInterface(aTBInterface) {}

void ThrComp::RocAction(TestRoc& roc)
{
    Float_t vcal = 200.;

    double data[psi::ROCNUMROWS * psi::ROCNUMCOLS];
    double dataMax[psi::ROCNUMROWS * psi::ROCNUMCOLS];
    double efficiency, lastEfficiency = 0.;

    psi::LogInfo() << "VthrComp roc " << roc.GetChipId() << std::endl;

    roc.SetDAC(DACParameters::Vcal, TMath::Nint(vcal));
    roc.SetDAC(DACParameters::CtrlReg, 0);

    TGraph* graph = new TGraph();
    std::ostringstream ss;
    ss << "VthrComp_Vcal" << vcal;

    graph->SetName(ss.str().c_str());
    graph->SetLineColor(2);
    graph->SetLineStyle(1);
    graph->SetLineWidth(2);
    graph->SetMarkerColor(2);

    int nPoints = 0;

    for ( Int_t ithrComp = 0; ithrComp < 255; ithrComp += 10 ) {
        psi::LogInfo() << "VthrComp = " << ithrComp << " : ";

        roc.SetDAC(DACParameters::VthrComp, ithrComp);

        RocActionAuxiliary(roc, data, dataMax);

        psi::LogInfo() << std::endl;

        efficiency = 0.;
        for ( unsigned ipixel = 0; ipixel < psi::ROCNUMROWS * psi::ROCNUMCOLS; ipixel++ ) efficiency += dataMax[ipixel];
        efficiency /= psi::ROCNUMROWS * psi::ROCNUMCOLS;
        psi::LogInfo() << " efficiency = " << efficiency << std::endl;

        if ( TMath::Abs(lastEfficiency - efficiency) > 0.1 ) {
            for ( int jthrComp = -9; jthrComp <= 0; jthrComp++ ) {
                psi::LogInfo() << "VthrComp = " << ithrComp + jthrComp << " : ";

                roc.SetDAC(DACParameters::VthrComp, ithrComp + jthrComp);

                RocActionAuxiliary(roc, data, dataMax);

                psi::LogInfo() << std::endl;

                efficiency = 0.;
                for ( unsigned ipixel = 0; ipixel < psi::ROCNUMROWS * psi::ROCNUMCOLS; ipixel++ ) efficiency += dataMax[ipixel];
                efficiency /= psi::ROCNUMROWS * psi::ROCNUMCOLS;
                psi::LogInfo() << " efficiency = " << efficiency << std::endl;

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

void ThrComp::RocActionAuxiliary(TestRoc& roc, double data[], double dataMax[])
{
    for ( unsigned ipixel = 0; ipixel < psi::ROCNUMROWS * psi::ROCNUMCOLS; ipixel++ ) {
        dataMax[ipixel] = -1e6;
    }

    for ( Int_t icalDel = 0; icalDel < 255; icalDel += 25 ) {
        psi::LogInfo() << ".";

        roc.SetDAC(DACParameters::CalDel, icalDel);
        roc.Flush();
        roc.ChipEfficiency(10, data);

        for ( unsigned ipixel = 0; ipixel < psi::ROCNUMROWS * psi::ROCNUMCOLS; ipixel++ ) {
            if ( data[ipixel] > dataMax[ipixel] ) dataMax[ipixel] = data[ipixel];
        }
    }
}
