/*!
 * \file VhldDelOptimization.cc
 * \brief Implementation of VhldDelOptimization class.
 */

#include "TF1.h"
#include "TH1D.h"
#include "TH2.h"
#include "TCanvas.h"
#include "psi/log.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/ConfigParameters.h"
#include "psi46expert/TestRoc.h"
#include "PhDacScan.h"
#include "VhldDelOptimization.h"
#include "OffsetOptimization.h"
#include "FigureOfMerit.h"
#include "BasePixel/TestParameters.h"

VhldDelOptimization::VhldDelOptimization(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface)
    : Test("VhldDelOptimization", testRange), tbInterface(aTBInterface)
{
    debug = true;
}

void VhldDelOptimization::RocAction(TestRoc& roc)
{
    psi::LogInfo() << "VhldDelOptimization roc " << roc.GetChipId() << std::endl;

    TH1D *VhldDelHist = new TH1D("VhldDel", "VhldDel", 26, 0, 260);
    boost::shared_ptr<TestRange> pixelRange(new TestRange());
    int bestHldDel;

    for (int col = 0; col < 5; col++) {
        for (int row = 0; row < 5; row++) {
            pixelRange->AddPixel(roc.GetChipId(), col, row);
            bestHldDel = AdjustVhldDel(roc, pixelRange);
            VhldDelHist->Fill(bestHldDel);
        }
    }
    histograms->Add(VhldDelHist);
}

int VhldDelOptimization::AdjustVhldDel(TestRoc& roc, PTestRange pixelRange)
{

    roc.SetDAC(DACParameters::CtrlReg, 4);
    tbInterface->Flush();

    const int vsfValue = 150, hldDelMin = 0, hldDelMax = 200, hldDelStep = 10;

    TestParameters& testParameters = TestParameters::ModifiableSingleton();
    testParameters.setPHdac1Start(vsfValue);   // Vsf
    testParameters.setPHdac1Stop(vsfValue);
    testParameters.setPHdac1Step(10);
    testParameters.setPHdac2Start(hldDelMin);     // VhldDel
    testParameters.setPHdac2Stop(hldDelMax);
    testParameters.setPHdac2Step(hldDelStep);

    SaveDacParameters(roc);

    FigureOfMerit fom(pixelRange, tbInterface, DACParameters::Vsf, DACParameters::VhldDel, 3);
    fom.RocAction(roc);
    boost::shared_ptr<TList> histos = fom.GetHistos();
    TIter next(histos.get());
    if (debug) while (TH1 *histo = (TH1*)next()) histograms->Add(histo);

    psi::LogInfo() << "dac1 = " << roc.GetDAC(DACParameters::Vsf)
                   << " DAC1 = " << roc.GetDAC(DACParameters::VhldDel) << std::endl;

    TH2D *qualityHist2D = (TH2D*)(histos->Last());
    int nBins = (hldDelMax - hldDelMin) / hldDelStep;
    TH1D *qualityHist1D = new TH1D("VhldDel", "VhldDel", nBins, hldDelMin, hldDelMax);

    double maxLinearity = 0;
    double maxBin = -1;

    for (int n = 0; n <= nBins; n++) {
        double linearRange = qualityHist2D->GetBinContent(1, n + 1);
        qualityHist1D->SetBinContent(n + 1, linearRange);
        if (linearRange > maxLinearity) {
            maxLinearity = linearRange;
            maxBin = hldDelMin + n * hldDelStep;
        }
    }
    psi::LogInfo() << "max Linearity = " << maxLinearity << " @ VhldDel = " << maxBin <<  std::endl;
    histograms->Add(qualityHist1D);
    RestoreDacParameters(roc);

    hldDelValue = static_cast<int>( maxBin);

    roc.SetDAC(DACParameters::VhldDel, hldDelValue);
    psi::LogInfo() << "VhldDel set to " << hldDelValue << std::endl;

    return hldDelValue;
}
