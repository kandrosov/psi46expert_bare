/*!
 * \file SCurveTestBeam.cc
 * \brief Implementation of SCurveTestBeam class.
 */

#include <TGraphErrors.h>
#include <TMath.h>
#include <TF1.h>
#include "SCurveTestBeam.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/CalibrationTable.h"
#include "BasePixel/RawPacketDecoder.h"
#include "BasePixel/DecoderCalibration.h"
#include "BasePixel/Test.h"
#include "psi46expert/TestRoc.h"
#include "psi/log.h"
#include "BasePixel/TestParameters.h"

using namespace DecoderCalibrationConstants;
using namespace DecodedReadoutConstants;
namespace {
Double_t Erffcn2( Double_t *x, Double_t *par)
{
    return par[0] * TMath::Erf(par[2] * (x[0] - par[1])) + par[3];
}
}

SCurveTestBeam::SCurveTestBeam(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface)
    : Test("SCurveTestBeam", testRange), tbInterface(aTBInterface)
{
    const TestParameters& testParameters = TestParameters::Singleton();
    vcal = testParameters.SCurveVcal();
    vthr = testParameters.SCurveVthr();
    mode = testParameters.SCurveMode();
    nTrig = testParameters.SCurveBeamNTrig();
}

void SCurveTestBeam::RocAction(TestRoc& roc)
{
    unsigned short count;
    int nReadouts, readoutStart[256];
    short data[psi::FIFOSIZE];
    bool noError, pixelFound;

    int nMaxTrigs = 5, nTriggers;
    double x[255], y[255], ph[255], vcal[255], xErr[255], yErr[255];
    for (int i = 0; i < 256; i++) {
        vcal[i] = i;
        x[i] = 0.;
        xErr[i] = 0.0002; //guess
        y[i] = 0.;
        yErr[i] = 0.;
        ph[i] = 0.;
    }

    SaveDacParameters(roc);
    roc.Mask();
    roc.ClrCal();

    unsigned column = 7;   //pixel under test
    unsigned row = 7;    //pixel under test
    for (int i = 0; i < 51; i++) {
        roc.EnableDoubleColumn(i);
        for (int k = 0; k < 79; k++) {
            roc.EnablePixel(i, k);
        }
    }
    roc.ArmPixel(column, row);

    dacReg = DACParameters::Vcal;
//         SetDAC("VthrComp", vthr);
    roc.Flush();

//         ((TBAnalogInterface*)tbInterface)->ADC();

    for (int i = 20; i < 100; i++) {
        x[i] = CalibrationTable::VcalDAC(0, i);
        roc.SetDAC(dacReg, i);
        roc.Flush();

        int n = nTrig;
        while (n > 0) {
            if (n > nMaxTrigs) nTriggers = nMaxTrigs;
            else nTriggers = n;
            n -= nTriggers;
            do {
                roc.SendADCTrigs(nTriggers);
                roc.Flush();
                noError = roc.GetADC(data, psi::FIFOSIZE, count, nTriggers, readoutStart, nReadouts);
            } while (!noError);

            for (int k = 0; k < nReadouts; k++) {
                pixelFound = false;
                int nDecodedPixelHitsModule = RawPacketDecoder::Singleton()->decode(
                            (int)count, &data[readoutStart[k]], decodedModuleReadout, NUM_ROCSMODULE);
                psi::LogDebug() << "[SCurveTestBeam] nDec " << nDecodedPixelHitsModule
                                << std::endl;
                for (int iroc = 0; iroc < NUM_ROCSMODULE; iroc++) {
                    int nDecodedPixelHitsROC = decodedModuleReadout.roc[iroc].numPixelHits;
                    for (int ipixelhit = 0; ipixelhit < nDecodedPixelHitsROC; ipixelhit++) {
                        DecodedReadoutPixel decodedPixelHit = decodedModuleReadout.roc[iroc].pixelHit[ipixelhit];
                        if ((decodedPixelHit.rowROC == row) && (decodedPixelHit.columnROC == column)) {
                            pixelFound = true;
                            ph[i] += decodedPixelHit.analogPulseHeight;
                        }
                    }
                }

                if (pixelFound) y[i]++;
            }
        }
        ph[i] /= y[i];
        y[i] = ((double)y[i] + 1.) / (nTrig + 2.);
        yErr[i] = TMath::Sqrt((y[i] * (1. - y[i])) / (nTrig + 3.));
    }

    roc.ClrCal();

    TGraphErrors *graph = new TGraphErrors(256, x, y, xErr, yErr);
    graph->SetTitle(Form("SCurve_c%ir%i_C%d", column, row, roc.GetChipId()));
    graph->SetName(Form("SCurve_c%ir%i_C%d", column, row, roc.GetChipId()));


    TF1 *fit = new TF1("Fit", Erffcn2, 0.03, .07, 4);
    fit->SetParameters(.5, 0.05, 200., .5);         // mean level, threshold, width
    graph->Fit("Fit", "R", "", 0.0, 0.3);

    double slope = 0.3071 / 256.;  // voltage per Vcal DAC
    double thr, sig, thrErr, sigErr;
    thr = fit->GetParameter(1) * 65. / slope;  // conversion Vcal voltage -> Vcal DACs -> electrons
    thrErr = fit->GetParError(1) * 65. / slope;
    sig = 1. / (TMath::Sqrt(2.) * fit->GetParameter(2)) * 65. / slope;  // conversion Vcal voltage -> Vcal DACs -> electrons
    sigErr = sig / fit->GetParameter(2) * fit->GetParError(2);
    psi::LogInfo() << "thr " << thr << " (" << thrErr << ") sigma " << sig << " (" << sigErr << ")\n";
    histograms->Add(graph);

    TGraph *graph2 = new TGraph(256, vcal, ph);
    graph2->SetTitle(Form("VcalPH_c%ir%i_C%d", column, row, roc.GetChipId()));
    graph2->SetName(Form("VcalPH_c%ir%i_C%d", column, row, roc.GetChipId()));

    TF1 *fit2 = new TF1("Fit2", "pol1", 120., 250.);
    fit2->SetParameters(-400., 2.);
    graph2->Fit("Fit2", "R", "", 120., 250.);

    histograms->Add(graph2);

    RestoreDacParameters(roc);
}
