/*!
 * \file UbCheck.cc
 * \brief Implementation of UbCheck class.
 */

#include "TF1.h"
#include "TH1D.h"
#include "TH2.h"

#include "psi/log.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/ConfigParameters.h"
#include "psi46expert/TestRoc.h"
#include "UbCheck.h"
#include "OffsetOptimization.h"
#include "BasePixel/TestParameters.h"

UbCheck::UbCheck(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface)
    : Test("UbCheck", testRange), tbInterface(aTBInterface)
{
    nTrig = TestParameters::Singleton().PHCalibrationNTrig();
}

void UbCheck::RocAction(TestRoc& roc)
{
    psi::LogInfo() << "UbCheck roc " << roc.GetChipId() << std::endl;
    const int testVcal = 200;

    std::ostringstream histoName;
    histoName << "PH" << testVcal << "_C" << roc.GetChipId();
    TH1D *histo = new TH1D(histoName.str().c_str(), histoName.str().c_str(), 400, -2000., 2000.);
    histo->GetXaxis()->SetTitle("PH");
    histo->GetYaxis()->SetTitle("# pixels");

    SaveDacParameters(roc);

    // == Measure pulse height for all pixels

    int data[psi::ROCNUMROWS * psi::ROCNUMCOLS];
    int phPosition = 16 + roc.GetAoutChipPosition() * 3;
    int minPixelPh = 2000;

    roc.SetDAC(DACParameters::CtrlReg, 0);
    roc.SetDAC(DACParameters::Vcal, testVcal);
    roc.SetDAC(DACParameters::VoffsetOp, 50);
    tbInterface->Flush();

    roc.AoutLevelChip(phPosition, nTrig, data);
    for (unsigned k = 0; k < psi::ROCNUMROWS * psi::ROCNUMCOLS; k++) histo->Fill(data[k]);
    for (unsigned k = 0; k < psi::ROCNUMROWS * psi::ROCNUMCOLS; k++) {
        if ((data[k] < minPixelPh) && (TMath::Abs(data[k] - histo->GetMean()) < 4 * histo->GetRMS())) {
            minPixelPh = data[k];
            minPixel = k;
        }
    }

    histograms->Add(histo);

    if (debug)
        psi::LogInfo() << "minimum pixel = " << minPixel << " minPH = " << minPixelPh << std::endl;
    if (debug)
        psi::LogInfo() << "col = " << minPixel / psi::ROCNUMROWS << " row = " << minPixel % psi::ROCNUMROWS << "\n";

    AdjustOpR0(roc);
}

int UbCheck::Ultrablack()
{
    short data[10000];
    unsigned short count;

    tbInterface->ADCData(data, count);
    if (count > 1) return (data[0] + data[1] + data[2]) / 3;
    psi::LogInfo() << " >>>>>>>>>>>>>>>> Error: Couldn't find ultra black level";
    return 0;
}

void UbCheck::AdjustOpR0(TestRoc& roc)
{
    int OpValue;

    roc.SetDAC(DACParameters::CtrlReg, 4);
    tbInterface->Flush();

    int R0Value = roc.GetDAC(DACParameters::VOffsetR0);
    psi::LogDebug() << "[UbCheck] VOffsetR0 " << R0Value << std::endl;

    boost::shared_ptr<TestRange> minPixelRange(new TestRange());
    minPixelRange->AddPixel(roc.GetChipId(), minPixel / psi::ROCNUMROWS, minPixel % psi::ROCNUMROWS);
    TestParameters& testParameters = TestParameters::ModifiableSingleton();
    testParameters.setPHdac1Start(R0Value);
    testParameters.setPHdac1Stop(R0Value);
    testParameters.setPHdac1Step(10);
    testParameters.setPHdac2Start(0);
    testParameters.setPHdac2Stop(200);
    testParameters.setPHdac2Step(5);

    OffsetOptimization phDacScan(minPixelRange, tbInterface);
    phDacScan.RocAction(roc);
    boost::shared_ptr<TList> histos = phDacScan.GetHistos();
    TIter next(histos.get());
    if (debug) while (TH1 *histo = (TH1*)next()) histograms->Add(histo);

    RestoreDacParameters(roc);

    // -- get the histos from the PhDac Scan


    TH2D *histoPhDacScan = (TH2D*)(histos->Last());
    TH2D *minPhHisto = (TH2D*)(histos->At(histos->GetSize() - 2));

    TH1D *projectionHisto = histoPhDacScan->ProjectionY(Form("LinearRange_C%i", roc.GetChipId()), 0, 50, "e");
    TH1D *minPhProj = minPhHisto->ProjectionY(Form("PhMin_C%i", roc.GetChipId()), 0, 50, "e");

    // -- fit the linear range histo

    double maximum = projectionHisto->GetBinCenter(projectionHisto->GetMaximumBin());

    TF1 *gauss = new TF1("gauss", "gaus");
    gauss->SetParameters(200, 50, 15);
    gauss->SetParLimits(1, -50, 200);

    if (debug) projectionHisto->Fit("gauss", "", "", maximum - 50, maximum + 50);
    else projectionHisto->Fit("gauss", "QN", "", maximum - 50, maximum + 50);

    histograms->Add(projectionHisto);
    histograms->Add(minPhProj);

    // -- determine cut level

    int safetyMargin = 400;
    int cutLevel =  Ultrablack() + safetyMargin;
    if (debug) psi::LogInfo() << "Ub level " << cutLevel - safetyMargin << " margin " << safetyMargin << std::endl;
    int opCut = -1, bin = 0; // first allowed value
    do {
        bin++;
        if (minPhProj->GetBinContent(bin) > cutLevel)
            opCut = static_cast<int>( minPhProj->GetBinCenter(bin) );
//     if (minPhProj->GetBinContent(bin) > cutLevel)
//     {
//     	if (bin == 1) opCut = 0;
// 	else
// 	{
// 	    double ph1 = minPhProj->GetBinContent(bin - 1);
// 	    double ph2 = minPhProj->GetBinContent(bin);
// 	    double op1 = minPhProj->GetBinLowEdge(bin - 1);
// 	    double op2 = minPhProj->GetBinLowEdge(bin);
//
//     	    opCut = (int)( op1 + (op2 - op1) * (cutLevel - ph1) / (ph2 - ph1) );
// 	}
//     }
    } while (opCut == -1 && bin < 40);
    if (opCut == -1) opCut = 255;
    if (debug) psi::LogInfo() << "OpCut = " << opCut << " MinPh " << minPhProj->GetBinContent(bin) << std::endl;

    if (gauss->GetParameter(1) >= opCut) OpValue = (int)gauss->GetParameter(1);
    else OpValue = opCut;

    roc.SetDAC(DACParameters::VoffsetOp, OpValue);
    psi::LogDebug() << "[UbCheck] VOffsetOp is set to " << OpValue << std::endl;
}
