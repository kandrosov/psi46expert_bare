/*!
 * \file TimeWalkStudy.cc
 * \brief Implementation of TimeWalkStudy class.
 */

#include <math.h>
#include <boost/format.hpp>

#include <TGraph.h>
#include <TParameter.h>
#include "TCanvas.h"
#include "psi/date_time.h"
#include "psi/log.h"
#include "TimeWalkStudy.h"
#include "psi46expert/TestRoc.h"
#include "psi46expert/TestModule.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/TestParameters.h"
#include "BasePixel/DataStorage.h"

TimeWalkStudy::TimeWalkStudy(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface)
    : Test("TimeWalkStudy", testRange), tbInterface(aTBInterface)
{
    vcalThreshold = 60;
    meanShift = TestParameters::Singleton().TWMeanShift();
    debug = true;

    fit = new TF1("fitfn", "[0]/((x-[2])^[1])+[3]", 10., 60.);
    fit->SetParameters(100000., 1.7, 0., 80.);
}

void TimeWalkStudy::ModuleAction(TestModule& module)
{
    int nRocs = module.NRocs();
    for (int iRoc = 0; iRoc < nRocs; iRoc++) {
        vana[iRoc] = module.GetRoc(iRoc).GetDAC(DACParameters::Vana);
        module.GetRoc(iRoc).SetDAC(DACParameters::Vana, 0);
    }
    tbInterface->Flush();
    psi::Sleep(2.0 * psi::seconds);
    zeroCurrent = tbInterface->GetIA();

    Test::ModuleAction(module);

    TH1F *histoBefore = new TH1F("twBefore", "twBefore", 100, -10., 10.);
    TH1F *histoAfter = new TH1F("twAfter", "twAfter", 100, -10., 10.);

    for (int iRoc = 0; iRoc < nRocs; iRoc++) {
        module.GetRoc(iRoc).SetDAC(DACParameters::Vana, vana[iRoc]);
        if (testRange->IncludesRoc(iRoc)) histoBefore->Fill( psi::DataStorage::ToStorageUnits(twBefore[iRoc]));
        if (testRange->IncludesRoc(iRoc)) histoAfter->Fill( psi::DataStorage::ToStorageUnits(twAfter[iRoc]));
    }
    histograms->Add(histoBefore);
    histograms->Add(histoAfter);
}

void TimeWalkStudy::RocAction(TestRoc& roc)
{
    psi::LogInfo() << "[TimeWalkStudy] ROC #" << roc.GetChipId() << '.' << std::endl;

    psi::LogInfo().PrintTimestamp();

    //init pixel
    SaveDacParameters(roc);
    roc.SetDAC(DACParameters::Vana, vana[roc.GetChipId()]);
    TestPixel& pixel = roc.GetPixel(26, 5); //pixel in lower half of the chip
    int trim = pixel.GetTrim();
    pixel.ArmPixel();
    roc.Flush();

    //do test
    CalDelDeltaT(roc);
    GetPowerSlope(pixel);
    vana[roc.GetChipId()] = FindNewVana(pixel);

    //restore settings
    pixel.SetTrim(trim);
    pixel.DisarmPixel();
    RestoreDacParameters(roc);
}

psi::Time TimeWalkStudy::TimeWalk(TestRoc& roc, int vcalStep)
{
    double fp[4];
    unsigned short res[1000], lres = 1000;
    fit->SetParameters(100000., 1.7, 0., 80.);

    int calDelSAVED = roc.GetDAC(DACParameters::CalDel);
    int vcalSAVED = roc.GetDAC(DACParameters::Vcal);
    int wbcSAVED = roc.GetDAC(DACParameters::WBC);

    tbInterface->CdVc(roc.GetChipId(), 97, 102, vcalStep, 90, lres, res);

    roc.SetDAC(DACParameters::CalDel, calDelSAVED);
    roc.SetDAC(DACParameters::Vcal, vcalSAVED);
    roc.SetDAC(DACParameters::WBC, wbcSAVED);
    tbInterface->DataCtrl(true, false);
    tbInterface->Flush();

    int t = lres / 3;
    double x[t], y[t];
    for(int i = 0; i < lres; i = i + 3) {
        y[i / 3] = res[i];
        x[i / 3] = ((102 - res[i + 2] + 1) * 25 - 25.0 / calDelDT * res[i + 1] + 0.5);
    }

    TGraph *gr1 = new TGraph(t, x, y);
    gr1->Fit(fit, "RQ");
    histograms->Add(gr1);

    for(int i = 0; i < 4; i++) fp[i] = fit->GetParameter(i);
    return (meanShift - (pow((fp[0] / (200 - fp[3])), 1 / fp[1]) + fp[2])) * psi::seconds; //shift in time
}


int TimeWalkStudy::FindNewVana(TestPixel &pixel)
{
    pixel.GetRoc().SetDAC(DACParameters::Vana, vana[pixel.GetRoc().GetChipId()]);
    tbInterface->Flush();
    psi::Sleep(2.0 * psi::seconds);

    SetThreshold(pixel, vcalThreshold);
    psi::Time tw = TimeWalk(pixel.GetRoc(), 5);
    psi::LogInfo() << "time shift " << tw << std::endl;
    twBefore[pixel.GetRoc().GetChipId()] = tw;

    psi::ElectricCurrent current = tbInterface->GetIA();
    psi::LogInfo() << "current " << current - zeroCurrent << std::endl;

    psi::ElectricCurrent goalCurrent = current - zeroCurrent + tw * powerSlope;
    if (goalCurrent > 0.030 * psi::amperes) goalCurrent = 0.030 * psi::amperes;
    if (goalCurrent < 0.018 * psi::amperes) goalCurrent = 0.018 * psi::amperes;

    psi::LogDebug() << "[TimeWalkStudy] Goal Current " << goalCurrent << std::endl;

    int vana = pixel.GetRoc().AdjustVana(zeroCurrent, goalCurrent);
    pixel.GetRoc().SetDAC(DACParameters::Vana, vana);

    psi::DataStorage::Active().SaveMeasurement((boost::format("IA_C%1%") % pixel.GetRoc().GetChipId()).str(),
                                               goalCurrent);
    psi::Sleep(2.0 * psi::seconds);
    tbInterface->Flush();

    //check result
    SetThreshold(pixel, vcalThreshold);
    twAfter[pixel.GetRoc().GetChipId()] = TimeWalk(pixel.GetRoc(), 5);

    return vana;
}


void TimeWalkStudy::GetPowerSlope(TestPixel& pixel)
{
    const int nPoints = 7;
    double x[nPoints], y[nPoints];
    psi::ElectricCurrent  iana[nPoints] = {0.030 * psi::amperes, 0.028 * psi::amperes, 0.026 * psi::amperes,
                                           0.024 * psi::amperes, 0.022 * psi::amperes, 0.020 * psi::amperes,
                                           0.018 * psi::amperes
                                          };

    for(int i = 0; i < nPoints; i++) {
        pixel.GetRoc().AdjustVana(zeroCurrent, iana[i]);
        SetThreshold(pixel, vcalThreshold);

        TimeWalk(pixel.GetRoc(), 5);

        double fp[4];
        for(int j = 0; j < 4; j++) fp[j] = fit->GetParameter(j);
        y[i] = psi::DataStorage::ToStorageUnits(iana[i]);
        x[i] = (pow((fp[0] / (200 - fp[3])), 1 / fp[1]) + fp[2]);
    }

    TGraph *gr1 = new TGraph(nPoints, x, y);
    TF1 *ff = new TF1("ff", "[0]*x+[1]", 10, 60);
    gr1->Fit("ff", "RQ");
    powerSlope = psi::DataStorage::FromStorageUnits<psi::CurrentPerTime>(ff->GetParameter(0));

    psi::LogDebug() << "[TimeWalkStudy] Power Slope " << powerSlope << std::endl;

    pixel.GetRoc().SetDAC(DACParameters::Vana, vana[pixel.GetRoc().GetChipId()]);
    tbInterface->Flush();
    histograms->Add(gr1);
    new TCanvas();
    gr1->Draw("A*");
}


void TimeWalkStudy::CalDelDeltaT(TestRoc& roc)
{
    double count = 0.;
    unsigned char res[256];
    int nTrigs = 10;

    int calDelSAVED = roc.GetDAC(DACParameters::CalDel);
    tbInterface->ScanAdac(roc.GetChipId(), 26, 0, 255, 1, nTrigs, 10, res);
    roc.SetDAC(DACParameters::CalDel, calDelSAVED);
    tbInterface->DataCtrl(true, false);   //to clear fifo buffer
    tbInterface->Flush();

    for(int i = 0; i < 255; i++) count += res[i];
    calDelDT = count / nTrigs;

    psi::LogDebug() << "[TimeWalkStudy] dt " << calDelDT << std::endl;
}


int TimeWalkStudy::GetThreshold(TestPixel& pixel)
{
    int wbc = pixel.GetRoc().GetDAC(DACParameters::WBC);
    int nTrig = 10;
    int thr = pixel.GetRoc().PixelThreshold(pixel.GetColumn(), pixel.GetRow(), 0, 1, nTrig, 2 * nTrig, 25, false,
                                            false, 0);
    pixel.GetRoc().SetDAC(DACParameters::WBC, wbc - 1);
    tbInterface->Flush();
    int thr2 = pixel.GetRoc().PixelThreshold(pixel.GetColumn(), pixel.GetRow(), 0, 1, nTrig, 2 * nTrig, 25, false,
                                             false, 0);
    pixel.GetRoc().SetDAC(DACParameters::WBC, wbc);
    tbInterface->Flush();
    return TMath::Min(thr, thr2);
}


void TimeWalkStudy::SetThreshold(TestPixel& pixel, int vcal)
{
    int vtrim = 0, thr, thrOld;
    int vcalSAVED = pixel.GetRoc().GetDAC(DACParameters::Vcal);
    int vthrComp = pixel.GetRoc().GetDAC(DACParameters::VthrComp);

    pixel.GetRoc().SetDAC(DACParameters::Vtrim, 0);
    tbInterface->Flush();

    thr = GetThreshold(pixel);
    if ((thr < 100) && (vthrComp - 10 > 0))
        pixel.GetRoc().SetDAC(DACParameters::VthrComp, vthrComp - 10); //if untrimmed threshold is below 100, increase threshold

    do {
        if (thr > vcal + 20) vtrim += 10;
        else if (thr > vcal + 10) vtrim += 5;
        else if (thr > vcal + 5) vtrim += 2;
        else vtrim++;
        pixel.GetRoc().SetDAC(DACParameters::Vtrim, vtrim);
        tbInterface->Flush();
        thrOld = thr;
        thr = GetThreshold(pixel);
    } while (((thr > vcal) || (thrOld > vcal)) && (vtrim < 255));
    pixel.GetRoc().SetDAC(DACParameters::Vcal, vcalSAVED);
    tbInterface->DataCtrl(true, false);
    pixel.SetTrim(0);
    pixel.ArmPixel(); //pixel was masked after PixelThreshold()
    tbInterface->Flush();
    psi::LogInfo() << "Vtrim set to " << vtrim << std::endl;
}
