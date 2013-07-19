/*!
 * \file TestRoc.cc
 * \brief Implementation of TestRoc class.
 */

#include <iomanip>

#include <TF1.h>
#include <TGraph.h>

#include "psi/log.h"
#include "psi/date_time.h"
#include "TestRoc.h"
#include "TestDoubleColumn.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/CalibrationTable.h"
#include "tests/PHCalibration.h"
#include "analysis/Analysis.h"
#include "tests/PixelAlive.h"
#include "BasePixel/Test.h"
#include "tests/FullTest.h"
#include "tests/BumpBonding.h"
#include "tests/AddressLevels.h"
#include "tests/AddressDecoding.h"
#include "tests/SCurveTest.h"
#include "tests/TrimBits.h"
#include "tests/Trim.h"
#include "tests/TrimVcal.h"
#include "tests/TrimLow.h"
#include "tests/UbCheck.h"
#include "tests/PHCalibration.h"
#include "tests/PHRange.h"
#include "tests/TrimBits.h"
#include "BasePixel/TestRange.h"
#include "tests/DacDependency.h"
#include "tests/TemperatureTest.h"
#include "tests/IVCurve.h"
#include "tests/OffsetOptimization.h"
#include "tests/SCurveTest.h"
#include <TFile.h>

#include <string.h>
#include <sstream>

TestRoc::TestRoc(boost::shared_ptr<TBAnalogInterface> aTBInterface, TestModule& _testModule, int aChipId, int aHubId,
                 int aPortId, int anAoutChipPosition)
    : tbInterface(aTBInterface), testModule(&_testModule), chipId(aChipId), hubId(aHubId), portId(aPortId),
      aoutChipPosition(anAoutChipPosition), dacParameters(new DACParameters()), fullRange(new TestRange())
{
    doubleColumns.assign(psi::ROCNUMDCOLS, boost::shared_ptr<TestDoubleColumn>());
    for (unsigned i = 0; i < psi::ROCNUMDCOLS; i++) {
        doubleColumns[i] = boost::shared_ptr<TestDoubleColumn>(new TestDoubleColumn(tbInterface, *this, i));
    }
    fullRange->CompleteRoc(chipId);
}

TestPixel& TestRoc::GetTestPixel()
{
    for (unsigned i = 5; i < psi::ROCNUMCOLS - 5; i++) { // no pixels near the edge
        for (unsigned k = 5; k < psi::ROCNUMROWS - 5; k++) { // no pixels near the edge
            TestPixel& pixel = GetPixel(i, k);
            if (pixel.IsAlive()) {
                return pixel;
            }
        }
    }
    THROW_PSI_EXCEPTION("Alive pixel for test not found.");
}

void TestRoc::DoTrim()
{
    DoTest(boost::shared_ptr<Test>(new Trim(GetRange(), tbInterface)));
}

void TestRoc::DoTrimVcal()
{
    DoTest(boost::shared_ptr<Test>(new TrimVcal(GetRange(), tbInterface)));
}

void TestRoc::DoTrimLow()
{
    DoTest(boost::shared_ptr<Test>(new TrimLow(GetRange(), tbInterface)));
}

void TestRoc::DoPhCalibration()
{
    DoTest(boost::shared_ptr<Test>(new PHCalibration(GetRange(), tbInterface)));
}

int TestRoc::CountReadouts(int count)
{
    //aoutChipPosition is only relevant for setup with TBM, otherwise count gives sum of all readouts
    return tbInterface->CountReadouts(count, aoutChipPosition);
}

// == Tests =============================================

void TestRoc::ChipTest()
{
    DoTest(boost::shared_ptr<Test>(new BumpBonding(GetRange(), tbInterface)));
    DoTest(boost::shared_ptr<Test>(new TrimBits(GetRange(), tbInterface)));
}

// -- Performs a test for this roc
void TestRoc::DoTest(boost::shared_ptr<Test> aTest)
{
    aTest->RocAction(*this);
}

void TestRoc::ADCSamplingTest()
{
    for (int delay = 0; delay < 40; ++delay) {
        psi::LogDebug() << "[TestRoc] Delay: " << delay << std::endl;

        tbInterface->SetTBParameter(TBParameters::clk, delay);
        tbInterface->SetTBParameter(TBParameters::sda, 17 + delay);
        tbInterface->SetTBParameter(TBParameters::ctr, 15 + delay);
        tbInterface->SetTBParameter(TBParameters::tin, 10 + delay);
        tbInterface->Flush();
        tbInterface->ADC();
    }
}

void TestRoc::PhError()
{
    int offset;
    int nReadouts = 1000;
    short data[psi::FIFOSIZE];
    unsigned short count;
    ArmPixel(20, 20);
    offset = chipId * 3 + 16;

    int vcal[10];
    vcal[0] = 50;
    vcal[1] = 100;
    vcal[2] = 150;
    vcal[3] = 200;
    vcal[4] = 250;
    vcal[5] = 30;
    vcal[6] = 50;
    vcal[7] = 70;
    vcal[8] = 90;
    vcal[9] = 200;

    SetDAC(DACParameters::VthrComp, 116);
    SetDAC(DACParameters::CtrlReg, 0);
    for (int Tvcal = 0; Tvcal < 10; Tvcal++)
        //for (int Tvcal = 0; Tvcal < 260; Tvcal+=50)
    {
        if (Tvcal > 4) SetDAC(DACParameters::CtrlReg, 4);
        SetDAC(DACParameters::Vcal, vcal[Tvcal]);
        //SetDAC("VIbias_roc",Tvcal);
        TH1D *phHist = new TH1D(Form("phHistVcal%d", vcal[Tvcal]), Form("phHistVcal%d", vcal[Tvcal]), 4000, -2000., 2000.);
        for (int i = 0; i < nReadouts; i++) {
            tbInterface->ADCRead(data, count, 1);
            if (count > offset) phHist->Fill(data[offset]);
        }
        psi::LogInfo() << "Vcal = " << vcal[Tvcal] << ", PH " << std::setprecision(1) << phHist->GetMean()
                       << "+- " << std::setprecision(2) << phHist->GetRMS() << std::endl;
    }

}

void TestRoc::Test1()
{
    int offset;
    if (tbInterface->TBMPresent()) offset = 16;
    else offset = 9;
    int nTrig = 10;

    SetDAC(DACParameters::CtrlReg, 4);

    for (int col = 0; col < 2; col++) {
        psi::LogInfo() << "col = " << col << std::endl;
        for (int row = 0; row < 2; row++) {
            psi::LogInfo() << "row = " << row << std::endl;
            TestPixel& pixel = GetPixel(col, row);
            pixel.ArmPixel();
            for (int vsf = 150; vsf < 255; vsf += 20) {
                GetDAC(DACParameters::Vsf);
                SetDAC(DACParameters::Vsf, vsf);
                Flush();
                short result[256];
                tbInterface->PHDac(25, 256, nTrig, offset + aoutChipPosition * 3, result);
                TH1D *histo = new TH1D(Form("Vsf%d_Col%d_Row%d", vsf, col, row), Form("Vsf%d_Col%d_Row%d", vsf, col, row), 256, 0., 256.);
                for (int dac = 0; dac < 256; dac++) {
                    psi::LogInfo() << "result = " << result[dac] << std::endl;
                    if (result[dac] == 7777) histo->SetBinContent(dac + 1, 555);
                    else histo->SetBinContent(dac + 1, result[dac]);
                }
            }
            pixel.DisarmPixel();
        }
    }
}

void TestRoc::Rainbow()
{
    TH2D *histo = new TH2D("thr", "thr", 255, 0., 255., 255, 0., 255.);
    int col = 5, row = 5, nTrig = 10, thr = 0;

//        EnableAllPixels();
    ArmPixel(col, row);

    for (int i = 100; i < 170; i += 1) {
        SetDAC(DACParameters::VthrComp, i);
        for (int k = 0; k < 5; k += 1) {
            SetDAC(DACParameters::Vtrim, k);
            Flush();
            thr = PixelThreshold(col, row, 0, 1, nTrig, 2 * nTrig, 25, false, false, 0);
            histo->SetBinContent(i, k, thr);
            psi::LogInfo() << "vthr " << i << " vtrim " << k << " " << thr << std::endl;
        }
    }
}

void TestRoc::Rainbow2()
{
    TH2D *histo = new TH2D("alive", "alive", 255, 0., 255., 255, 0., 255.);
    double data[psi::ROCNUMROWS * psi::ROCNUMCOLS];
    int nTrig = 5, nAlive;

//        SetTrim(0.);
    tbInterface->SetEnableAll(1);
    Flush();

    for (int i = 120; i < 121; i += 5) {
        SetDAC(DACParameters::VthrComp, i);
        for (int k = 0; k < 200; k += 2) {
            SetDAC(DACParameters::Vtrim, k);
            Flush();

            ChipEfficiency(nTrig, data);

            nAlive = 0;
            for (unsigned l = 0; l < psi::ROCNUMROWS * psi::ROCNUMCOLS; l++) {
                if (data[l] == 1.) nAlive++;
            }

            histo->SetBinContent(i, k, nAlive);
            psi::LogInfo() << "vthr " << i << " vtrim " << k << " " << nAlive << std::endl;
            if (nAlive == 0) break;
        }
    }
}

// -- Function where you can quickly implement some tests
// -- This code is not persistent
void TestRoc::TestM()
{
    for (unsigned i = 0; i < psi::ROCNUMCOLS; i++) {
        for (unsigned k = 0; k < psi::ROCNUMROWS; k++) {
            EnablePixel(i, k);
        }
    }

    for (int i = 0; i < 26; i++) {
        EnableDoubleColumn(i * 2);
        GetPixel(i * 2, 6).ArmPixel();
    }
}

// -- Tests how the ROC reacts after power on
void TestRoc::PowerOnTest(int nTests)
{
    TestPixel& pixel = GetTestPixel();
    pixel.EnablePixel();
    pixel.Cal();
    GetDoubleColumnByColumnId(pixel.GetColumn()).EnableDoubleColumn();
    TH1D* histo = new TH1D("PowerOnTest", "PowerOnTest", nTests, 0., nTests);
    for (int i = 0; i < nTests; i++) {
        histo->SetBinContent(i + 1, pixel.FindThreshold(10));
        psi::Sleep(20.0 * psi::milli * psi::seconds);
    }
}

// Tries to automatically adjust Vana, may not work yet
int TestRoc::AdjustVana(psi::ElectricCurrent current0, psi::ElectricCurrent goalcurrent)
{
    int vana = 140;
    psi::ElectricCurrent currentMeasured, currentMeasuredOld;
    SetDAC(DACParameters::Vana, vana);
    Flush();
    sleep(1);
    currentMeasured = tbInterface->GetIA();

    //guess value, slope is roughly 0.5 mA / DAC

    psi::ElectricCurrent currentDiff = currentMeasured - current0 - goalcurrent;
    int dacDiff = (int)(currentDiff / (0.0005 * psi::amperes));
    vana -= dacDiff;
    if (vana < 0) vana = 0;
    if (vana > 255) vana = 255;
    SetDAC(DACParameters::Vana, vana);
    Flush();
    sleep(1);
    currentMeasured = tbInterface->GetIA();

    if (currentMeasured < current0 + goalcurrent) {
        do {
            vana++;
            SetDAC(DACParameters::Vana, vana);
            Flush();
            sleep(.1);
            currentMeasuredOld = currentMeasured;
            currentMeasured = tbInterface->GetIA();
        } while (currentMeasured < current0 + goalcurrent  && vana < 255);
        if (psi::abs(currentMeasuredOld - current0 - goalcurrent)
                < psi::abs(currentMeasured - current0 - goalcurrent)) {
            vana--;
            currentMeasured = currentMeasuredOld;
        }
    } else {
        do {
            vana--;
            SetDAC(DACParameters::Vana, vana);
            Flush();
            sleep(0.1);
            currentMeasuredOld = currentMeasured;
            currentMeasured = tbInterface->GetIA();
        } while (currentMeasured > current0 + goalcurrent  && vana > 0);
        if (psi::abs(currentMeasuredOld - current0 - goalcurrent)
                < psi::abs(currentMeasured - current0 - goalcurrent)) {
            vana++;
            currentMeasured = currentMeasuredOld;
        }
    }

    psi::LogDebug() << "[TestRoc] Vana is set to " << vana
                    << " Current: " << ( currentMeasured - current0) << std::endl;

    return vana;
}

void TestRoc::AdjustCalDelVthrComp()
{
    int calDel = 0, vthrComp = 0;

    int vOffsetOp = GetDAC(DACParameters::VoffsetOp);
    SetDAC(DACParameters::VoffsetOp, 255);  //make sure that ph above ub

    //AS 24/05/06 - to be fixed Row:Col below

    AdjustCalDelVthrComp(20, 20, 200, -50);

    calDel += GetDAC(DACParameters::CalDel);
    vthrComp += GetDAC(DACParameters::VthrComp);

    SetDAC(DACParameters::CalDel, calDel);
    SetDAC(DACParameters::VthrComp, vthrComp);
    SetDAC(DACParameters::VoffsetOp, vOffsetOp);
    Flush();

    psi::LogDebug() << "[TestRoc] CalDel   is set to " << calDel   << std::endl;
    psi::LogDebug() << "[TestRoc] VthrComp is set to " << vthrComp << std::endl;
}

void TestRoc::AdjustCalDelVthrComp(int column, int row, int vcal, int belowNoise)
{
    bool verbose = false;
    int sCurve[180], nTrig = 5, nTrials = 5, n = 0, testColumn = column, testRow = row;
    int calDel, vthr;
    int oldCalDel = GetDAC(DACParameters::CalDel);
    int oldVthrComp = GetDAC(DACParameters::VthrComp);
    TH2D *histo;

    SetDAC(DACParameters::Vcal, vcal);
    Flush();

    do {
        TestPixel& pixel = GetPixel(testColumn, testRow);
        EnableDoubleColumn(pixel.GetColumn());

        boost::shared_ptr<TestRange> testRange(new TestRange());
        testRange->AddPixel(chipId, testColumn, testRow);
        DacDependency dacTest(testRange, tbInterface);
        dacTest.SetDacs(DACParameters::CalDel, DACParameters::VthrComp, 180, 180);
        dacTest.SetNTrig(nTrig);
        dacTest.RocAction(*this);
        histo = (TH2D*)(dacTest.GetHistos()->First());

        n++;
        testColumn = (testColumn + 1) % psi::ROCNUMCOLS;

        double vthrMax = 0., vthrMin = 179., sum ;
        vthr = 179;
        do {
            sum = 0.;
            for (int caldel = 0; caldel < 180; caldel++) sum += histo->GetBinContent(caldel + 1, (int)vthr + 1);
            if (sum > nTrig * 20) vthrMax = vthr;
            vthr--;
        } while (vthrMax == 0. && vthr > 0);

        vthr = 0;
        do {
            sum = 0.;
            for (int caldel = 0; caldel < 180; caldel++) sum += histo->GetBinContent(caldel + 1, (int)vthr + 1);
            if (sum > nTrig * 20) vthrMin = vthr;
            vthr++;
        } while (vthrMin == 179. && vthr < 180);

        if (verbose)
            psi::LogDebug() << "[TestRoc] vthr range [ " << vthrMin << ", "
                            << vthrMin << "]." << std::endl;

        if (belowNoise == 0) vthr = static_cast<int>( (vthrMax + vthrMin) / 2);
        else if (belowNoise >  0) vthr = static_cast<int>( vthrMax - belowNoise);
        else if (belowNoise <  0) vthr = static_cast<int>( vthrMin - belowNoise);

        for (int caldel = 0; caldel < 180; caldel++) sCurve[caldel] = (int)histo->GetBinContent(caldel + 1, (int)vthr + 1);

        int calDel1 = (int)Threshold(sCurve, 0, 1, 1, nTrig - 1);
        int calDel2 = (int)Threshold(sCurve, 179, -1, 1, nTrig - 1);
        calDel = (calDel1 + calDel2) / 2;
    } while ((n < nTrials) && ((histo->GetMaximum() == 0) || (histo->GetBinContent(calDel + 1, vthr + 1) != nTrig)));
    if (n == nTrials) {
        psi::LogInfo() << "[TestRoc] Error: Can not adjust VthrComp and CalDel."
                       << std::endl;

        vthr = oldVthrComp;
        calDel = oldCalDel;
    }

    SetDAC(DACParameters::VthrComp, vthr);
    if (verbose)
        psi::LogDebug() << "[TestRoc] VthrComp is set to " << vthr << std::endl;

    SetDAC(DACParameters::CalDel, calDel);
    if (verbose)
        psi::LogDebug() << "[TestRoc] CalDel is set to " << calDel << std::endl;
}

void TestRoc::AdjustUltraBlackLevel(int ubLevel)
{
    //psi::LogInfo() << "starting <TestRoc::AdjustUltraBlackLevel>: chipId = " << chipId << endl;

    int vibias = 90, ubPosition = 8 + aoutChipPosition * 3;
    short data[10000];
    unsigned short count;

    SetDAC(DACParameters::Ibias_DAC, vibias);
    Flush();
    tbInterface->ADCData(data, count);

    int levelMeasured = data[ubPosition], levelMeasuredOld;
//  psi::LogInfo() << "Ibias = " << vibias << " (start value) : measured UB level = " << levelMeasured << "; target = " << ubLevel << endl;
    if (levelMeasured > ubLevel) {
        do {
            vibias++;
            SetDAC(DACParameters::Ibias_DAC, vibias);
            Flush();
            tbInterface->ADCData(data, count);
            levelMeasuredOld = levelMeasured;
            levelMeasured = data[ubPosition];
        } while (levelMeasured > ubLevel && vibias < 255);
        if (TMath::Abs(levelMeasuredOld - ubLevel) < TMath::Abs(levelMeasured - ubLevel)) {
            vibias--;
            levelMeasured = levelMeasuredOld;
        }
    } else {
        do {
            vibias--;
            SetDAC(DACParameters::Ibias_DAC, vibias);
            Flush();
            tbInterface->ADCData(data, count);
            levelMeasuredOld = levelMeasured;
            levelMeasured = data[ubPosition];
        } while (levelMeasured < ubLevel  && vibias > 0);
        if (TMath::Abs(levelMeasuredOld - ubLevel) < TMath::Abs(levelMeasured - ubLevel)) {
            vibias++;
            levelMeasured = levelMeasuredOld;
        }
    }

    SetDAC(DACParameters::Ibias_DAC, vibias);

    psi::LogDebug() << "[TestRoc] Ibias_DAC is set to " << vibias << std::endl;
    psi::LogDebug() << "[TestRoc] ubLevel " << levelMeasured << std::endl;
}

double TestRoc::Threshold(int sCurve[], int start, int sign, int step, double thrLevel)
{
    int threshold = start;
    double result;

    //avoid the problem, that the first entry is not zero, because the DAC was not set fast enough
    if ((sCurve[start] == thrLevel) && (sCurve[start + sign * step] == 0) && (sCurve[start + 2 * sign * step] == 0)) sCurve[start] = 0;

    if (sCurve[threshold] > thrLevel) {
        do {
            threshold -= sign * step;
        } while ((sCurve[threshold] > thrLevel) && (threshold > (step - 1)) && (threshold < (256 - step)));

        if (sCurve[threshold + sign * step] == sCurve[threshold]) result = threshold;
        else result = threshold + (double)(sign * step) * (double)(thrLevel - sCurve[threshold]) / (double)(sCurve[threshold + sign * step] - sCurve[threshold]);
    } else {
        do {
            threshold += sign * step;
        } while ((sCurve[threshold] <= thrLevel) && (threshold > (step - 1)) && (threshold < (256 - step)));

        if (sCurve[threshold - sign * step] == sCurve[threshold]) result = threshold;
        else result = threshold - (double)(sign * step) * (double)(thrLevel - sCurve[threshold]) / (double)(sCurve[threshold - sign * step] - sCurve[threshold]);
    }

    result = TMath::Min(result, 255.);
    result = TMath::Max(result, 0.);

    return result;
}

void TestRoc::SendSignals(int start, int stop, int step, int nTrig, DACParameters::Register dacReg)
{
    for (int i = start; i < stop; i += step) {
        SetDAC(dacReg, i);
        if (i == start) CDelay(1000);  // The jump from a very high value down to very low value may need more time
        SendCal(nTrig);
    }
}

void TestRoc::ReadSignals(int start, int stop, int step, int nTrig, int sCurve[])
{
    for (int i = 0; i < 256; i++) sCurve[i] = 0;
    for (int i = start; i < stop; i += step) {
        for (int k = 0; k < nTrig; k++) {
            sCurve[i] += RecvRoCnt();
        }
    }
}

// == Histos =================================================================
TH2D* TestRoc::TrimMap(unsigned mapId)
{
    std::ostringstream name;
    name << "TrimMap_C" << chipId;
    if(mapId)
        name << "_nb" << mapId;
    TH2D *map = new TH2D(name.str().c_str(), name.str().c_str(), psi::ROCNUMCOLS, 0, psi::ROCNUMCOLS, psi::ROCNUMROWS,
                         0, psi::ROCNUMROWS);
    for (unsigned i = 0; i < psi::ROCNUMCOLS; i++) {
        for (unsigned k = 0; k < psi::ROCNUMROWS; k++) {
            map->SetBinContent(i + 1, k + 1, GetPixel(i, k).GetTrim());
        }
    }
    return map;
}

Double_t Fitfcn2( Double_t *x, Double_t *par)
{
    return par[1] * x[0] + par[0];
}

double TestRoc::GetTemperature()
{
    bool debug = false;
    psi::LogInfo() << "[TestRoc] Temperature for ROC #" << chipId << std::endl;

    const int nTriggers = 10;
    int temp[8], calib[8];

    // get black level
    unsigned short count;
    short data[10000], blackLevel;

    tbInterface->ADCRead(data, count, nTriggers);
    blackLevel = data[9 + aoutChipPosition * 3];
    if (debug)
        psi::LogDebug() << "[TestRoc] blackLevel " << blackLevel << std::endl;

    // Calibrate

    for (int rangeTemp = 0; rangeTemp < 8; rangeTemp++) {
        SetDAC(DACParameters::RangeTemp, rangeTemp + 8);
        Flush();
        calib[rangeTemp] = tbInterface->LastDAC(nTriggers, aoutChipPosition);
        if (debug)
            psi::LogDebug() << "[TestRoc] Calib " << calib[rangeTemp] << std::endl;
    }

    // Measure temperature

    for (int rangeTemp = 0; rangeTemp < 8; rangeTemp++) {
        SetDAC(DACParameters::RangeTemp, rangeTemp);
        Flush();
        temp[rangeTemp] = tbInterface->LastDAC(nTriggers, aoutChipPosition);
        if (debug)
            psi::LogDebug() << "[TestRoc] Temperature " << temp[rangeTemp] << std::endl;
    }

    // Compute voltage

    int n = 0;
    double x[8], y[8];

    for (int i = 0; i < 8; i++) {
        if (TMath::Abs(calib[i] - blackLevel) > 20) {
            x[n] = calib[i] - blackLevel;
            y[n] = -70. + (7 - i) * 164. / 7.;
            n++;
        }
    }
    if (n == 0) return -9999.;
    if (debug)
        psi::LogDebug() << "[TestRoc] n = " << n << std::endl;

    if (debug)
        for (int i = 0; i < n; i++)
            psi::LogDebug() << "[TestRoc] x " << x[i] << ", y " << y[i] << std::endl;

    TGraph *graph = new TGraph(n, x, y);
    TF1 *fit = new TF1("fit", Fitfcn2, 0., 2000., 2);
    fit->SetParameter(0, 50.);
    fit->SetParameter(1, 0.1);
    graph->Fit("fit", "NRQ");

    double voltage = 400. + fit->Eval(temp[0] - blackLevel);
    if (debug)
        psi::LogDebug() << "[TestRoc] Voltage " << voltage << std::endl;

    //Convert to temperature
    double temperature = (voltage - 410.) * 5. / 8.;
    psi::LogDebug() << "[TestRoc] Temperature " << temperature << std::endl;

    delete graph;
    delete fit;

    return temperature;
}

void TestRoc::TrimVerification()
{
    SaveDacParameters();
    ThresholdMap thresholdMap;
    thresholdMap.SetDoubleWbc();
    TH2D *map = thresholdMap.MeasureMap(ThresholdMap::VcalThresholdMapParameters, *this, *GetRange(), 5);
    Analysis::Distribution(map, 255, 0., 255.);
    RestoreDacParameters();
}

void TestRoc::ThrMaps()
{
    ThresholdMap thresholdMap;
    thresholdMap.SetDoubleWbc(); //absolute threshold (not in-time)

    SaveDacParameters();

    int vthrComp = GetDAC(DACParameters::VthrComp);

    SetDAC(DACParameters::VthrComp, vthrComp);
    psi::LogInfo() << "VthrComp " << vthrComp << std::endl;
    Flush();

    TH2D* vcalMap = thresholdMap.MeasureMap(ThresholdMap::VcalThresholdMapParameters, *this, *GetRange(), 5);
    vcalMap->SetNameTitle(Form("VcalThresholdMap_C%i", chipId), Form("VcalThresholdMap_C%i", chipId));
    vcalMap->Write();
    TH1D* vcalMapDistribution = Analysis::Distribution(vcalMap);
    vcalMapDistribution->Write();

    RestoreDacParameters();
}

// added by Tilman Oct. 2009 for Pulseshape determination
//
void TestRoc::DoPulseShape()
{
    psi::LogInfo() << "==================================================================================\n";
    psi::LogInfo() << "=                                                                                =\n";
    psi::LogInfo() << "=    Third try of an implementation of the rise time estimation.                 =\n";
    psi::LogInfo() << "=    .                                                                           =\n";
    psi::LogInfo() << "=                                                                                =\n";
    psi::LogInfo() << "=    At the moment this program loops over every 4th pixel in each direction     =\n";
    psi::LogInfo() << "=    at Vcal 120                                                                 =\n";
    psi::LogInfo() << "=                                                          Hank, March 2010     =\n";
    psi::LogInfo() << "==================================================================================\n";

    int c_skip = 4;
    int r_skip = 4;
    int c_num = 52;
    int r_num =  80;
    int c_offset = 0;
    int r_offset = 0;
    int i_max = c_num / c_skip;
    int j_max = r_num / r_skip;

    TH2D *risetime = new TH2D("RiseTime", "Risetime", c_num + 1, -1, c_num, r_num + 1, -1, r_num);
    double time;
    for (int i = 0; i < (i_max); i++) {
        for (int j = 0; j < (j_max); j++) {
            time = DoPulseShape(c_skip * i + c_offset, r_skip * j + r_offset, 120);
            risetime->SetBinContent(c_skip * i + c_offset + 1, r_skip * j + r_offset + 1, time);
        }
    }
    risetime->Write();
}

double TestRoc::DoPulseShape(int column, int row, int vcal)
{
    int nTrig = 5, testColumn = column, testRow = row, testVcal = vcal;
    psi::LogInfo() << "Find pulse shape pixel column " << column << " row " << row << std::endl;

    // 1st Step scan Vthr vs CalDel

    psi::LogInfo() << "Scan Vthr vs CalDel, Vcal = " << testVcal << std::endl;
    int oldCalDel = GetDAC(DACParameters::CalDel);
    int oldVthrComp = GetDAC(DACParameters::VthrComp);
    ;
    TH2D *ptVthrVsCalDel;
    SetDAC(DACParameters::Vcal, testVcal);
    Flush();

    TestPixel& pixel = GetPixel(testColumn, testRow);
    EnableDoubleColumn(pixel.GetColumn());

    boost::shared_ptr<TestRange> testRange(new TestRange());
    testRange->AddPixel(chipId, testColumn, testRow);
    DacDependency dacTest(testRange, tbInterface);
    dacTest.SetDacs(DACParameters::CalDel, DACParameters::VthrComp, 256, 256);
    dacTest.SetNTrig(nTrig);
    dacTest.RocAction(*this);

    SetDAC(DACParameters::CalDel, oldCalDel); // restore old CalDel value

    ptVthrVsCalDel = (TH2D*)(dacTest.GetHistos()->First());
    ptVthrVsCalDel->Write();

    psi::LogInfo() << "Scan Vthr vs CalDel finished" << std::endl;
    psi::LogInfo() << "===" << std::endl;
    psi::LogInfo() << "Find lowest threshold for Vcal vs CalDel scan" << std::endl;

    // Find Lowest Threshold: move from top of plot (in Oy)
    int minThreshold = 0;
    for (int binCounterY = ptVthrVsCalDel->GetNbinsY(); binCounterY > 0; binCounterY--) {
        int nBins = 0;
        for (int binCounterX = 1; binCounterX < ptVthrVsCalDel->GetNbinsX(); binCounterX++) {
            if (ptVthrVsCalDel->GetBinContent(binCounterX, binCounterY) == nTrig) nBins++;
        }
        if ((nBins > 15) && (nBins < 80)) { // if too many bins are "effective" this is due to noise
            minThreshold = binCounterY; // in all those histograms binNumber = value
            break;
        }
    }

    minThreshold -= 10 ; // just for safety

    psi::LogInfo() << "Lowest threshold - 10 is " << minThreshold << std::endl;
    psi::LogInfo() << "============================" << std::endl;


    //=======================================================

    psi::LogInfo() << "Scan Vthr vs Vcal, CalDel = " << oldCalDel << std::endl;

    TH2D *ptVthrVsVcal; //pointer

    DacDependency dacTest2(testRange, tbInterface);
    dacTest2.SetDacs(DACParameters::Vcal, DACParameters::VthrComp, 256, 256);
    dacTest2.SetNTrig(nTrig);
    dacTest2.RocAction(*this);

    ptVthrVsVcal = (TH2D*)(dacTest2.GetHistos()->First());
    TH2D hVthrVsVcal(*ptVthrVsVcal);//histogram
    // rename
    char hisName[100], hisNameBase[100];
    sprintf(hisNameBase, hVthrVsVcal.GetName());
    sprintf(hisName, "%s_WBC", hisNameBase);

    hVthrVsVcal.SetName(hisName);
    psi::LogInfo() << "===" << std::endl;


    // ===

    int oldWBC = GetDAC(DACParameters::WBC);
    SetDAC(DACParameters::WBC, oldWBC - 1);
    psi::LogInfo() << "Scan Vthr vs Vcal at WBC-1, CalDel = " << oldCalDel << std::endl;
    psi::LogInfo() << "Old WBC= " << oldWBC << " now use WBC-1 =  " << oldWBC - 1 << std::endl;

    TH2D *ptVthrVsVcalWBCm1;//pointer

    DacDependency dacTest3(testRange, tbInterface);
    dacTest3.SetDacs(DACParameters::Vcal, DACParameters::VthrComp, 256, 256);
    dacTest3.SetNTrig(nTrig);
    dacTest3.RocAction(*this);
    SetDAC(DACParameters::WBC, oldWBC); // restore old WBC value


    ptVthrVsVcalWBCm1 = (TH2D*)(dacTest3.GetHistos()->First());
    TH2D hVthrVsVcalWBCm1(*ptVthrVsVcalWBCm1);

    sprintf (hisName, "%s_WBCm1", hisNameBase);
    hVthrVsVcalWBCm1.SetName(hisName);
    psi::LogInfo() << "===" << std::endl;
    psi::LogInfo() << "Add the histograms at WBC and WBC-1 " << std::endl;

    // ===


    SetDAC(DACParameters::WBC, oldWBC - 2);
    psi::LogInfo() << "Scan Vthr vs Vcal at WBC-1, CalDel = " << oldCalDel << std::endl;
    psi::LogInfo() << "Old WBC= " << oldWBC << " now use WBC-1 = " << oldWBC - 2 << std::endl;

    TH2D *ptVthrVsVcalWBCm2;//pointer

    DacDependency dacTest5(testRange, tbInterface);
    dacTest5.SetDacs(DACParameters::Vcal, DACParameters::VthrComp, 256, 256);
    dacTest5.SetNTrig(nTrig);
    dacTest5.RocAction(*this);
    SetDAC(DACParameters::WBC, oldWBC); // restore old WBC value


    ptVthrVsVcalWBCm2 = (TH2D*)(dacTest5.GetHistos()->First());
    TH2D hVthrVsVcalWBCm2(*ptVthrVsVcalWBCm2);

    sprintf (hisName, "%s_WBCm2", hisNameBase);
    hVthrVsVcalWBCm2.SetName(hisName);
    psi::LogInfo() << "===\n" << std::endl;
    psi::LogInfo() << "Add the histograms at WBC and WBC-1 " << std::endl;

    TH2D *hVthrVsVcal_tot(&hVthrVsVcal);
    hVthrVsVcal_tot->Add(&hVthrVsVcalWBCm1);
    hVthrVsVcal_tot->Add(&hVthrVsVcalWBCm2);
    sprintf (hisName, "%s_tot", hisNameBase);
    //	hVthrVsVcal;
    //	hVthrVsVcalWBCm1;
    //	hVthrVsVcalWBCm2;
    hVthrVsVcal_tot->Write(hisName);



    psi::LogInfo() << "into " << hisName << std::endl;
    psi::LogInfo() << "============================" << std::endl;


    // Scan Vcal vs CalDel at Vthr=min to define time == 0
    psi::LogInfo() << "Scan Vcal vs CalDel at Vthr = " << minThreshold << std::endl;
    SetDAC(DACParameters::VthrComp, minThreshold);

    TH2D *ptVcalVsCalDel;

    DacDependency dacTest4(testRange, tbInterface);
    dacTest4.SetDacs(DACParameters::CalDel, DACParameters::Vcal, 256, 256);
    dacTest4.SetNTrig(nTrig);
    dacTest4.RocAction(*this);

    ptVcalVsCalDel = (TH2D*)(dacTest4.GetHistos()->First());

    SetDAC(DACParameters::VthrComp, oldVthrComp); // restore old Vthr value

    ptVcalVsCalDel = (TH2D*)(dacTest4.GetHistos()->First());

    psi::LogInfo() << "Scan Vthr vs CalDel finished\n" << std::endl;
    psi::LogInfo() << "============================\n" << std::endl;

    char name[50];
    char pixelname[15];

    sprintf(pixelname, "PH_C%i_R%i", testColumn, testRow);
    sprintf(name, "CalDelcal_C%i_R%i", testColumn, testRow);
    TH2D *ptCalDelcalib = new TH2D(name, name, 256, 0, 256, 256, 0, 256);

    sprintf(name, "CalDelwidth_C%i_R%i", testColumn, testRow);
    TH1D *ptCalDelwidth = new TH1D(name, name, 256, 0, 256);

    sprintf(name, "Vthrcalib_C%i_R%i", testColumn, testRow);
    TH2D *ptVthrcalib = new TH2D(name, name, 256, 0, 256, 256, 0, 256);

    sprintf(name, "Vthrline_C%i_R%i", testColumn, testRow);
    TH1D *ptVthrline = new TH1D(name, name, 256, 0, 256);

    sprintf(name, "PHdataPoints_C%i_R%i", testColumn, testRow);
    TH2D *ptPHdataPoints = new TH2D(name, name, 256, 0, 256, 256, 0, 256);

    sprintf(name, "PHintcurve_C%i_R%i", testColumn, testRow);
    TH1D *ptPHintcurve = new TH1D(name, name, 256, 0, 256);

    int pulseHeight;

    psi::LogInfo() << "The Pixel is : " << pixelname << std::endl;
    psi::LogInfo() << "The number of bins is " << ptVcalVsCalDel->GetNbinsX() << std::endl;
    psi::LogInfo() << "The nTrig is " << nTrig << std::endl;
    psi::LogInfo() << "The minThreshold is " << minThreshold << std::endl;


    psi::LogInfo() << "Find t_0 from Vthr vs CalDel Scan\n";

    //
    //  Vcal
    //  |         .________.
    //  |         |        |
    //  |         |        |
    //  |         |        |
    //  |         |        |
    //  |         |        |
    //  |         |        |
    //  |        /         |
    //  |      _/         /
    //  |   __/         _/
    //  |__/       ____/
    //  |_________/
    //  |
    //  +-------------------------------- CalDel

    // Find Right Edge (~ Time of Vcal injection)
    // CalDel for large signals and small thresholds = tZero
    int tZero = 0;

    for (int binCounterX = ptVcalVsCalDel->GetNbinsX(); binCounterX > 0; binCounterX--) {
        int nBins = 0;
        for (int binCounterY = ptVcalVsCalDel->GetNbinsY(); binCounterY > 0; binCounterY--) {
            if (ptVcalVsCalDel->GetBinContent(binCounterX, binCounterY) == nTrig) nBins++;
        }
        if (nBins > 2) {
            tZero = binCounterX; // in all those histograms binNumber = value
            break;
        }
    }
    psi::LogInfo() << "  t_0 is at CalDel " << tZero << std::endl;



    // scan the Vcal vs CalDel tornado from
    // top (minThreshold) to bottom and measure
    // right edge (time) and width of the tornado
    // use a very simplistic method:
    // right edge is defined as (last bin with value zero) - (first bin with value nTrig) / 2
    // last bin with zero is defined a the 1st bin from the right with 0 followed by >=3 bins >0
    // first bin with nTrig is defined as the first bin from the right with nTrig followed by >= 2
    // 	bins with nTrig.
    // left edge is defined accordingly
    // only lines with > 30 bins with value nTrig are considered.
    psi::LogInfo() << "Find CalDel cal width from Vcal vs CalDel Scan" << std::endl;
    double widthavg = 0., widthtot = 0.;
    int nLinesUsed = 0;
    for (int binCounterY = ptVcalVsCalDel->GetNbinsY(); binCounterY > 0; binCounterY--) {

        double rightEdge = 256., leftEdge = 0., width = 0.;
        int lastBinZero = 256, nBinNotZero = 0, firstBinnTrig = 0, nBinnTrig = 0;
        // right edge
        for (int binCounterX = ptVcalVsCalDel->GetNbinsX(); binCounterX > 0; binCounterX--) {
            if (ptVcalVsCalDel->GetBinContent(binCounterX, binCounterY) > 0) {
                nBinNotZero++;
                if (nBinNotZero == 3) {
                    lastBinZero = binCounterX - 3;
                }
            } else {
                nBinNotZero = 0;
            }
            if (ptVcalVsCalDel->GetBinContent(binCounterX, binCounterY) == nTrig) {
                nBinnTrig++;
                if (nBinnTrig == 3) {
                    firstBinnTrig = binCounterX - 3;
                    rightEdge = (lastBinZero + firstBinnTrig) / 2;
                    ptCalDelcalib->SetBinContent(lastBinZero, binCounterY, 1);
                    ptCalDelcalib->SetBinContent(firstBinnTrig, binCounterY, 3);
                    break;
                }
            } else {
                nBinnTrig = 0;
            }
        }

        // left edge is only searched if a right edge was found
        int nBinZero = 0;
        int lastBinNotZero = 256,  firstBinnNotTrig = 0, nBinnNotTrig = 0;
        nBinNotZero = 0;
        if (rightEdge < 256.) {
            for (int binCounterX = rightEdge; binCounterX > 0; binCounterX--) {
                if (ptVcalVsCalDel->GetBinContent(binCounterX, binCounterY) < 1) {
                    nBinZero++;
                    if (nBinZero == 3) lastBinNotZero = binCounterX - 3;
                } else {
                    nBinNotZero = 0;
                }
                if (ptVcalVsCalDel->GetBinContent(binCounterX, binCounterY) == 0) {
                    nBinnNotTrig++;
                    if (nBinnNotTrig == 3) {
                        firstBinnNotTrig = binCounterX - 3;
                        leftEdge = (lastBinNotZero + firstBinnNotTrig) / 2.0;
                        ptCalDelcalib->SetBinContent(lastBinNotZero, binCounterY, 2);
                        ptCalDelcalib->SetBinContent(firstBinnNotTrig, binCounterY, 4);
                        break;
                    }
                } else {
                    nBinnNotTrig = 0;
                }
            }
            width = rightEdge - leftEdge;
            ptCalDelwidth->SetBinContent(binCounterY, width);
            if ( width > 30. && width < 80 ) {
                widthtot += width;
                nLinesUsed++;
            }
        }
    }
    ptCalDelcalib->Write();
    widthavg = widthtot / nLinesUsed;
    psi::LogInfo() << "  cal width is " << widthavg << std::endl;


    //scan Vthrcomp vs Vcal
    //The purpose of this test is to scan the Vthrcomp vs. VCal
    //curve to get the linear relation between Vthrcomp and Vcal.
    //It currently takes in the WBC and WBC summed file and
    //Searches from the upper right to bottom left for points
    //where the signal goes from nTrig to 0.  It uses the same
    //matching function as above where it takes the location of
    //nTrig as the last place 3 nTrigs were seen in a row and then
    //The first place a 3 0's are seen and averages the two of them
    //To get the location for that point.  Once the two have been
    //Found for all Vthrcomps, it will fit to a curve and output the
    //calibration factor for Vthrcomp.

// Vthrcomp
//	|
//	|   \------\---------------------
//	|	 \------\--------------------
//	|	  \------\-------------------
//	|	   \------\------------------
//	|		\-------\----------------
//	|		 \---------\-------------
//	|		  \-----------\----------
//	|		   \-------------\-------
//	|			\---------------\----
//  +-------------------------------- Vcal
//  We are looking for the linear line that is at the end, whether it comes from
//  WBC n or if we have to look lower at WBC n-1.  This has already been added
//  Togehter and the curved line should be a point where spurious counts from 5
//	to 10 can occur, so we can assume that the searcher will pass over them and
//  go to the end at the left side for each point.

    psi::LogInfo() << "Find the VthrComp calibration from Vthrcomp vs. Vcal" << std::endl;
    int Vthrmin = 0; //This is the starting threshold for the Vthrcomp vs Vcal calb.
    double leftEdge = 0;
    int numBins;
    for (int binCounterY = hVthrVsVcal_tot->GetNbinsY(); binCounterY > 0; binCounterY--) {
        numBins = 0;
        for( int binCounterX = hVthrVsVcal_tot->GetNbinsX(); binCounterX > 150; binCounterX--) {
            if (hVthrVsVcal_tot->GetBinContent(binCounterX, binCounterY) != 0) {
                numBins ++;
                if (numBins > 10) {
                    Vthrmin = binCounterY - 6;  //Arbitary drop to clear the noise
                    break;
                }
            }
        }
        if(Vthrmin != 0) {
            break;
        }
    }
    for (int binCounterY = Vthrmin; binCounterY > 0; binCounterY--) {
        int nBinZero = 0;
        int lastBinNotZero = 256, firstBinnNotTrig = 0, nBinnNotTrig = 0;
        for (int binCounterX = hVthrVsVcal_tot->GetNbinsX(); binCounterX > 0; binCounterX--) {
            if (hVthrVsVcal_tot->GetBinContent(binCounterX, binCounterY) < nTrig) {
                nBinnNotTrig++;
                if (nBinnNotTrig == 3) {
                    firstBinnNotTrig = binCounterX - 3;
                }
            } else {
                nBinnNotTrig = 0;
            }

            if (hVthrVsVcal_tot->GetBinContent(binCounterX, binCounterY) < 1) {
                nBinZero++;
                if (nBinZero == 3) {
                    lastBinNotZero = binCounterX - 3;
                    leftEdge = (lastBinNotZero + firstBinnNotTrig) / 2.0;
                    ptVthrcalib->SetBinContent(lastBinNotZero, binCounterY, 1);
                    ptVthrcalib->SetBinContent(firstBinnNotTrig, binCounterY, 2);
                    ptVthrline->SetBinContent(binCounterY, leftEdge);
//							psi::LogInfo() << firstBinnNotTrig << "    " << lastBinNotZero << "    " << leftEdge << endl;
                    break;
                }
            } else {
                nBinZero = 0;
            }
        }
    }
    ptVthrline->Write();
    ptVthrline->Draw("A*");
    ptVthrline->Fit("pol1", "Q");
    TF1 *VthrlineParam = ptVthrline->GetFunction("pol1");
    psi::LogInfo() << "  The y interscept of the Vthrcomp vs. Vcal is " << VthrlineParam->GetParameter(0) << std::endl;
    psi::LogInfo() << "  The slope of the V thrcomp vs. Vcal is " << VthrlineParam->GetParameter(1) << std::endl;

    /* Scan vthrcomp vs CalDel
     	//
      	//  Vthrcomp
      	//  |         .________.
      	//  |         |        |
    	//  |         |        |
      	//  |         |        |
      	//  |         |        |
      	//  |         |        |
      	//  |         |        |
      	//  |        /         |
      	//  |      _/         /
      	//  |   __/         _/
      	//  |__/       ____/
      	//  |_________/
      	//  |
      	//  +-------------------------------- CalDel
    	Here we will be using the Vthrcomp vs CalDel graph to actually
    	extract the pulse height curve.  This corresponds to the two edges
    	tornado.  For this curve, we will find both the left and right edge
    	of the graph, and then we will do a linear transformation of the right
    	edge and overlay it on the left to get extra points of data for the
    	curve fit.  This will use the same routines that have been implemented
    	before and the only difference will be that we are now going to sum
    	them sides instead of finding the width.  The widthavg will be used as
    	the number for the linear transformation to shift by.  This part should
    	output the inital edges, the average edges, the linear transformed edges,
    	and finally the totally calibration pulse height curve using all of the
    	data collected above.
    */

    psi::LogInfo() << "Find Pulse Height Curves from Vthrcomp vs. CalDel" << std::endl;

    for (int binCounterY = minThreshold; binCounterY > 0; binCounterY--) {

        double rightEdge = 256., leftEdge = 0.;
        int lastBinZero = 256, nBinNotZero = 0, firstBinnTrig = 0, nBinnTrig = 0;
        // right edge
        for (int binCounterX = ptVthrVsCalDel->GetNbinsX(); binCounterX > 0; binCounterX--) {
            if (ptVthrVsCalDel->GetBinContent(binCounterX, binCounterY) > 0) {
                nBinNotZero++;
                if (nBinNotZero == 3) {
                    lastBinZero = binCounterX - 3;
                }
            } else {
                nBinNotZero = 0;
            }
            if (ptVthrVsCalDel->GetBinContent(binCounterX, binCounterY) == nTrig) {
                nBinnTrig++;
                if (nBinnTrig == 3) {
                    firstBinnTrig = binCounterX - 3;
                    rightEdge = (lastBinZero + firstBinnTrig) / 2;
                    ptPHdataPoints->SetBinContent(lastBinZero, binCounterY, 1);
                    ptPHdataPoints->SetBinContent(firstBinnTrig, binCounterY, 3);
                    break;
                }
            } else {
                nBinnTrig = 0;
            }
        }

        // left edge is only searched if a right edge was found
        int nBinZero = 0;
        int lastBinNotZero = 256, firstBinnNotTrig = 0, nBinnNotTrig = 0;
        nBinNotZero = 0;
        if (rightEdge < 256.) {
            for (int binCounterX = rightEdge; binCounterX > 0; binCounterX--) {
                if (ptVthrVsCalDel->GetBinContent(binCounterX, binCounterY) < 1) {
                    nBinZero++;
                    if (nBinZero == 3) lastBinNotZero = binCounterX - 3;
                } else {
                    nBinNotZero = 0;
                }
                if (ptVthrVsCalDel->GetBinContent(binCounterX, binCounterY) == 0) {
                    nBinnNotTrig++;
                    if (nBinnNotTrig == 3) {
                        firstBinnNotTrig = binCounterX - 3;
                        leftEdge = (lastBinNotZero + firstBinnNotTrig) / 2.0;
                        ptPHdataPoints->SetBinContent(lastBinNotZero, binCounterY, 2);
                        ptPHdataPoints->SetBinContent(firstBinnNotTrig, binCounterY, 4);
                        pulseHeight = (rightEdge + leftEdge + widthavg) / 2;
                        ptPHintcurve->SetBinContent(pulseHeight, binCounterY);
                        break;
                    }
                } else {
                    nBinnNotTrig = 0;
                }
            }
        }
    }
    for (int binCounterX = 150; binCounterX > 0; binCounterX--) {

        double rightEdge = 256.;
        int lastBinZero = 256, nBinNotZero = 0, firstBinnTrig = 0, nBinnTrig = 0;
        // right edge
        for (int binCounterY = 0; binCounterY < ptVthrVsCalDel->GetNbinsY(); binCounterY++) {
            if (ptVthrVsCalDel->GetBinContent(binCounterX, binCounterY) > 0) {
                nBinNotZero++;
                if (nBinNotZero == 3) {
                    lastBinZero = binCounterY - 3;
                }
            } else {
                nBinNotZero = 0;
            }
            if (ptVthrVsCalDel->GetBinContent(binCounterX, binCounterY) == nTrig) {
                nBinnTrig++;
                if (nBinnTrig == 3) {
                    firstBinnTrig = binCounterY - 3;
                    rightEdge = (lastBinZero + firstBinnTrig) / 2;
                    ptPHdataPoints->SetBinContent(binCounterX, lastBinZero, 5);
                    ptPHdataPoints->SetBinContent(binCounterX, firstBinnTrig, 6);
                    double edgeavg;
                    if(ptPHintcurve->GetBinContent(binCounterX) != 0) {
                        edgeavg = (rightEdge + ptPHintcurve->GetBinContent(binCounterX)) / 2.0;
                    } else {
                        edgeavg = rightEdge;
                    }
                    ptPHintcurve->SetBinContent(binCounterX, edgeavg);
                    break;
                }
            } else {
                nBinnTrig = 0;
            }
        }
    }
    ptPHdataPoints->Write();
    ptPHintcurve->Write();
    double x[256];
    double pHlevel[256];
    double pHuncal;
    double slope = VthrlineParam->GetParameter(1);
    double yinter = VthrlineParam->GetParameter(0);
    int counter = 0;
    for (int i = 0; i < 256; i++) {
        pHuncal = ptPHintcurve->GetBinContent(i);
        if (pHuncal != 0) {
            x[counter] = (25.0 / widthavg * (tZero - i)); //Set units of ns
            pHlevel[counter] = yinter + pHuncal * slope;
            counter++;
        }
    }

    TGraph *PHcurve = new TGraph (counter, x, pHlevel);
    PHcurve->SetTitle(pixelname);
    PHcurve->Write(pixelname);

    //This section will find the rise time for the pixel.  It does this first, by searching for the maximum value
    //of the curve.  It then looks for the point that has 90% of that height, and then outputs the time for this.
    double pHmax = 0;
    double riseTime = 0;
    for (int i = counter; i > 0; i--) {
        if (pHlevel[i] > pHmax) {
            pHmax = pHlevel[i];
        }
    }
    for (int i = counter; i > 0; i--) {
        if (pHlevel[i] > 0.9 * pHmax) {
            riseTime = x[i];
            break;
        }
    }
    psi::LogInfo() << "The rise time is: " << riseTime << std::endl;
    psi::LogInfo() << "==============================" << std::endl;

    delete ptVcalVsCalDel;
    delete ptCalDelwidth;
    delete ptVthrVsCalDel;
    return riseTime;
}

void TestRoc::Initialize()
{
    const ConfigParameters& configParameters = ConfigParameters::Singleton();
    ReadDACParameterFile(configParameters.FullDacParametersFileName().c_str());
    ReadTrimConfiguration(configParameters.FullTrimParametersFileName().c_str());

    ClrCal();
    Mask();
    tbInterface->Flush();
}

int TestRoc::GetChipId()
{
    return chipId;
}

int TestRoc::GetAoutChipPosition()
{
    return aoutChipPosition;
}

void TestRoc::SetTrim(int iCol, int iRow, int trimBit)
{
    GetPixel(iCol, iRow).SetTrim(trimBit);
}

void TestRoc::GetTrimValues(int buffer[])
{
    for (unsigned i = 0; i < psi::ROCNUMCOLS; i++) {
        for (unsigned k = 0; k < psi::ROCNUMROWS; k++) {
            buffer[i * psi::ROCNUMROWS + k] = GetPixel(i, k).GetTrim();
        }
    }
}

// == Parameters =============================================================

void TestRoc::SetDAC(DACParameters::Register reg, int value)
{
    dacParameters->Set(*this, reg, value);
}

int TestRoc::GetDAC(DACParameters::Register dacReg)
{
    return dacParameters->Get(dacReg);
}

// -- Saves the current DAC parameters for later use
boost::shared_ptr<DACParameters> TestRoc::SaveDacParameters()
{
    const boost::shared_ptr<DACParameters> copy(new DACParameters(*dacParameters));
    savedDacParameters = copy;
    return copy;
}

// -- Restores the saved DAC parameters
void TestRoc::RestoreDacParameters(boost::shared_ptr<DACParameters> aDacParameters)
{
    if (aDacParameters)
        dacParameters = boost::shared_ptr<DACParameters>(new DACParameters(*aDacParameters));
    else if(savedDacParameters)
        dacParameters = boost::shared_ptr<DACParameters>(new DACParameters(*savedDacParameters));
    dacParameters->Apply(*this, false);
    Flush();
}

void TestRoc::ReadDACParameterFile(const std::string& filename)
{
    if (filename.find_first_of(".dat") != std::string::npos) {
        dacParameters->Read(filename);
    } else {
        std::ostringstream ss;
        ss << filename << "_C" << chipId << ".dat";
        dacParameters->Read(ss.str());
    }
    dacParameters->Apply(*this, true);
    Flush();
}

void TestRoc::WriteDACParameterFile(const std::string& filename)
{
    if (filename.find_first_of(".dat") != std::string::npos) {
        dacParameters->Write(filename);
    } else {
        std::ostringstream ss;
        ss << filename << "_C" << chipId << ".dat";
        dacParameters->Write(ss.str());
    }
}

// == Roc actions ==================================================================

void TestRoc::ClrCal()
{
    SetChip();
    tbInterface->RocClrCal();
    tbInterface->CDelay(50);
}

void TestRoc::SendCal(int nTrig)
{
    tbInterface->SendCal(nTrig);
}

void TestRoc::SingleCal()
{
    tbInterface->SingleCal();
}

// -- Reads back the result of an earlier sent calibrate signal
int TestRoc::RecvRoCnt()
{
    return tbInterface->RecvRoCnt();
}

// -- Disables all double columns and pixels
void TestRoc::Mask()
{
    for (unsigned i = 0; i < psi::ROCNUMDCOLS; i++) {
        doubleColumns[i]->Mask();
    }
}

int TestRoc::GetRoCnt()
{
    return tbInterface->GetRoCnt();
}

// == Pixel actions ===============================================================

// -- Enables a pixels and sends a calibrate signal
void TestRoc::ArmPixel(int column, int row)
{
    GetDoubleColumnByColumnId(column).ArmPixel(column, row);
}

void TestRoc::DisarmPixel(int column, int row)
{
    GetDoubleColumnByColumnId(column).DisarmPixel(column, row);
}

void TestRoc::SetTrim(int trim)
{
    for (unsigned i = 0; i < psi::ROCNUMCOLS; i++) {
        for (unsigned k = 0; k < psi::ROCNUMROWS; k++) {
            GetPixel(i, k).SetTrim(trim);
        }
    }
}

void TestRoc::EnablePixel(int col, int row)
{
    GetDoubleColumnByColumnId(col).EnablePixel(col, row);
}

void TestRoc::EnableAllPixels()
{
    for (unsigned i = 0; i < psi::ROCNUMCOLS; i++) {
        for (unsigned k = 0; k < psi::ROCNUMROWS; k++) {
            EnablePixel(i, k);
        }
    }
}

void TestRoc::DisablePixel(int col, int row)
{
    GetDoubleColumnByColumnId(col).DisablePixel(col, row);
}

void TestRoc::Cal(int col, int row)
{
    GetDoubleColumnByColumnId(col).Cal(col, row);
}

void TestRoc::Cals(int col, int row)
{
    GetDoubleColumnByColumnId(col).Cals(col, row);
}

// -- sends n calibrate signals and gives back the resulting ADC readout
void TestRoc::SendADCTrigs(int nTrig)
{
    tbInterface->SendADCTrigs(nTrig);
}

bool TestRoc::GetADC(short buffer[], unsigned short buffersize, unsigned short &wordsread, int nTrig, int startBuffer[], int &nReadouts)
{
    return tbInterface->GetADC(buffer, buffersize, wordsread, nTrig, startBuffer, nReadouts);
}

// == Private low level Roc actions ==========================================================

void TestRoc::SetChip()
{
    tbInterface->SetChip(chipId, hubId, portId, aoutChipPosition);
}

void TestRoc::PixTrim(int col, int row, int value)
{
    SetChip();
    tbInterface->RocPixTrim(col, row, value);
    tbInterface->CDelay(50);
}

void TestRoc::PixMask(int col, int row)
{
    SetChip();
    tbInterface->RocPixMask(col, row);
    tbInterface->CDelay(50);
}

void TestRoc::PixCal(int col, int row, int sensorcal)
{
    SetChip();
    tbInterface->RocPixCal(col, row, sensorcal);
    tbInterface->CDelay(50);
}

void TestRoc::ColEnable(int col, int on)
{
    SetChip();
    tbInterface->RocColEnable(col, on);
    tbInterface->CDelay(50);
}

void TestRoc::RocSetDAC(int reg, int value)
{
    SetChip();
    tbInterface->RocSetDAC(reg, value);
}

int TestRoc::ChipThreshold(int start, int step, int thrLevel, int nTrig, DACParameters::Register dacReg, int xtalk,
                           int cals, int data[])
{
    SetChip();
    Flush();
    int trim[psi::ROCNUMROWS * psi::ROCNUMCOLS];
    GetTrimValues(trim);
    return tbInterface->ChipThreshold(start, step, thrLevel, nTrig, dacReg, xtalk, cals, trim, data);
}

int TestRoc::PixelThreshold(int col, int row, int start, int step, int thrLevel, int nTrig, int dacReg, int xtalk, int cals, int trim)
{
    SetChip();
    Flush();
    return tbInterface->PixelThreshold(col, row, start, step, thrLevel, nTrig, dacReg, xtalk, cals, trim);
}

int TestRoc::MaskTest(short nTriggers, short res[])
{
    SetChip();
    Flush();
    return tbInterface->MaskTest(nTriggers, res);
}

int TestRoc::ChipEfficiency(int nTriggers, double res[])
{
    SetChip();
    Flush();
    int trim[psi::ROCNUMROWS * psi::ROCNUMCOLS];
    GetTrimValues(trim);
    return tbInterface->ChipEfficiency(nTriggers, trim, res);
}

int TestRoc::AoutLevelChip(int position, int nTriggers, int res[])
{
    SetChip();
    Flush();
    int trim[psi::ROCNUMROWS * psi::ROCNUMCOLS];
    GetTrimValues(trim);
    return tbInterface->AoutLevelChip(position, nTriggers, trim, res);
}

int TestRoc::AoutLevelPartOfChip(int position, int nTriggers, int res[], bool pxlFlags[])
{
    int trim[psi::ROCNUMROWS * psi::ROCNUMCOLS];
    GetTrimValues(trim);
    return tbInterface->AoutLevelPartOfChip(position, nTriggers, trim, res, pxlFlags);
}

void TestRoc::DacDac(int dac1, int dacRange1, int dac2, int dacRange2, int nTrig, int result[])
{
    SetChip();
    Flush();
    tbInterface->DacDac(dac1, dacRange1, dac2, dacRange2, nTrig, result);
}

void TestRoc::AddressLevelsTest(int result[])
{
    SetChip();
    Flush();
    int position = aoutChipPosition * 3;
    if (tbInterface->TBMIsPresent()) position += 8;
    tbInterface->AddressLevels(position, result);
}

void TestRoc::TrimAboveNoise(short nTrigs, short thr, short mode, short result[])
{
    SetChip();
    Flush();
    tbInterface->TrimAboveNoise(nTrigs, thr, mode, result);
}

// == DoubleColumn actions ===============================================

void TestRoc::EnableDoubleColumn(int col)
{
    GetDoubleColumnByColumnId(col).EnableDoubleColumn();
}

void TestRoc::DisableDoubleColumn(int col)
{
    GetDoubleColumnByColumnId(col).DisableDoubleColumn();
}

// -- sends the commands to the testboard, only meaningful for an analog testboard
void TestRoc::Flush()
{
    tbInterface->Flush();
}

// -- sends a delay command to the testboard, only meaningful for an analog testboard
void TestRoc::CDelay(int clocks)
{
    tbInterface->CDelay(clocks);
}

void TestRoc::WriteTrimConfiguration(const char* filename)
{
    char fname[1000];
    if (strstr(filename, ".dat")) sprintf(fname, "%s", filename);
    else sprintf(fname, "%s_C%i.dat", filename, chipId);

    FILE *file = fopen(fname, "w");
    if (!file) {
        psi::LogInfo() << "[TestRoc] Can not open file '" << fname
                       << "' to write trim configuration." << std::endl;
        return;
    }
    psi::LogInfo() << "[TestRoc] TestRoc" << chipId
                   << ": Writing trim configuration to '" << filename
                   << "'." << std::endl;

    for (unsigned iCol = 0; iCol < psi::ROCNUMCOLS; iCol++) {
        for (unsigned iRow = 0; iRow < psi::ROCNUMROWS; iRow++) {
            fprintf(file, "%2i   Pix %2i %2i\n", GetPixel(iCol, iRow).GetTrim(), iCol, iRow);
        }
    }
    fclose(file);
}

void TestRoc::ReadTrimConfiguration(const char * filename)
{
    if (!filename)
        return;

    /* Add a filename extension if necessary */
    char * fname;
    int fname_len = strlen(filename);

    char * extension = (char *) strstr(filename, ".dat");
    if (extension && (extension - filename == fname_len - 4)) {
        fname = new char [fname_len + 1];
        strcpy(fname, filename);
    } else {
        fname = new char [fname_len + 8 + 1];
        sprintf(fname, "%s_C%i.dat", filename, chipId);
    }

    /* Open the file */
    FILE * file = fopen(fname, "r");
    if (!file) {
        psi::LogInfo() << "Can not open file '" << fname << "' to read trim configuration." << std::endl;
        return;
    }

    psi::LogInfo() << "Reading Trim configuration from '" << fname << "'." << std::endl;

    /* Set default trim values (trimming off = 15) */
    for (unsigned col = 0; col < psi::ROCNUMCOLS; col++) {
        for (unsigned row = 0; row < psi::ROCNUMROWS; row++) {
            GetPixel(col, row).SetTrim(15);
        }
    }

    unsigned col, row;
    /* Read the trim values from the file */
    int trim, retval;
    while ((retval = fscanf(file, "%2d Pix %2d %2d", &trim, &col, &row)) != EOF) {
        if (retval != 3) {
            /* There were less than 3 integers read */
            psi::LogInfo() << "[TestRoc] Error reading from file '" << fname << "': Invalid syntax." << std::endl;
            break;
        }

        if (col < 0 || col >= psi::ROCNUMCOLS || row < 0 || row >= psi::ROCNUMROWS) {
            psi::LogInfo() << "[TestRoc] Skipping trim bits for invalid pixel " << col << ":" << row << std::endl;
            continue;
        }

        if (trim >= 0 && trim <= 15)
            GetPixel(col, row).SetTrim(trim);
        else
            GetPixel(col, row).MaskCompletely();
    }

    /* Clean up */
    fclose(file);
    delete [] fname;
}

bool TestRoc::IsIncluded(boost::shared_ptr<const TestRange> testRange) const
{
    return testRange && testRange->IncludesRoc(chipId);
}

void TestRoc::SendReset()
{
    tbInterface->Single(0x08); //send a reset to set a DAC
}
