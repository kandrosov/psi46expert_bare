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

PHRange::PHRange(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface)
    : Test("PHRange", testRange), tbInterface(aTBInterface)
{
    const TestParameters& testParameters = TestParameters::Singleton();

    phSafety = testParameters.PHSafety(); //DELTA
    tbmUbLevel = testParameters.TBMUbLevel();  //negative

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

void PHRange::Init(TestRoc& roc)
{
    int aoutData[psi::ROCNUMROWS * psi::ROCNUMCOLS], offset, minPixelPh = 2000, maxPixelPh = -2000;
    if (tbInterface->TBMPresent()) offset = 16;
    else offset = 9;
    int trim[psi::ROCNUMROWS * psi::ROCNUMCOLS];
    phPosition = offset + roc.GetAoutChipPosition() * 3;
    std::ostringstream histoMinName;
    histoMinName << "PH" << vcalMin << "_C" <<  roc.GetChipId();
    TH1D *histoMin = new TH1D(histoMinName.str().c_str(), histoMinName.str().c_str(), 400, -2000., 2000.);
    std::ostringstream histoMaxName;
    histoMinName << "PH" << vcalMax << "_C" <<  roc.GetChipId();
    TH1D *histoMax = new TH1D(histoMaxName.str().c_str(), histoMaxName.str().c_str(), 400, -2000., 2000.);

    // == get settings and pixel for minimum

    roc.GetTrimValues(trim);
    roc.SetDAC(DACParameters::CtrlReg, ctrlRegMin);
    roc.SetDAC(DACParameters::Vtrim, vtrimMin);  //trimming the pixels helps measuring pulse heights at low vcal
    for (unsigned k = 0; k < psi::ROCNUMROWS * psi::ROCNUMCOLS; k++)
        roc.SetTrim(k / psi::ROCNUMROWS, k % psi::ROCNUMROWS, 0);
    roc.Flush();

    roc.AdjustCalDelVthrComp(5, 5, vcalMin, -0);
    roc.AoutLevelChip(phPosition, 10, aoutData);
    for (unsigned k = 0; k < psi::ROCNUMROWS * psi::ROCNUMCOLS; k++) histoMin->Fill(aoutData[k]);
    for (unsigned k = 0; k < psi::ROCNUMROWS * psi::ROCNUMCOLS; k++) {
        if ((aoutData[k] < minPixelPh) && (TMath::Abs(aoutData[k] - histoMin->GetMean()) < 4 * histoMin->GetRMS())) {
            minPixelPh = aoutData[k];
            minPixel = k;
        }
    }

    roc.AdjustCalDelVthrComp(minPixel / psi::ROCNUMROWS, minPixel % psi::ROCNUMROWS, vcalMin, -0);
    calDelMin = roc.GetDAC(DACParameters::CalDel);
    vthrCompMin = roc.GetDAC(DACParameters::VthrComp);

    for (unsigned k = 0; k < psi::ROCNUMROWS * psi::ROCNUMCOLS; k++)
        roc.SetTrim(k / psi::ROCNUMROWS, k % psi::ROCNUMROWS, trim[k]);
    roc.SetDAC(DACParameters::Vtrim, 0);

    if (debug)
        psi::LogInfo() << "MinPixel " << (minPixel / psi::ROCNUMROWS) << " " << (minPixel % psi::ROCNUMROWS)
                       << " " << minPixelPh << std::endl;

    // == get settings and pixel for maximum

    roc.SetDAC(DACParameters::CtrlReg, ctrlRegMax);
    roc.Flush();

    roc.AdjustCalDelVthrComp(5, 5, vcalMax, -0);
    roc.AoutLevelChip(phPosition, 10, aoutData);
    for (unsigned k = 0; k < psi::ROCNUMROWS * psi::ROCNUMCOLS; k++) histoMax->Fill(aoutData[k]);
    for (unsigned k = 0; k < psi::ROCNUMROWS * psi::ROCNUMCOLS; k++) {
        if ((aoutData[k] > maxPixelPh) && (TMath::Abs(aoutData[k] - histoMax->GetMean()) < 4 * histoMax->GetRMS())) {
            maxPixelPh = aoutData[k];
            maxPixel = k;
        }
    }

//   roc->AdjustCalDelVthrComp(maxPixel/ROCNUMROWS, maxPixel%ROCNUMROWS, vcalMax, -0);
    calDelMax = roc.GetDAC(DACParameters::CalDel);
    vthrCompMax = roc.GetDAC(DACParameters::VthrComp);

    if (debug)
        psi::LogInfo() << "MaxPixel " << (maxPixel / psi::ROCNUMROWS) << " " << (maxPixel % psi::ROCNUMROWS)
                       << " " << maxPixelPh << std::endl;
}

int PHRange::PHMin(TestRoc& roc)
{
    int ph = PH(roc, ctrlRegMin, vcalMin, calDelMin, vthrCompMin, vtrimMin, minPixel);
    if (ph == 7777) ph = PH(roc, ctrlRegMin, vcalMin, calDelMin, vthrCompMin, vtrimMin, minPixel); //second try
    if (ph == 7777) ph = PH(roc, ctrlRegMin, vcalMin, calDelMin, vthrCompMin, vtrimMin, minPixel); //third try
    if (ph == 7777)
        psi::LogInfo() << "[PHRange] Error: Can not measure Pulse Height."
                       << std::endl;

    return ph;
}


int PHRange::PHMax(TestRoc& roc)
{
    int ph = PH(roc, ctrlRegMax, vcalMax, calDelMax, vthrCompMax, vtrimMax, maxPixel);
    if (ph == 7777)
        psi::LogInfo() << "[PHRange] Error: Can not measure Pulse Height."
                       << std::endl;

    return ph;
}


int PHRange::PH(TestRoc& roc, int ctrlReg, int vcal, int calDel, int vthrComp, int vtrim, int pixel)
{
    unsigned short count;
    short data[psi::FIFOSIZE];
    int trim[psi::ROCNUMROWS * psi::ROCNUMCOLS];
    int ph = 7777;

    roc.GetTrimValues(trim);
    roc.SetDAC(DACParameters::CtrlReg, ctrlReg);
    roc.SetDAC(DACParameters::Vcal, vcal);
    roc.SetDAC(DACParameters::CalDel, calDel);
    roc.SetDAC(DACParameters::VthrComp, vthrComp);
    roc.SetDAC(DACParameters::Vtrim, vtrim);

    tbInterface->DataCtrl(true, false);  //somehow needed to clear fifo buffer after AdjustCalDelVthrComp

    roc.SetTrim(pixel / psi::ROCNUMROWS, pixel % psi::ROCNUMROWS, 0);
    roc.ArmPixel(pixel / psi::ROCNUMROWS, pixel % psi::ROCNUMROWS);

    tbInterface->ADCRead(data, count, 10);
    if (count > tbInterface->GetEmptyReadoutLengthADC()) ph = data[phPosition];

    roc.DisarmPixel(pixel / psi::ROCNUMROWS, pixel % psi::ROCNUMROWS);
    roc.SetTrim(pixel / psi::ROCNUMROWS, pixel % psi::ROCNUMROWS, trim[pixel]);

    return ph;
}

void PHRange::RocAction(TestRoc& roc)
{
    psi::LogDebug() << "[PHRange] Roc #" << roc.GetChipId() << '.' << std::endl;

    psi::LogInfo().PrintTimestamp();

    SaveDacParameters(roc);
    Init(roc);

    int vibiasPh = 90, offsetOp = 40, stepSize;
    int goalRange = 2 * TMath::Abs(tbmUbLevel) - phSafety;
    int loopnumber = 0;
    int diffRange, diffPos, diffRangeOld, diffPosOld;

    // loop to achieve best range & best position

    if (debug)
        psi::LogInfo() << "goalRange " << goalRange << std::endl;

    roc.SetDAC(DACParameters::VIbias_PH, vibiasPh);
    roc.SetDAC(DACParameters::VoffsetOp, offsetOp);
    roc.Flush();

    do {
        if (debug)
            psi::LogInfo() << "loop: " << loopnumber << std::endl;
        if (loopnumber == 0) stepSize = 5;
        else stepSize = 1;

        diffRange = PHMax(roc) - PHMin(roc) - goalRange;
        if (diffRange > 0) {
            do {
                vibiasPh -= stepSize;
                roc.SetDAC(DACParameters::VIbias_PH, vibiasPh);
                diffRangeOld = diffRange;
                diffRange = PHMax(roc) - PHMin(roc) - goalRange;
            } while (diffRange > 0 && vibiasPh > stepSize);
            if (TMath::Abs(diffRangeOld) < TMath::Abs(diffRange)) {
                vibiasPh += stepSize;
                roc.SetDAC(DACParameters::VIbias_PH, vibiasPh);
            }
        } else {
            do {
                vibiasPh += stepSize;
                roc.SetDAC(DACParameters::VIbias_PH, vibiasPh);
                diffRangeOld = diffRange;
                diffRange = PHMax(roc) - PHMin(roc) - goalRange;
            } while (diffRange < 0 && vibiasPh < 230 - stepSize);
            if (TMath::Abs(diffRangeOld) < TMath::Abs(diffRange)) {
                vibiasPh -= stepSize;
                roc.SetDAC(DACParameters::VIbias_PH, vibiasPh);
            }
        }

        diffPos = TMath::Abs(tbmUbLevel) - PHMax(roc);
        if (diffPos > 0) {
            do {
                offsetOp += stepSize;
                roc.SetDAC(DACParameters::VoffsetOp, offsetOp);
                diffPosOld = diffPos;
                diffPos = TMath::Abs(tbmUbLevel) - PHMax(roc);
            } while (diffPos > 0 && offsetOp < 255 - stepSize);
            if (TMath::Abs(diffPosOld) < TMath::Abs(diffPos)) {
                offsetOp -= stepSize;
                roc.SetDAC(DACParameters::VoffsetOp, offsetOp);
            }
        } else {
            do {
                offsetOp -= stepSize;
                roc.SetDAC(DACParameters::VoffsetOp, offsetOp);
                diffPosOld = diffPos;
                diffPos = TMath::Abs(tbmUbLevel) - PHMax(roc);
            } while (diffPos < 0 && offsetOp > stepSize);
            if (TMath::Abs(diffPosOld) < TMath::Abs(diffPos)) {
                offsetOp += stepSize;
                roc.SetDAC(DACParameters::VoffsetOp, offsetOp);
            }
        }

        diffRange = PHMax(roc) - PHMin(roc) - goalRange;
        diffPos = TMath::Abs(tbmUbLevel) - PHMax(roc);
        if (debug)
            psi::LogInfo() << "diffRange " << diffRange << " diffPos " << diffPos << std::endl;
        loopnumber++;
    } while ((TMath::Abs(diffRange) > 5 || TMath::Abs(diffPos) > 5) && loopnumber < 3);


    RestoreDacParameters(roc);

    roc.SetDAC(DACParameters::VIbias_PH, vibiasPh);
    roc.SetDAC(DACParameters::VoffsetOp, offsetOp);

    psi::LogDebug() << "[PHRange] VIbias_PH " << vibiasPh << " VoffsetOp "
                    << offsetOp << std::endl;

    roc.Flush();

    ValidationPlot(roc);
}

void PHRange::ValidationPlot(TestRoc& roc)  //fast (minimal) version
{
    psi::LogInfo() << "Validation plot\n";
    std::ostringstream valPlotName;
    valPlotName << "ValPlot_C" << roc.GetChipId();
    TH2D *valPlot = new TH2D(valPlotName.str().c_str(), valPlotName.str().c_str(), 9, 0, 9, 4000, -2000, 2000);
    unsigned short count;
    short data[psi::FIFOSIZE];

    SaveDacParameters(roc);

    //address levels from pixel 10, 13
    int colNumber = 10, rowNumber = 13;
    roc.ArmPixel(colNumber, rowNumber);
    roc.SetDAC(DACParameters::RangeTemp, 0); //maximal last dac
    tbInterface->ADCRead(data, count, 5);
    if (count == tbInterface->GetEmptyReadoutLengthADC() + 6) {
        for (int i = 0; i < 8; i++)
            valPlot->Fill(i, data[8 + roc.GetAoutChipPosition() * 3 + i]);
    }
    roc.DisarmPixel(colNumber, rowNumber);

    //pulse height minimum and maximum
    valPlot->Fill(8, PHMin(roc));
    valPlot->Fill(8, PHMax(roc));

    RestoreDacParameters(roc);
    histograms->Add(valPlot);
}
