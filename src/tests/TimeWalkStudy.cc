/*!
 * \file TimeWalkStudy.cc
 * \brief Implementation of TimeWalkStudy class.
 *
 * \b Changelog
 * 02-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using psi::Sleep instead interface/Delay.
 * 25-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - DataStorage moved into psi namespace.
 * 21-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using DataStorage class to save the results.
 * 15-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using boost::units::quantity to represent physical values.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
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

TimeWalkStudy::TimeWalkStudy(TestRange *aTestRange, TBInterface *aTBInterface)
{
    testRange = aTestRange;
    tbInterface = aTBInterface;
    ReadTestParameters();
    debug = true;

    fit = new TF1("fitfn", "[0]/((x-[2])^[1])+[3]", 10., 60.);
    fit->SetParameters(100000., 1.7, 0., 80.);
}

void TimeWalkStudy::ReadTestParameters()
{
    vcalThreshold = 60;
    meanShift = TestParameters::Singleton().TWMeanShift();
}

void TimeWalkStudy::ModuleAction()
{
    int nRocs = module->NRocs();
    for (int iRoc = 0; iRoc < nRocs; iRoc++)
    {
        vana[iRoc] = module->GetRoc(iRoc)->GetDAC("Vana");
        module->GetRoc(iRoc)->SetDAC("Vana", 0);
    }
    Flush();
    psi::Sleep(2.0 * psi::seconds);
    zeroCurrent = ((TBAnalogInterface*)tbInterface)->GetIA();

    Test::ModuleAction();

    TH1F *histoBefore = new TH1F("twBefore", "twBefore", 100, -10., 10.);
    TH1F *histoAfter = new TH1F("twAfter", "twAfter", 100, -10., 10.);

    for (int iRoc = 0; iRoc < nRocs; iRoc++)
    {
        module->GetRoc(iRoc)->SetDAC("Vana", vana[iRoc]);
        if (testRange->IncludesRoc(iRoc)) histoBefore->Fill( psi::DataStorage::ToStorageUnits(twBefore[iRoc]));
        if (testRange->IncludesRoc(iRoc)) histoAfter->Fill( psi::DataStorage::ToStorageUnits(twAfter[iRoc]));
    }
    histograms->Add(histoBefore);
    histograms->Add(histoAfter);
}


void TimeWalkStudy::RocAction()
{
    psi::LogInfo() << "[TimeWalkStudy] ROC #" << chipId << '.' << std::endl;

    psi::LogInfo().PrintTimestamp();

    //init pixel
    SaveDacParameters();
    SetDAC("Vana", vana[chipId]);
    SetPixel(roc->GetPixel(26, 5)); //pixel in lower half of the chip
    int trim = pixel->GetTrim();
    roc->ArmPixel(column, row);
    Flush();

    //do test
    CalDelDeltaT();
    GetPowerSlope();
    vana[chipId] = FindNewVana();

    //restore settings
    pixel->SetTrim(trim);
    roc->DisarmPixel(column , row);
    RestoreDacParameters();
}


psi::Time TimeWalkStudy::TimeWalk(int vcalStep)
{
    double fp[4];
    unsigned short res[1000], lres = 1000;
    fit->SetParameters(100000., 1.7, 0., 80.);

    int calDelSAVED = GetDAC("CalDel"), vcalSAVED = GetDAC("Vcal"), wbcSAVED = GetDAC("WBC");

    ((TBAnalogInterface*)tbInterface)->CdVc(chipId, 97, 102, vcalStep, 90, lres, res);

    SetDAC("CalDel", calDelSAVED);
    SetDAC("Vcal", vcalSAVED);
    SetDAC("WBC", wbcSAVED);
    ((TBAnalogInterface*)tbInterface)->DataCtrl(true, false);
    Flush();

    int t = lres / 3;
    double x[t], y[t];
    for(int i = 0; i < lres; i = i + 3)
    {
        y[i / 3] = res[i];
        x[i / 3] = ((102 - res[i + 2] + 1) * 25 - 25.0 / calDelDT * res[i + 1] + 0.5);
    }

    TGraph *gr1 = new TGraph(t, x, y);
    gr1->Fit(fit, "RQ");
    histograms->Add(gr1);

    for(int i = 0; i < 4; i++) fp[i] = fit->GetParameter(i);
    return (meanShift - (pow((fp[0] / (200 - fp[3])), 1 / fp[1]) + fp[2])) * psi::seconds; //shift in time
}


int TimeWalkStudy::FindNewVana()
{
    SetDAC("Vana", vana[chipId]);
    Flush();
    psi::Sleep(2.0 * psi::seconds);

    SetThreshold(vcalThreshold);
    psi::Time tw = TimeWalk(5);
    psi::LogInfo() << "time shift " << tw << std::endl;
    twBefore[chipId] = tw;

    psi::ElectricCurrent current = ((TBAnalogInterface*)tbInterface)->GetIA();
    psi::LogInfo() << "current " << current - zeroCurrent << std::endl;

    psi::ElectricCurrent goalCurrent = current - zeroCurrent + tw * powerSlope;
    if (goalCurrent > 0.030 * psi::amperes) goalCurrent = 0.030 * psi::amperes;
    if (goalCurrent < 0.018 * psi::amperes) goalCurrent = 0.018 * psi::amperes;

    psi::LogDebug() << "[TimeWalkStudy] Goal Current " << goalCurrent << std::endl;

    int vana = roc->AdjustVana(zeroCurrent, goalCurrent);
    SetDAC("Vana", vana);

    psi::DataStorage::Active().SaveMeasurement((boost::format("IA_C%1%") % chipId).str(), goalCurrent);
    psi::Sleep(2.0 * psi::seconds);
    Flush();

    //check result
    SetThreshold(vcalThreshold);
    twAfter[chipId] = TimeWalk(5);

    return vana;
}


void TimeWalkStudy::GetPowerSlope()
{
    const int nPoints = 7;
    double x[nPoints], y[nPoints];
    psi::ElectricCurrent  iana[nPoints] = {0.030 * psi::amperes, 0.028 * psi::amperes, 0.026 * psi::amperes,
                                           0.024 * psi::amperes, 0.022 * psi::amperes, 0.020 * psi::amperes,
                                           0.018 * psi::amperes
                                          };

    for(int i = 0; i < nPoints; i++)
    {
        roc->AdjustVana(zeroCurrent, iana[i]);
        SetThreshold(vcalThreshold);

        TimeWalk(5);

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

    SetDAC("Vana", vana[chipId]);
    Flush();
    histograms->Add(gr1);
    new TCanvas();
    gr1->Draw("A*");
}


void TimeWalkStudy::CalDelDeltaT()
{
    double count = 0.;
    unsigned char res[256];
    int nTrigs = 10;

    int calDelSAVED = GetDAC("CalDel");
    ((TBAnalogInterface*)tbInterface)->ScanAdac(chipId, 26, 0, 255, 1, nTrigs, 10, res);
    SetDAC("CalDel", calDelSAVED);
    ((TBAnalogInterface*)tbInterface)->DataCtrl(true, false);   //to clear fifo buffer
    Flush();

    for(int i = 0; i < 255; i++) count += res[i];
    calDelDT = count / nTrigs;

    psi::LogDebug() << "[TimeWalkStudy] dt " << calDelDT << std::endl;
}


int TimeWalkStudy::GetThreshold()
{
    int wbc = GetDAC("WBC"), nTrig = 10;
    int thr = roc->PixelThreshold(column, row, 0, 1, nTrig, 2 * nTrig, 25, false, false, 0);
    SetDAC("WBC", wbc - 1);
    Flush();
    int thr2 = roc->PixelThreshold(column, row, 0, 1, nTrig, 2 * nTrig, 25, false, false, 0);
    SetDAC("WBC", wbc);
    Flush();
    return TMath::Min(thr, thr2);
}


void TimeWalkStudy::SetThreshold(int vcal)
{
    int vtrim = 0, thr, thrOld, vcalSAVED = GetDAC("Vcal"), vthrComp = GetDAC("VthrComp");

    SetDAC("Vtrim", 0);
    Flush();

    thr = GetThreshold();
    if ((thr < 100) && (vthrComp - 10 > 0)) SetDAC("VthrComp", vthrComp - 10); //if untrimmed threshold is below 100, increase threshold

    do
    {
        if (thr > vcal + 20) vtrim += 10;
        else if (thr > vcal + 10) vtrim += 5;
        else if (thr > vcal + 5) vtrim += 2;
        else vtrim++;
        SetDAC("Vtrim", vtrim);
        Flush();
        thrOld = thr;
        thr = GetThreshold();
    }
    while (((thr > vcal) || (thrOld > vcal)) && (vtrim < 255));
    SetDAC("Vcal", vcalSAVED);
    ((TBAnalogInterface*)tbInterface)->DataCtrl(true, false);
    pixel->SetTrim(0);
    ArmPixel(); //pixel was masked after PixelThreshold()
    Flush();
    psi::LogInfo() << "Vtrim set to " << vtrim << std::endl;
}
