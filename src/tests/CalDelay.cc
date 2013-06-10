/*!
 * \file CalDelay.cc
 * \brief Implementation of CalDelay class.
 */

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

namespace {
double vcal[13];
int vwllsh[4];
}

CalDelay::CalDelay(PTestRange testRange)
    : Test("CalDelay", testRange)
{
}

void CalDelay::RocAction(TestRoc& roc)
{
    psi::LogInfo() << "CalDelay roc " << roc.GetChipId() << std::endl;

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

    boost::shared_ptr<TestRange> testRange_allPixels(new TestRange());
    boost::shared_ptr<TestRange> testRange_edgePixels(new TestRange());
    boost::shared_ptr<TestRange> testRange_cornerPixels(new TestRange());
    for ( unsigned iColumn = 0; iColumn < psi::ROCNUMCOLS; iColumn++ ) {
        for ( unsigned iRow = 0; iRow < psi::ROCNUMROWS; iRow++ ) {
            testRange_allPixels->AddPixel(roc.GetChipId(), iColumn, iRow);
            if ( (iColumn == 0 || iColumn == (psi::ROCNUMCOLS - 1)) ||
                    (iRow    == 0 || iRow    == (psi::ROCNUMROWS - 1)) ) {
                if ( (iColumn == 0 || iColumn == (psi::ROCNUMCOLS - 1)) &&
                        (iRow    == 0 || iRow    == (psi::ROCNUMROWS - 1)) ) {
                    testRange_cornerPixels->AddPixel(roc.GetChipId(), iColumn, iRow);
                } else {
                    testRange_edgePixels->AddPixel(roc.GetChipId(), iColumn, iRow);
                }
            }
        }
    }

    //for (int iVwllSh = 0; iVwllSh < 4; iVwllSh++){
    for (int iVwllSh = 1; iVwllSh < 2; iVwllSh++) {
        roc.SetDAC(DACParameters::VwllSh, vwllsh[iVwllSh]);

        psi::LogInfo() << "VwllSh = " << vwllsh[iVwllSh] << " : ";

        std::ostringstream allPixelsName;
        allPixelsName << "allPixels_VwllSh" << vwllsh[iVwllSh];
        TObjArray* graphArray_allPixels = GetEfficiency(allPixelsName.str(), roc, testRange_allPixels);
        histograms->AddAll(graphArray_allPixels);

        std::ostringstream edgePixelsName;
        edgePixelsName << "edgePixels_VwllSh" << vwllsh[iVwllSh];
        TObjArray* graphArray_edgePixels = GetEfficiency(edgePixelsName.str(), roc, testRange_edgePixels);
        histograms->AddAll(graphArray_edgePixels);

        std::ostringstream cornerPixelsName;
        cornerPixelsName << "cornerPixels_VwllSh" << vwllsh[iVwllSh];
        TObjArray* graphArray_cornerPixels = GetEfficiency(cornerPixelsName.str(), roc, testRange_cornerPixels);
        histograms->AddAll(graphArray_cornerPixels);
    }
}

TObjArray* CalDelay::GetEfficiency(const std::string& testName, TestRoc& roc, PTestRange testRange)
{
    TObjArray* graphArray = new TObjArray();

    double dataBuffer[psi::ROCNUMROWS * psi::ROCNUMCOLS], lastEfficiency = 0.;

    for ( int iVcal = 0; iVcal < 13; iVcal++ ) {
        double vCalRangeFactor = (iVcal > 8) ? 7. : 1.;
        psi::LogInfo() << " Vcal = " << (vcal[iVcal] * vCalRangeFactor) << " : ";

        roc.SetDAC(DACParameters::Vcal, TMath::Nint(vcal[iVcal]));
        if ( iVcal > 8 )
            roc.SetDAC(DACParameters::CtrlReg, 4);
        else
            roc.SetDAC(DACParameters::CtrlReg, 0);

        TGraph* graph = new TGraph();
        TString name = Form("CalDelay_%s_Vcal%i", testName.c_str(), iVcal);
        graph->SetName(name);
        int nPoints = 0;
        for ( int iCalDel = 0; iCalDel < 255; iCalDel += 10 ) {
            psi::LogInfo() << ".";

            roc.SetDAC(DACParameters::CalDel, iCalDel);
            roc.Flush();
            roc.ChipEfficiency(10, dataBuffer);

            double efficiency = 0.;
            int numPixels = 0;
            for ( unsigned iColumn = 0; iColumn < psi::ROCNUMCOLS; iColumn++ ) {
                for ( unsigned iRow = 0; iRow < psi::ROCNUMROWS; iRow++ ) {
                    if ( testRange->IncludesPixel(roc.GetChipId(), iColumn, iRow) ) {
                        efficiency += dataBuffer[iColumn * psi::ROCNUMROWS + iRow];
                        numPixels++;
                    }
                }
            }
            efficiency /= numPixels;

            if ( TMath::Abs(lastEfficiency - efficiency) > 0.05 ) {
                for ( int jCalDel = -9; jCalDel <= 0; jCalDel++ ) {
                    psi::LogInfo() << ".";

                    roc.SetDAC(DACParameters::CalDel, iCalDel + jCalDel);
                    roc.Flush();
                    roc.ChipEfficiency(10, dataBuffer);

                    efficiency = 0.;
                    numPixels = 0;
                    for ( unsigned iColumn = 0; iColumn < psi::ROCNUMCOLS; iColumn++ ) {
                        for ( unsigned iRow = 0; iRow < psi::ROCNUMROWS; iRow++ ) {
                            if ( testRange->IncludesPixel(roc.GetChipId(), iColumn, iRow) ) {
                                efficiency += dataBuffer[iColumn * psi::ROCNUMROWS + iRow];
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

        psi::LogInfo() << std::endl;

        int color = (iVcal % 7) + 1;
        int style = (iVcal / 7) + 1;
        graph->SetLineColor(color);
        graph->SetLineStyle(style);
        graph->SetMarkerColor(color);
        graphArray->AddLast(graph);
    }

    return graphArray;
}
