/*!
 * \file Xray.cc
 * \brief Implementation of Xray class.
 *
 * \b Changelog
 * 09-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Corrected questionable language constructions, which was found using -Wall g++ option.
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using definitions from PsiCommon.h.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#include <iomanip>

#include "psi/log.h"

#include "TMath.h"
#include <TF1.h>

#include "Xray.h"
#include "psi46expert/TestModule.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/ThresholdMap.h"
#include "BasePixel/Analysis.h"
#include <TMinuit.h>
#include "BasePixel/TestParameters.h"

Double_t Erffcn( Double_t *x, Double_t *par)
{
    return par[0] * TMath::Erf(par[2] * (x[0] - par[1])) + par[3];
}

// ----------------------------------------------------------------------
// 3 parameter version
Double_t Erf3fcn( Double_t *x, Double_t *par)
{
    return par[0] * TMath::Erf(par[2] * (x[0] - par[1])) + par[0];
}

Xray::Xray(TestRange *aTestRange, TBInterface *aTBInterface)
{
    psi::LogDebug() << "[Xray] Initialization." << std::endl;

    testRange = aTestRange;
    tbInterface = aTBInterface;
    ReadTestParameters();
}

void Xray::ReadTestParameters()
{
    const TestParameters& testParameters = TestParameters::Singleton();
    nTrig = testParameters.XrayNTrig();
    vthrCompMin = testParameters.XrayVthrCompMin();
    vthrCompMax = testParameters.XrayVthrCompMax();
    maxEff = testParameters.XrayMaxEff();
}

void Xray::ModuleAction()
{
    TBAnalogInterface *tb = ((TBAnalogInterface*)tbInterface);
    int counts[psi::MODULENUMROCS], amplitudes[psi::MODULENUMROCS], countsTemp[psi::MODULENUMROCS],
        amplitudesTemp[psi::MODULENUMROCS];
    int sum, nRocs = module->NRocs();

    module->AdjustDTL();

    for (int iRoc = 0; iRoc < nRocs; iRoc++)
    {
        int chipId = module->GetRoc(iRoc)->GetChipId();
        histo[chipId] = new TH1F(Form("XrayCal_C%i", chipId), Form("XrayCal_C%i", chipId), 256, 0., 256.);

        module->GetRoc(iRoc)->SaveDacParameters();
        module->GetRoc(iRoc)->SetDAC("WBC", 106);
        if (testRange->IncludesRoc(iRoc))
        {
            module->GetRoc(iRoc)->EnableAllPixels();
        }
    }

    tb->DataEnable(false);
    // max stretching is 1022 (Beat)
    tb->SetClockStretch(STRETCH_AFTER_CAL, 5, 65535);
    tb->Flush();

    // Check for noisy pixels

    int nTrigs = 10000;
    for (int iRoc = 0; iRoc < nRocs; iRoc++) module->GetRoc(iRoc)->SetDAC("VthrComp", vthrCompMin);
    tb->CountAllReadouts(nTrigs / 10, countsTemp, amplitudesTemp);
    for (int iRoc = 0; iRoc < nRocs; iRoc++)
    {
        if (countsTemp[iRoc] > maxEff * nTrigs / 10.)
        {
            psi::LogInfo() << "[Xray] Noisy ROC #"
                           << module->GetRoc(iRoc)->GetChipId() << std::endl;

            std::vector<int> *badCols = new std::vector<int>;

            for (int i = 0; i < 26; i++)
            {
                module->GetRoc(iRoc)->Mask();
                for (int ir = 0; ir < 80; ir++)
                {
                    module->GetRoc(iRoc)->EnablePixel(i * 2, ir);
                    module->GetRoc(iRoc)->EnablePixel(i * 2 + 1, ir);
                }
                Flush();
                tb->CountAllReadouts(nTrigs / 10, countsTemp, amplitudesTemp);
                psi::LogDebug() << "[Xray] Dcol " << i << " readouts "
                                << countsTemp[iRoc] << std::endl;

                if (countsTemp[iRoc] > maxEff * nTrigs / 10.)
                {
                    badCols->push_back(i);
                    psi::LogDebug() << "[Xray] Disabling dcol " << i << std::endl;
                }
            }

            module->GetRoc(iRoc)->EnableAllPixels();
            for (int i = 0; i < static_cast<int>( badCols->size() ); i++) module->GetRoc(iRoc)->DisableDoubleColumn(badCols->at(i) * 2);
        }
    }

    // Start scan

    sum = 0;
    for (int vthrComp = vthrCompMin; vthrComp < vthrCompMax; vthrComp++)
    {
        for (int iRoc = 0; iRoc < nRocs; iRoc++)
        {
            counts[iRoc] = 0;
            amplitudes[iRoc] = 0;
            module->GetRoc(iRoc)->SetDAC("VthrComp", vthrComp);
        }
        tb->Flush();

        for (int k = 0; k < nTrig / nTrigs; k++)
        {
            tb->CountAllReadouts(nTrigs, countsTemp, amplitudesTemp);
            for (int iRoc = 0; iRoc < nRocs; iRoc++)
            {
                counts[iRoc] += countsTemp[iRoc];
                amplitudes[iRoc] += amplitudesTemp[iRoc];
            }
        }

        if (nTrig % nTrigs > 0)
        {
            tb->CountAllReadouts(nTrig % nTrigs, countsTemp, amplitudesTemp);
            for (int iRoc = 0; iRoc < nRocs; iRoc++)
            {
                counts[iRoc] += countsTemp[iRoc];
                amplitudes[iRoc] += amplitudesTemp[iRoc];
            }
        }

        sum = 0;

        for (int iRoc = 0; iRoc < nRocs; iRoc++)
        {
            psi::LogDebug() << "[Xray] Roc #" << iRoc << " has "
                            << counts[iRoc] << " counts." << std::endl;
            if (counts[iRoc] < maxEff * nTrig) histo[module->GetRoc(iRoc)->GetChipId()]->Fill(vthrComp, counts[iRoc]); //if threshold too low -> noise hits
            else
            {
                module->GetRoc(iRoc)->Mask();
                tb->Flush();
            }

            sum += counts[iRoc];
        }
        psi::LogInfo() << "VthrComp " << vthrComp << " Sum " << sum << std::endl;
    }

    tb->SetClockStretch(0, 0, 0);
    tb->DataEnable(true);
    for (int iRoc = 0; iRoc < nRocs; iRoc++)
    {
        module->GetRoc(iRoc)->Mask();
        histograms->Add(histo[module->GetRoc(iRoc)->GetChipId()]);
    }
    tb->Flush();

    Test::ModuleAction();

    for (int iRoc = 0; iRoc < nRocs; iRoc++) module->GetRoc(iRoc)->RestoreDacParameters();
}


void Xray::RocAction()
{
    TH1F *h1 = histo[chipId];
    TF1 *fit = new TF1("Fit", Erf3fcn, 0., 256., 3);

    int minFit = 20;
    int maxFit = 120;

    // -- Patch missing errors in empty bins
    for (int i = 1; i <= h1->GetNbinsX(); ++i)
    {
        if (h1->GetBinContent(i) < 0.5) h1->SetBinError(i, 1.);
    }


    // -- minimum fit range
    for (int i = 0; i < 100; ++i)
    {
        if (h1->GetBinContent(i) > 0)
        {
            minFit = i;
            break;
        }
    }
    minFit -= 10;
    if (minFit < 0) minFit = 1;


    // -- maximum fit range
    for (int i = 150; i > 50; --i)
    {
        if ((h1->GetBinContent(i) > 0)
                && (h1->GetBinContent(i - 1) > h1->GetBinContent(i) - 3.*h1->GetBinError(i))
                && (h1->GetBinContent(i - 1) < h1->GetBinContent(i) + 3.*h1->GetBinError(i))
                && (h1->GetBinContent(i - 2) > h1->GetBinContent(i) - 3.*h1->GetBinError(i))
                && (h1->GetBinContent(i - 2) < h1->GetBinContent(i) + 3.*h1->GetBinError(i))
                && (h1->GetBinContent(i - 3) > h1->GetBinContent(i) - 3.*h1->GetBinError(i))
                && (h1->GetBinContent(i - 3) < h1->GetBinContent(i) + 3.*h1->GetBinError(i))
           )
        {
            maxFit = i - 1;
            break;
        }
    }


    // -- plateau value
    double ave(0.);
    int nbin = 15;
    for (int i = maxFit; i > maxFit - nbin; --i)
    {
        ave += h1->GetBinContent(i);
        if (h1->GetBinContent(i) < h1->GetBinContent(i + 1) - 5.*h1->GetBinError(i + 1)
                && h1->GetBinContent(i) < h1->GetBinContent(i - 1) - 5.*h1->GetBinError(i - 1)
           )
        {
            h1->SetBinContent(i, h1->GetBinContent(i + 1));
            h1->SetBinError(i, TMath::Sqrt(h1->GetBinContent(i + 1)));
        }
    }
    ave /= nbin;
    ave /= 2.;

    // -- Threshold value
    double thr(0.);
    double redTotal(0.);
    if (h1->GetSumOfWeights() > 1)
    {
        redTotal = 0.15 * h1->Integral(1, maxFit);
    }

    int ii = 0;
    for (ii = 20; ii < maxFit; ++ii)
    {
        if (h1->Integral(1, ii) > redTotal)
        {
            thr = ii;
            break;
        }
    }

    fit->SetParameter(0, ave);
    fit->SetParLimits(0, 0., ave * 2.);
    fit->SetParameter(1, thr);
    fit->SetParLimits(1, 0.7 * thr, (1.3 * thr > maxFit ? maxFit : 1.3 * thr));
    fit->SetParameter(2, .1);
    fit->SetParLimits(2, 0.05, 2.);

    ((TBAnalogInterface*)tbInterface)->Clear();

    histo[chipId]->Fit("Fit", "RQ", "", minFit, maxFit);

    double threshold = fit->GetParameter(1);
    double sigma = 1. / (TMath::Sqrt(2.) * fit->GetParameter(2));
    psi::LogInfo() << "Roc " << chipId << " Thr " << std::setprecision(1) << threshold
                   << " Sigma " << std::setprecision(1) << sigma << std::endl;

    roc->RestoreDacParameters(); //restore wbc

    if (threshold > vthrCompMin && threshold < vthrCompMax && sigma > 0.05 && sigma < 20.)
    {
        ThresholdMap *thresholdMap = new ThresholdMap();
        thresholdMap->SetDoubleWbc(); //absolute threshold (not in-time)

        SetDAC("VthrComp", (int)TMath::Floor(threshold));
        Flush();

        TH2D* vcalMap = thresholdMap->GetMap("VcalThresholdMap", roc, testRange, 5);
        TH1D* vcalMapDistribution = Analysis::Distribution(vcalMap);
        double vcal1 = vcalMapDistribution->GetMean();
        double vcalSigma1 = vcalMapDistribution->GetRMS();

        SetDAC("VthrComp", (int)TMath::Floor(threshold) + 1);
        Flush();

        TH2D* vcalMap2 = thresholdMap->GetMap("VcalThresholdMap", roc, testRange, 2);
        TH1D* vcalMapDistribution2 = Analysis::Distribution(vcalMap2);
        double vcal2 = vcalMapDistribution2->GetMean();

        psi::LogInfo() << "vcal1 " << vcal1 << " vcal2 " << vcal2 << std::endl;

        double vcal = vcal1 - (vcal1 - vcal2) * (threshold - (double)TMath::Floor(threshold));

        psi::LogInfo() << "Vcal " << std::setprecision(1) << vcal << " pm " << std::setprecision(1) << vcalSigma1
                       << std::endl;

        roc->RestoreDacParameters();
        Flush();

        histograms->Add(vcalMap);
        histograms->Add(vcalMapDistribution);
        histograms->Add(vcalMap2);
        histograms->Add(vcalMapDistribution2);

        vcalMap->Write();
        vcalMapDistribution->Write();
        vcalMap2->Write();
        vcalMapDistribution2->Write();
    }
}
