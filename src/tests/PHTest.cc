/*!
 * \file PHTest.cc
 * \brief Implementation of PHTest class.
 */

#include "PHTest.h"
#include "psi46expert/TestRoc.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/TestParameters.h"

PHTest::PHTest(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface)
    : Test("PHTest", testRange), tbInterface(aTBInterface)
{
    const TestParameters& testParameters = TestParameters::Singleton();
    mode = testParameters.PHMode();
    nTrig = testParameters.PHNTrig();
}

void PHTest::RocAction(TestRoc& roc)
{
    SaveDacParameters(roc);
    if (mode == 0) {
        std::ostringstream mapName;
        mapName << "PH_C" << roc.GetChipId();
        map = new TH2D(mapName.str().c_str(), mapName.str().c_str(), psi::ROCNUMCOLS, 0, psi::ROCNUMCOLS,
                       psi::ROCNUMROWS, 0, psi::ROCNUMROWS);
        int data[psi::ROCNUMROWS * psi::ROCNUMCOLS], offset;
        if (tbInterface->TBMPresent()) offset = 16;
        else offset = 9;
        roc.AoutLevelChip(offset + roc.GetAoutChipPosition() * 3, nTrig, data);
        for (unsigned col = 0; col < psi::ROCNUMCOLS; col++) {
            for (unsigned row = 0; row < psi::ROCNUMROWS; row++)
                map->SetBinContent(col + 1, row + 1, data[col * psi::ROCNUMROWS + row]);
        }
        histograms->Add(map);

    }
    Test::RocAction(roc);
    RestoreDacParameters(roc);
}

void PHTest::PixelAction(TestPixel& pixel)
{
    if(mode) {
        const std::string& dacName = DACParameters::GetRegisterName((DACParameters::Register)mode);
        PhDac(pixel, dacName);
    }
}

void PHTest::PhDac(TestPixel& pixel, const std::string& dacName)
{
    std::ostringstream histoName;
    histoName << "Ph" << dacName << "_c" << pixel.GetColumn() << "r" << pixel.GetRow()
              << "_C" << pixel.GetRoc().GetChipId();
    TH1D *histo = new TH1D(histoName.str().c_str(), histoName.str().c_str(), 256, 0, 256);
    TH1D *ubHist = new TH1D("ubHist", "ubHist", 256, 0, 256);
    ubHist->SetLineColor(kRed);

    pixel.EnablePixel();
    pixel.Cal();
    tbInterface->Flush();

    short result[256], data[10000];
    int offset;
    int ubPosition = 8 + pixel.GetRoc().GetAoutChipPosition() * 3;
    unsigned short count;

    if (tbInterface->TBMPresent()) offset = 16;
    else offset = 9;
    tbInterface->PHDac(mode, 256, nTrig, offset + pixel.GetRoc().GetAoutChipPosition() * 3, result);

    tbInterface->ADCData(data, count);

    int ubLevel = data[ubPosition];

    for (int dac = 0; dac < 256; dac++) {
        if (result[dac] == 7777) histo->SetBinContent(dac + 1, 0);
        else histo->SetBinContent(dac + 1, result[dac]);
        ubHist->SetBinContent(dac + 1, ubLevel);
    }

    pixel.GetRoc().ClrCal();
    pixel.DisablePixel();
    histograms->Add(histo);
    histograms->Add(ubHist);
}
