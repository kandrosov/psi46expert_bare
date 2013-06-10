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
#include "psi46expert/TestRoc.h"

TemperatureTest::TemperatureTest(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface)
    : Test("TemperatureTest", testRange), tbInterface(aTBInterface)
{
    nTrig = TestParameters::Singleton().TempNTrig();
}

void TemperatureTest::RocAction(TestRoc& roc)
{
    // get black level
    unsigned short count;
    short data[psi::FIFOSIZE], blackLevel;

    tbInterface->ADCRead(data, count, nTrig);
    blackLevel = data[9 + roc.GetAoutChipPosition() * 3];

    // Calibrate
    TGraph *calib = new TGraph();
    calib->SetName(Form("TempCalibration_C%i", roc.GetChipId()));
    for (int rangeTemp = 0; rangeTemp < 8; rangeTemp++) {
        roc.SetDAC(DACParameters::RangeTemp, rangeTemp + 8);
        roc.Flush();
        calib->SetPoint(rangeTemp, rangeTemp, tbInterface->LastDAC(nTrig, roc.GetAoutChipPosition()));
    }
    histograms->Add(calib);

    // Measure temperature
    TGraph *meas = new TGraph();
    meas->SetName(Form("TempMeasurement_C%i", roc.GetChipId()));
    for (int rangeTemp = 0; rangeTemp < 8; rangeTemp++) {
        roc.SetDAC(DACParameters::RangeTemp, rangeTemp);
        roc.Flush();
        meas->SetPoint(rangeTemp, rangeTemp, tbInterface->LastDAC(nTrig, roc.GetAoutChipPosition()));
    }
    histograms->Add(meas);
}
