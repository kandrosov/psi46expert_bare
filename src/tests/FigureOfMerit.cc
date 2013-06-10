/*!
 * \file FigureOfMerit.cc
 * \brief Implementation of FigureOfMerit class.
 */

#include "FigureOfMerit.h"
#include "psi46expert/TestRoc.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/constants.h"
#include "TCanvas.h"
#include <TF1.h>
#include "PhDacScan.h"
#include "BasePixel/TestParameters.h"

FigureOfMerit::FigureOfMerit(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface,
                             DACParameters::Register dac1, DACParameters::Register dac2, int crit)
    : Test("FigureOfMerit", testRange), tbInterface(aTBInterface), firstDac(dac1), secondDac(dac2), criterion(crit)
{
    const TestParameters& testParameters = TestParameters::Singleton();
    dac1Start = testParameters.PHdac1Start();
    dac1Stop = testParameters.PHdac1Stop();
    dac1Step = testParameters.PHdac1Step();
    dac2Start = testParameters.PHdac2Start();
    dac2Stop = testParameters.PHdac2Stop();
    dac2Step = testParameters.PHdac2Step();
    testVcal = testParameters.PHtestVcal();
    debug = true;
}

void FigureOfMerit::RocAction(TestRoc& roc)
{
    SaveDacParameters(roc);
    roc.SetDAC(DACParameters::CtrlReg, 4);
    Test::RocAction(roc);
    RestoreDacParameters(roc);
}

void FigureOfMerit::PixelAction(TestPixel& pixel)
{
    pixel.ArmPixel();
    tbInterface->Flush();
    DoDacDacScan(pixel);
    pixel.DisarmPixel();
}

void FigureOfMerit::DoDacDacScan(TestPixel& pixel)
{
    if (debug) psi::LogInfo() << " ************************* SCAN IS RUNNING **************************" << std::endl;

    int dacValue1Size = (dac1Stop - dac1Start) / dac1Step;
    int dacValue2Size = (dac2Stop - dac2Start) / dac2Step;
    double quality = 0;
    int optimalDac2, optimalDac1;

    char testNameUnit[100], testName[100];

    if (criterion == 0) {
        strcpy(testNameUnit, "timewalk [ns]");
        strcpy(testName, "Timewalk");
    }
    if (criterion == 1) {
        strcpy(testNameUnit, "Linearity (high range) [DAC units]");
        strcpy(testName, "high_linear_range");
    }
    if (criterion == 2) {
        strcpy(testNameUnit, "pulse height [ADC units]");
        strcpy(testName, "Pulse_height");
    }
    if (criterion == 3) {
        strcpy(testNameUnit, "Linearity (low range) [ADC units]");
        strcpy(testName, "low_linear_range");
    }
    if (criterion == 4) {
        strcpy(testNameUnit, "Threshold (low range) [DAC units]");
        strcpy(testName, "threshold");
    }

    const std::string& firstDacName = DACParameters::GetRegisterName(firstDac);
    const std::string& secondDacName = DACParameters::GetRegisterName(secondDac);

    std::ostringstream histo2Name;
    histo2Name << testName << "_of_c" << pixel.GetColumn() << "r" << pixel.GetRow()
               << "_C" << pixel.GetRoc().GetChipId();
    TH2D *histo2 = new TH2D(histo2Name.str().c_str(), histo2Name.str().c_str(), dacValue1Size + 1 ,
                            dac1Start, dac1Stop + dac1Step, dacValue2Size + 1, dac2Start, dac2Stop + dac2Step);
    histo2->GetXaxis()->SetTitle(Form("%s [DAC units]", firstDacName.c_str()));
    histo2->GetYaxis()->SetTitle(Form("%s [DAC units]", secondDacName.c_str()));
    histo2->GetZaxis()->SetTitle(Form("%s", testNameUnit));

    std::ostringstream minPhHistoName;
    minPhHistoName << "Min_PH_c" << pixel.GetColumn() << "r" << pixel.GetRow() << "_C" << pixel.GetRoc().GetChipId();
    TH2D *minPhHisto = new TH2D(minPhHistoName.str().c_str(), minPhHistoName.str().c_str(), dacValue1Size + 1,
                                dac1Start, dac1Stop + dac1Step, dacValue2Size + 1, dac2Start, dac2Stop + dac2Step);
    minPhHisto->GetXaxis()->SetTitle(Form("%s [DAC units]", firstDacName.c_str()));
    minPhHisto->GetYaxis()->SetTitle(Form("%s [DAC units]", secondDacName.c_str()));
    minPhHisto->GetZaxis()->SetTitle(Form("%s", testNameUnit));

    nor = new TH1D("numberOfReadouts", "numberOfReadouts", 250, 0, 250);

    if (criterion == 0) bestQuality = 100;
    if (criterion == 1) bestQuality = -99;
    if (criterion == 2) bestQuality = -99;
    if (criterion == 3) bestQuality = -99;
    if (criterion == 4) bestQuality = 100;

    for (int i = 0; i <= dacValue1Size; i++) {
        dacValue1 = dac1Start + i * dac1Step;
        pixel.GetRoc().SetDAC(firstDac, dacValue1);

        for (int k = 0; k <= dacValue2Size; k++) {
            dacValue2 = dac2Start + k * dac2Step;
            pixel.GetRoc().SetDAC(secondDac, dacValue2);

            if(debug) psi::LogInfo() << firstDacName << " = " << dacValue1 << "   "
                                     << secondDacName << " = " << dacValue2 << std::endl;

            if (criterion == 0) quality = Timewalk(pixel.GetRoc(), i, k);
            if (criterion == 1) quality = LinearRange(pixel.GetRoc(), i, k);
            if (criterion == 2) quality = PulseHeight(pixel.GetRoc(), i, k);
            if (criterion == 3) quality = LowLinearRange(pixel.GetRoc(), i, k);
            if (criterion == 4) quality = Threshold(pixel.GetRoc(), i, k);

            if (debug) psi::LogInfo() << "Quality = " << quality << std::endl;

            histo2->SetBinContent(i + 1, k + 1, quality);
            minPhHisto->SetBinContent(i + 1, k + 1, phDacScan.GetMinPh());
        }
    }

    histograms->Add(histo2);
    histograms->Add(nor);
    if (criterion == 1) histograms->Add(minPhHisto);

    optimalDac2 = dac2Start + index2 * dac2Step;
    optimalDac1 = dac1Start + index1 * dac1Step;
    psi::LogInfo() << "bestQuality = " << bestQuality << " @ " << firstDacName << " = " << optimalDac1
                   << " and " << secondDacName << " = " << optimalDac2 << std::endl;
    if (debug) psi::LogInfo() << "pixel column = " << pixel.GetColumn() << " pixel row = " << pixel.GetRow() << "\n";
}


double FigureOfMerit::Timewalk(TestRoc& roc, int i, int k)
{
    const std::string& firstDacName = DACParameters::GetRegisterName(firstDac);
    const std::string& secondDacName = DACParameters::GetRegisterName(secondDac);

    short resultA[256], resultB[256];

    std::ostringstream histoNamePrefix;
    histoNamePrefix << "PHVhldDel_" << firstDacName << dacValue1 << "_" << secondDacName << dacValue2
                    << "_C" << roc.GetChipId();
    const std::string histoAName = histoNamePrefix.str() + "A";
    TH1D *histoA = new TH1D(histoAName.c_str(), histoAName.c_str(), 256, 0, 256);
    const std::string histoBName = histoNamePrefix.str() + "B";
    TH1D *histoB = new TH1D(histoBName.c_str(), histoBName.c_str(), 256, 0, 256);

    roc.SetDAC(DACParameters::CtrlReg, 0);
    roc.SetDAC(DACParameters::Vcal, 80);
    tbInterface->PHDac(26, 256, phDacScan.GetNTrig(), 16 + roc.GetAoutChipPosition() * 3, resultA);
    roc.SetDAC(DACParameters::Vcal, 250);
    tbInterface->PHDac(26, 256, phDacScan.GetNTrig(), 16 + roc.GetAoutChipPosition() * 3, resultB);

    int numberOfReadoutsA = 0;
    int numberOfReadoutsB = 0;

    for (int dac = 0; dac < 256; dac++) {
        histoA->SetBinContent(dac + 1, resultA[dac]);
        histoB->SetBinContent(dac + 1, resultB[dac]);
        if (resultA[dac] != 7777) numberOfReadoutsA++;
        if (resultB[dac] != 7777) numberOfReadoutsB++;
    }

    nor->Fill(numberOfReadoutsA);
    nor->Fill(numberOfReadoutsB);

    if(debug)
        psi::LogInfo() << "number of readoutsA = " << numberOfReadoutsA
                       << " Number of readouts B = " << numberOfReadoutsB << std::endl;

    if (numberOfReadoutsA < 30) return 0;
    if (numberOfReadoutsB < 20) return 0;

    double firstCalDelA = 0.45 * (256 - FindFirstValue(resultA)) + 30;  // converts CalDel from DAC units to ns
    double firstCalDelB = 0.45 * (256 - FindFirstValue(resultB)) + 30;  // converts CalDel from DAC units to ns
    double timewalk = firstCalDelA - firstCalDelB;

    if (debug)
        psi::LogInfo() << "first CalDel [ns] = " << FindFirstValue(resultA)
                       << " second CalDel [ns] = " << FindFirstValue(resultB) << std::endl;
    if (debug)
        psi::LogInfo() << "first CalDel [DAC units] = " << firstCalDelA
                       << " second CalDel [DAC units] = " << firstCalDelB << std::endl;

    histograms->Add(histoA);
    histograms->Add(histoB);

    if (timewalk < bestQuality) {
        bestQuality = static_cast<int>( timewalk);
        index1 = i;
        index2 = k;
    }

    return timewalk;
}

int FigureOfMerit::LinearRange(TestRoc& roc, int i, int k)
{
    const std::string& firstDacName = DACParameters::GetRegisterName(firstDac);
    const std::string& secondDacName = DACParameters::GetRegisterName(secondDac);

    short result[256];
    std::ostringstream histoName;
    histoName << "PHVcal_" << firstDacName << dacValue1 << "_" << secondDacName << dacValue2 << "_C"
              << roc.GetChipId();
    TH1D *histo = new TH1D(histoName.str().c_str(), histoName.str().c_str(), 256, 0, 256);

    tbInterface->PHDac(25, 256, phDacScan.GetNTrig(), 16 + roc.GetAoutChipPosition() * 3, result);

    for (int dac = 0; dac < 256; dac++) histo->SetBinContent(dac + 1, result[dac]);
    histo->SetMaximum(result[255] + 100);
    histograms->Add(histo);

    int linearRange = static_cast<int>( phDacScan.FindLinearRange(histo) );

    if (linearRange > bestQuality) {
        bestQuality = linearRange;
        index1 = i;
        index2 = k;
    }

    return linearRange;
}

int FigureOfMerit::PulseHeight(TestRoc& roc, int i, int k)
{
    const std::string& firstDacName = DACParameters::GetRegisterName(firstDac);
    const std::string& secondDacName = DACParameters::GetRegisterName(secondDac);

    short result[256];
    std::ostringstream histoName;
    histoName << "PHVcal_" << firstDacName << dacValue1 << "_" << secondDacName << dacValue2 << "_C"
              << roc.GetChipId();
    TH1D *histo = new TH1D(histoName.str().c_str(), histoName.str().c_str(), 256, 0, 256);

    tbInterface->PHDac(25, 256, phDacScan.GetNTrig(), 16 + roc.GetAoutChipPosition() * 3, result);

    for (int dac = 0; dac < 256; dac++) histo->SetBinContent(dac + 1, result[dac]);
    histo->SetMaximum(result[255] + 100);
    histograms->Add(histo);

    int minPh = phDacScan.FitStartPoint(histo);
    double pulseHeight = result[testVcal - 1] - result[minPh];

    if (pulseHeight > bestQuality) {
        bestQuality = static_cast<int>( pulseHeight);
        index1 = i;
        index2 = k;
    }

    return static_cast<int>( pulseHeight);
}

int FigureOfMerit::FindFirstValue(short *result)
{
    int firstCalDel = -1;
    for (int n = 255; n > 0; n--) {
        if (result[n] != 7777) {
            firstCalDel = n;
            break;
        }
    }
    return firstCalDel;
}

double FigureOfMerit::LowLinearRange(TestRoc& roc, int i, int k)
{
    const std::string& firstDacName = DACParameters::GetRegisterName(firstDac);
    const std::string& secondDacName = DACParameters::GetRegisterName(secondDac);

    short result[256], resultHR[256];
    std::ostringstream histoName;
    histoName << "PHVcal_" << firstDacName << dacValue1 << "_" << secondDacName << dacValue2 << "_C"
              << roc.GetChipId();
    TH1D *histo = new TH1D(histoName.str().c_str(), histoName.str().c_str(), 256, 0, 256);
    const std::string fullRangeHistName = histoName.str() + "FullRange";
    TH1D *fullRangeHist = new TH1D(fullRangeHistName.c_str(), fullRangeHistName.c_str(), 1792, 0, 1792);

    roc.SetDAC(DACParameters::CtrlReg, 4);
    tbInterface->PHDac(25, 256, phDacScan.GetNTrig(), 16 + roc.GetAoutChipPosition() * 3, resultHR);
    roc.SetDAC(DACParameters::CtrlReg, 0);
    tbInterface->PHDac(25, 256, phDacScan.GetNTrig(), 16 + roc.GetAoutChipPosition() * 3, result);

    int value = 0;
    for (int vcal = 0; vcal < 256; vcal++) {
        for (int n = 0; n < 7; n++) {
            fullRangeHist->SetBinContent(value + 1, resultHR[vcal]);
            value++;
        }
    }
    for (int vcal = 0; vcal < 256; vcal++) fullRangeHist->SetBinContent(vcal + 1, result[vcal]);
    fullRangeHist->GetYaxis()->SetRangeUser(fullRangeHist->GetBinContent(100) - 200, fullRangeHist->GetBinContent(1790) + 100);

    for (int dac = 0; dac < 256; dac++) histo->SetBinContent(dac + 1, result[dac]);
    histo->SetMaximum(result[255] + 100);
    histograms->Add(histo);
    histograms->Add(fullRangeHist);

    double aoverb = phDacScan.QualityLowRange(histo);

    if (aoverb > bestQuality) {
        bestQuality = static_cast<int>( aoverb);
        index1 = i;
        index2 = k;
    }

    roc.SetDAC(DACParameters::CtrlReg, 4);

    return TMath::Abs(aoverb);
}

int FigureOfMerit::Threshold(TestRoc& roc, int i, int k)
{
    const std::string& firstDacName = DACParameters::GetRegisterName(firstDac);
    const std::string& secondDacName = DACParameters::GetRegisterName(secondDac);

    short result[256];

    std::ostringstream histoName;
    histoName << "PHVcal_" << firstDacName << dacValue1 << "_" << secondDacName << dacValue2 << "_C"
              << roc.GetChipId();
    TH1D *histo = new TH1D(histoName.str().c_str(), histoName.str().c_str(), 256, 0, 256);

    roc.SetDAC(DACParameters::CtrlReg, 0);
    tbInterface->PHDac(25, 256, phDacScan.GetNTrig(), 16 + roc.GetAoutChipPosition() * 3, result);

    for (int dac = 0; dac < 256; dac++) histo->SetBinContent(dac + 1, result[dac]);
    histo->SetMaximum(result[255] + 100);
    histograms->Add(histo);

    int threshold = phDacScan.FitStartPoint(histo);

    if (threshold < bestQuality) {
        bestQuality = threshold;
        index1 = i;
        index2 = k;
    }

    return threshold;
}
