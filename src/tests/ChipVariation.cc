/*!
 * \file ChipVariation.cc
 * \brief Implementation of ChipVariation class.
 */

#include "ChipVariation.h"
#include "psi46expert/TestRoc.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/constants.h"
#include "TCanvas.h"
#include "PhDacScan.h"
#include "BasePixel/TestParameters.h"

ChipVariation::ChipVariation(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface)
    : Test("ChipVariation", testRange), tbInterface(aTBInterface)
{
    NumberOfSteps = TestParameters::Singleton().PHNumberOfSteps();
}

void ChipVariation::ModuleAction(TestModule& module)
{
    linRange = new TH1D("Linear_Range", "Linear Range", 2000, 0, 2000);
    Test::ModuleAction(module);
    histograms->Add(linRange);
}

void ChipVariation::RocAction(TestRoc& roc)
{
    SaveDacParameters(roc);
    Test::RocAction(roc);
    RestoreDacParameters(roc);
}

void ChipVariation::PixelAction(TestPixel& pixel)
{
    pixel.ArmPixel();
    tbInterface->Flush();
    Scan(pixel);
    pixel.DisarmPixel();
}

void ChipVariation::Scan(TestPixel &pixel)
{
    pixel.GetRoc().SetDAC(DACParameters::CtrlReg, 4);

    int offset;
    if (tbInterface->TBMPresent()) offset = 16;
    else offset = 9;

    std::ostringstream histoName;
    histoName << "ROC" << pixel.GetRoc().GetChipId() << "_Col" << pixel.GetColumn() << "_Row" << pixel.GetRow();
    TH1D *histo = new TH1D(histoName.str().c_str(), histoName.str().c_str(), 256, 0, 256);
    const int aoutChipPosition = pixel.GetRoc().GetAoutChipPosition();
    psi::LogInfo() << "Chip position " << aoutChipPosition << std::endl;
    short result[256];
    tbInterface->PHDac(25, 256, phDacScan.GetNTrig(), offset + aoutChipPosition * 3, result);
    for (int dac = 0; dac < 256; dac++) {
        if (result[dac] == 7777) histo->SetBinContent(dac + 1, 0);
        else histo->SetBinContent(dac + 1, result[dac]);
    }
    const int linearRange = static_cast<int>( phDacScan.FindLinearRange(histo) );
    psi::LogInfo() << "LINEAR RANGE = " << linearRange << std::endl;
    linRange->Fill(linearRange);
    histograms->Add(histo);

}
