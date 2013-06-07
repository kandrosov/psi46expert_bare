/*!
 * \file PHRange.cc
 * \brief Definition of PHRange class.
 */

#include "TH1D.h"

#include "psi/log.h"
#include "BasePixel/TBAnalogInterface.h"
#include "psi46expert/TestRoc.h"
#include "PHRange.h"
#include "BasePixel/TestParameters.h"

PHRange::PHRange(TestRange *aTestRange, TBInterface *aTBInterface)
{
    psi::LogDebug() << "[PHRange] Initialization." << std::endl;

    testRange = aTestRange;
    tbInterface = aTBInterface;
    ReadTestParameters();
}


void PHRange::ReadTestParameters()
{
    const TestParameters& testParameters = TestParameters::Singleton();

    phSafety = testParameters.PHSafety(); //DELTA
    tbmUbLevel = testParameters.TBMUbLevel();  //negative
    debug = false;

    calDelMin = 0;
    vthrCompMin = 0;
    ctrlRegMin = 0;
    vcalMin = 60;
    vtrimMin = 80;

    calDelMax = 0;
    vthrCompMax = 0;
    ctrlRegMax = 4;
    vcalMax = 200;
    vtrimMax = 0;
}


void PHRange::Init()
{
    int aoutData[psi::ROCNUMROWS * psi::ROCNUMCOLS], offset, minPixelPh = 2000, maxPixelPh = -2000;
    if (((TBAnalogInterface*)tbInterface)->TBMPresent()) offset = 16;
    else offset = 9;
    int trim[psi::ROCNUMROWS * psi::ROCNUMCOLS];
    phPosition = offset + aoutChipPosition * 3;
    TH1D *histoMin = new TH1D(Form("PH%i_C%i", vcalMin, chipId), Form("PH%i_C%i", vcalMin, chipId), 400, -2000., 2000.);
    TH1D *histoMax = new TH1D(Form("PH%i_C%i", vcalMax, chipId), Form("PH%i_C%i", vcalMax, chipId), 400, -2000., 2000.);

    // == get settings and pixel for minimum

    roc->GetTrimValues(trim);
    SetDAC(DACParameters::CtrlReg, ctrlRegMin);
    SetDAC(DACParameters::Vtrim, vtrimMin);  //trimming the pixels helps measuring pulse heights at low vcal
    for (unsigned k = 0; k < psi::ROCNUMROWS * psi::ROCNUMCOLS; k++) roc->SetTrim(k / psi::ROCNUMROWS, k % psi::ROCNUMROWS, 0);
    Flush();

    roc->AdjustCalDelVthrComp(5, 5, vcalMin, -0);
    roc->AoutLevelChip(phPosition, 10, aoutData);
    for (unsigned k = 0; k < psi::ROCNUMROWS * psi::ROCNUMCOLS; k++) histoMin->Fill(aoutData[k]);
    for (unsigned k = 0; k < psi::ROCNUMROWS * psi::ROCNUMCOLS; k++) {
        if ((aoutData[k] < minPixelPh) && (TMath::Abs(aoutData[k] - histoMin->GetMean()) < 4 * histoMin->GetRMS())) {
            minPixelPh = aoutData[k];
            minPixel = k;
        }
    }

    roc->AdjustCalDelVthrComp(minPixel / psi::ROCNUMROWS, minPixel % psi::ROCNUMROWS, vcalMin, -0);
    calDelMin = roc->GetDAC(DACParameters::CalDel);
    vthrCompMin = roc->GetDAC(DACParameters::VthrComp);

    for (unsigned k = 0; k < psi::ROCNUMROWS * psi::ROCNUMCOLS; k++) roc->SetTrim(k / psi::ROCNUMROWS, k % psi::ROCNUMROWS, trim[k]);
    SetDAC(DACParameters::Vtrim, 0);

    if (debug)
        psi::LogInfo() << "MinPixel " << (minPixel / psi::ROCNUMROWS) << " " << (minPixel % psi::ROCNUMROWS)
                       << " " << minPixelPh << std::endl;

    // == get settings and pixel for maximum

    SetDAC(DACParameters::CtrlReg, ctrlRegMax);
    Flush();

    roc->AdjustCalDelVthrComp(5, 5, vcalMax, -0);
    roc->AoutLevelChip(phPosition, 10, aoutData);
    for (unsigned k = 0; k < psi::ROCNUMROWS * psi::ROCNUMCOLS; k++) histoMax->Fill(aoutData[k]);
    for (unsigned k = 0; k < psi::ROCNUMROWS * psi::ROCNUMCOLS; k++) {
        if ((aoutData[k] > maxPixelPh) && (TMath::Abs(aoutData[k] - histoMax->GetMean()) < 4 * histoMax->GetRMS())) {
            maxPixelPh = aoutData[k];
            maxPixel = k;
        }
    }

//   roc->AdjustCalDelVthrComp(maxPixel/ROCNUMROWS, maxPixel%ROCNUMROWS, vcalMax, -0);
    calDelMax = roc->GetDAC(DACParameters::CalDel);
    vthrCompMax = roc->GetDAC(DACParameters::VthrComp);

    if (debug)
        psi::LogInfo() << "MaxPixel " << (maxPixel / psi::ROCNUMROWS) << " " << (maxPixel % psi::ROCNUMROWS)
                       << " " << maxPixelPh << std::endl;
}

int PHRange::PHMin()
{
    int ph = PH(ctrlRegMin, vcalMin, calDelMin, vthrCompMin, vtrimMin, minPixel);
    if (ph == 7777) ph = PH(ctrlRegMin, vcalMin, calDelMin, vthrCompMin, vtrimMin, minPixel); //second try
    if (ph == 7777) ph = PH(ctrlRegMin, vcalMin, calDelMin, vthrCompMin, vtrimMin, minPixel); //third try
    if (ph == 7777)
        psi::LogInfo() << "[PHRange] Error: Can not measure Pulse Height."
                       << std::endl;

    return ph;
}


int PHRange::PHMax()
{
    int ph = PH(ctrlRegMax, vcalMax, calDelMax, vthrCompMax, vtrimMax, maxPixel);
    if (ph == 7777)
        psi::LogInfo() << "[PHRange] Error: Can not measure Pulse Height."
                       << std::endl;

    return ph;
}


int PHRange::PH(int ctrlReg, int vcal, int calDel, int vthrComp, int vtrim, int pixel)
{
    unsigned short count;
    short data[psi::FIFOSIZE];
    int trim[psi::ROCNUMROWS * psi::ROCNUMCOLS];
    int ph = 7777;

    roc->GetTrimValues(trim);
    SetDAC(DACParameters::CtrlReg, ctrlReg);
    SetDAC(DACParameters::Vcal, vcal);
    SetDAC(DACParameters::CalDel, calDel);
    SetDAC(DACParameters::VthrComp, vthrComp);
    SetDAC(DACParameters::Vtrim, vtrim);

    TBAnalogInterface *anaInterface = (TBAnalogInterface*)tbInterface;
    anaInterface->DataCtrl(true, false);  //somehow needed to clear fifo buffer after AdjustCalDelVthrComp

    roc->SetTrim(pixel / psi::ROCNUMROWS, pixel % psi::ROCNUMROWS, 0);
    roc->ArmPixel(pixel / psi::ROCNUMROWS, pixel % psi::ROCNUMROWS);

    anaInterface->ADCRead(data, count, 10);
    if (count > anaInterface->GetEmptyReadoutLengthADC()) ph = data[phPosition];

    roc->DisarmPixel(pixel / psi::ROCNUMROWS, pixel % psi::ROCNUMROWS);
    roc->SetTrim(pixel / psi::ROCNUMROWS, pixel % psi::ROCNUMROWS, trim[pixel]);

    return ph;
}


void PHRange::RocAction()
{
    psi::LogDebug() << "[PHRange] Roc #" << chipId << '.' << std::endl;

    psi::LogInfo().PrintTimestamp();

    SaveDacParameters();
    Init();

    int vibiasPh = 90, offsetOp = 40, stepSize;
    int goalRange = 2 * TMath::Abs(tbmUbLevel) - phSafety;
    int loopnumber = 0;
    int diffRange, diffPos, diffRangeOld, diffPosOld;

    // loop to achieve best range & best position

    if (debug)
        psi::LogInfo() << "goalRange " << goalRange << std::endl;

    SetDAC(DACParameters::VIbias_PH, vibiasPh);
    SetDAC(DACParameters::VoffsetOp, offsetOp);
    Flush();

    do {
        if (debug)
            psi::LogInfo() << "loop: " << loopnumber << std::endl;
        if (loopnumber == 0) stepSize = 5;
        else stepSize = 1;

        diffRange = PHMax() - PHMin() - goalRange;
        if (diffRange > 0) {
            do {
                vibiasPh -= stepSize;
                SetDAC(DACParameters::VIbias_PH, vibiasPh);
                diffRangeOld = diffRange;
                diffRange = PHMax() - PHMin() - goalRange;
            } while (diffRange > 0 && vibiasPh > stepSize);
            if (TMath::Abs(diffRangeOld) < TMath::Abs(diffRange)) {
                vibiasPh += stepSize;
                SetDAC(DACParameters::VIbias_PH, vibiasPh);
            }
        } else {
            do {
                vibiasPh += stepSize;
                SetDAC(DACParameters::VIbias_PH, vibiasPh);
                diffRangeOld = diffRange;
                diffRange = PHMax() - PHMin() - goalRange;
            } while (diffRange < 0 && vibiasPh < 230 - stepSize);
            if (TMath::Abs(diffRangeOld) < TMath::Abs(diffRange)) {
                vibiasPh -= stepSize;
                SetDAC(DACParameters::VIbias_PH, vibiasPh);
            }
        }

        diffPos = TMath::Abs(tbmUbLevel) - PHMax();
        if (diffPos > 0) {
            do {
                offsetOp += stepSize;
                SetDAC(DACParameters::VoffsetOp, offsetOp);
                diffPosOld = diffPos;
                diffPos = TMath::Abs(tbmUbLevel) - PHMax();
            } while (diffPos > 0 && offsetOp < 255 - stepSize);
            if (TMath::Abs(diffPosOld) < TMath::Abs(diffPos)) {
                offsetOp -= stepSize;
                SetDAC(DACParameters::VoffsetOp, offsetOp);
            }
        } else {
            do {
                offsetOp -= stepSize;
                SetDAC(DACParameters::VoffsetOp, offsetOp);
                diffPosOld = diffPos;
                diffPos = TMath::Abs(tbmUbLevel) - PHMax();
            } while (diffPos < 0 && offsetOp > stepSize);
            if (TMath::Abs(diffPosOld) < TMath::Abs(diffPos)) {
                offsetOp += stepSize;
                SetDAC(DACParameters::VoffsetOp, offsetOp);
            }
        }

        diffRange = PHMax() - PHMin() - goalRange;
        diffPos = TMath::Abs(tbmUbLevel) - PHMax();
        if (debug)
            psi::LogInfo() << "diffRange " << diffRange << " diffPos " << diffPos << std::endl;
        loopnumber++;
    } while ((TMath::Abs(diffRange) > 5 || TMath::Abs(diffPos) > 5) && loopnumber < 3);


    RestoreDacParameters();

    SetDAC(DACParameters::VIbias_PH, vibiasPh);
    SetDAC(DACParameters::VoffsetOp, offsetOp);

    psi::LogDebug() << "[PHRange] VIbias_PH " << vibiasPh << " VoffsetOp "
                    << offsetOp << std::endl;

    Flush();

    ValidationPlot();
}


void PHRange::ValidationPlot()  //fast (minimal) version
{
    psi::LogInfo() << "Validation plot\n";
    TH2D *valPlot = new TH2D(Form("ValPlot_C%i", chipId), Form("ValidationPlot_C%i", chipId), 9, 0, 9, 4000, -2000, 2000);
    unsigned short count;
    short data[psi::FIFOSIZE];

    SaveDacParameters();

    //address levels from pixel 10, 13
    int colNumber = 10, rowNumber = 13;
    roc->ArmPixel(colNumber, rowNumber);
    SetDAC(DACParameters::RangeTemp, 0); //maximal last dac
    ((TBAnalogInterface*)tbInterface)->ADCRead(data, count, 5);
    if (count == ((TBAnalogInterface*)tbInterface)->GetEmptyReadoutLengthADC() + 6) for (int i = 0; i < 8; i++) valPlot->Fill(i, data[8 + aoutChipPosition * 3 + i]);
    roc->DisarmPixel(colNumber, rowNumber);

    //pulse height minimum and maximum
    valPlot->Fill(8, PHMin());
    valPlot->Fill(8, PHMax());

    RestoreDacParameters();
    histograms->Add(valPlot);
}

