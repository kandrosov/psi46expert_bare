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

ChipVariation::ChipVariation(TestRange *aTestRange, TBInterface *aTBInterface)
    : PhDacScan(aTestRange, aTBInterface)
{
    testRange = aTestRange;
    tbInterface = aTBInterface;
    ReadTestParameters();
    debug = true;
}


void ChipVariation::ReadTestParameters()
{
    PhDacScan::ReadTestParameters();
    NumberOfSteps = TestParameters::Singleton().PHNumberOfSteps();
}


void ChipVariation::ModuleAction()
{
    linRange = new TH1D("Linear_Range", "Linear Range", 2000, 0, 2000);
    Test::ModuleAction();
    histograms->Add(linRange);
}

void ChipVariation::RocAction()
{
    SaveDacParameters();
    Test::RocAction();
    RestoreDacParameters();
}


void ChipVariation::PixelAction()
{
    ArmPixel();
    Flush();
    Scan();
    DisarmPixel();
}

void ChipVariation::Scan()
{

    int linearRange;

    SetDAC(DACParameters::CtrlReg, 4);

    int offset;
    if (((TBAnalogInterface*)tbInterface)->TBMPresent()) offset = 16;
    else offset = 9;

    TH1D *histo = new TH1D(Form("ROC%i_Col%i_Row%i", chipId, column, row), Form("ROC%i_Col%i_Row%i", chipId, column, row), 256, 0, 256);
    psi::LogInfo() << "Chip position " << aoutChipPosition << std::endl;
    short result[256];
    ((TBAnalogInterface*)tbInterface)->PHDac(25, 256, nTrig, offset + aoutChipPosition * 3, result);
    for (int dac = 0; dac < 256; dac++) {
        if (result[dac] == 7777) histo->SetBinContent(dac + 1, 0);
        else histo->SetBinContent(dac + 1, result[dac]);
    }
    linearRange = static_cast<int>( FindLinearRange(histo) );
    psi::LogInfo() << "LINEAR RANGE = " << linearRange << std::endl;
    linRange->Fill(linearRange);
    histograms->Add(histo);

}
