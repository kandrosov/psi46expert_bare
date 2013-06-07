/*!
 * \file TemperatureTest.cc
 * \brief Implementation of TemperatureTest class.
 */

#include <TGraph.h>
#include <TF1.h>

#include "TemperatureTest.h"
#include "psi/log.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/TestParameters.h"

TemperatureTest::TemperatureTest(TestRange *aTestRange, TBInterface *aTBInterface)
{
    psi::LogDebug() << "[TemperatureTest] Initialization." << std::endl;

    testRange = aTestRange;
    tbInterface = aTBInterface;
    ReadTestParameters();
}

void TemperatureTest::ReadTestParameters()
{
    nTrig = TestParameters::Singleton().TempNTrig();
}

Double_t Fitfcn( Double_t *x, Double_t *par)
{
    return par[1] * x[0] + par[0];
}

void TemperatureTest::RocAction()
{
    TBAnalogInterface* anaInterface = (TBAnalogInterface*)tbInterface;

    // get black level
    unsigned short count;
    short data[psi::FIFOSIZE], blackLevel;

    anaInterface->ADCRead(data, count, nTrig);
    blackLevel = data[9 + aoutChipPosition * 3];

    // Calibrate
    TGraph *calib = new TGraph();
    calib->SetName(Form("TempCalibration_C%i", chipId));
    for (int rangeTemp = 0; rangeTemp < 8; rangeTemp++) {
        SetDAC(DACParameters::RangeTemp, rangeTemp + 8);
        Flush();
        calib->SetPoint(rangeTemp, rangeTemp, anaInterface->LastDAC(nTrig, aoutChipPosition));
    }
    histograms->Add(calib);
    calib->Write();

    // Measure temperature
    TGraph *meas = new TGraph();
    meas->SetName(Form("TempMeasurement_C%i", chipId));

    for (int rangeTemp = 0; rangeTemp < 8; rangeTemp++) {
        SetDAC(DACParameters::RangeTemp, rangeTemp);
        Flush();
        meas->SetPoint(rangeTemp, rangeTemp, anaInterface->LastDAC(nTrig, aoutChipPosition));
    }

    histograms->Add(meas);
    meas->Write();
}
