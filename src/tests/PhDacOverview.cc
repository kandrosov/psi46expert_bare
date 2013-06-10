/*!
 * \file PhDacOverview.cc
 * \brief Implementation of PhDacOverview class.
 */

#include <set>

#include "PhDacOverview.h"
#include "psi46expert/TestRoc.h"
#include "psi46expert/TestModule.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/constants.h"
#include "TCanvas.h"
#include "PhDacScan.h"
#include "BasePixel/TestParameters.h"

PhDacOverview::PhDacOverview(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface)
    : Test("PhDacOverview", testRange), tbInterface(aTBInterface)
{
    NumberOfSteps = TestParameters::Singleton().PHNumberOfSteps();
    debug = true;
}

void PhDacOverview::RocAction(TestRoc& roc)
{
    SaveDacParameters(roc);
    Test::RocAction(roc);
    RestoreDacParameters(roc);
}

void PhDacOverview::PixelAction(TestPixel& pixel)
{
    pixel.ArmPixel();
    tbInterface->Flush();
    DoDacScan(pixel);
    //  DoVsfScan(); // xxx test me!!!
    pixel.DisarmPixel();
}

void PhDacOverview::DoDacScan(TestPixel& pixel)
{
    psi::LogInfo() << " ************************* DAC SCAN **************************" << std::endl;

    int offset;
    if (tbInterface->TBMPresent()) offset = 16;
    else offset = 9;

    psi::LogInfo() << "chipId = " << pixel.GetRoc().GetChipId() << ", col = " << pixel.GetColumn()
                   << ", row = " << pixel.GetRow() << std::endl;

    for (int DacRegister = 1; DacRegister < 28; DacRegister++) {
        psi::LogInfo() << "DAC set to " << DacRegister << std::endl;
        int scanMax;
        if ((DacRegister == 1) || (DacRegister == 4) || (DacRegister == 6) || (DacRegister == 8) || (DacRegister == 14))
            scanMax = 16;
        else scanMax = 256;
        int defaultValue = pixel.GetRoc().GetDAC((DACParameters::Register)DacRegister);
        // int defaultValue2 = GetDAC(DacRegister+2);
        int loopNumber = 0;
        for (int scanValue = 0; scanValue < scanMax; scanValue += ((int)scanMax / NumberOfSteps)) {
            loopNumber++;
            const std::string& dacName = DACParameters::GetRegisterName((DACParameters::Register)DacRegister);

            TH1D *histo = new TH1D(Form("DAC%i_Value%i", DacRegister, loopNumber), Form("%s=%d", dacName.c_str(), scanValue), 256, 0, 256);
            psi::LogInfo() << "default value = " << defaultValue << std::endl;
            //psi::LogInfo() << "default value2 = " << defaultValue2 << endl;
            pixel.GetRoc().SetDAC((DACParameters::Register)DacRegister, scanValue);
            //SetDAC(DacRegister+2, scanValue);
            short result[256];
            tbInterface->PHDac(25, 256, phDacScan.GetNTrig(), offset + pixel.GetRoc().GetAoutChipPosition() * 3,
                               result);
            for (int dac = 0; dac < 256; dac++) {
                if (result[dac] == 7777) histo->SetBinContent(dac + 1, 0);
                else histo->SetBinContent(dac + 1, result[dac]);
            }
            histograms->Add(histo);
        }
        pixel.GetRoc().SetDAC((DACParameters::Register)DacRegister, defaultValue);
        //SetDAC(DacRegister+2, defaultValue2);
    }

    std::set<TBMParameters::Register> tbmRegistersToScan;
    tbmRegistersToScan.insert(TBMParameters::Inputbias);
    tbmRegistersToScan.insert(TBMParameters::Outputbias);
    tbmRegistersToScan.insert(TBMParameters::Dacgain);
    for(std::set<TBMParameters::Register>::const_iterator iter = tbmRegistersToScan.begin();
            iter != tbmRegistersToScan.end(); ++iter)
        DoTBMRegScan(pixel.GetRoc(), *iter, offset);
}

void PhDacOverview::DoTBMRegScan(TestRoc& roc, TBMParameters::Register DacRegister, int offset)
{
    psi::LogInfo() << "DAC set to " << DacRegister << std::endl;
    int scanMax = 256;
    int defaultValue = 0;
    const bool haveDefaultValue = roc.GetModule().GetTBM(DacRegister, defaultValue);
    int loopNumber = 0;
    const std::string dacName = TBMParameters::GetRegisterName(DacRegister);
    for (int scanValue = 0; scanValue < scanMax; scanValue += ((int)scanMax / NumberOfSteps)) {
        loopNumber++;

        TH1D *histo = new TH1D(Form("TBM_DAC%i_Value%i", DacRegister, loopNumber), Form("%s=%d", dacName.c_str(), scanValue), 256, 0, 256);
        psi::LogInfo() << "default value = " << defaultValue << std::endl;
        roc.GetModule().SetTBM(roc.GetChipId(), DacRegister, scanValue);
        short result[256];
        tbInterface->PHDac(25, 256, phDacScan.GetNTrig(), offset + roc.GetAoutChipPosition() * 3, result); ///!!!
        for (int dac = 0; dac < 256; dac++) {
            if (result[dac] == 7777) histo->SetBinContent(dac + 1, 0);
            else histo->SetBinContent(dac + 1, result[dac]);
        }
        histograms->Add(histo);
    }
    if(haveDefaultValue)
        roc.GetModule().SetTBM(roc.GetChipId(), DacRegister, defaultValue);
}


void PhDacOverview::DoVsfScan(TestRoc& roc)
{
    int offset;
    if (tbInterface->TBMPresent()) offset = 16;
    else offset = 9;
    int nTrig = 10;

    roc.SetDAC(DACParameters::CtrlReg, 4);

    for (int col = 0; col < 2; col++) {
        psi::LogInfo() << "col = " << col << std::endl;
        for (int row = 0; row < 2; row++) {
            for (int vsf = 150; vsf < 255; vsf += 20) {
                roc.GetDAC(DACParameters::Vsf);
                roc.SetDAC(DACParameters::Vsf, vsf);
                tbInterface->Flush();
                short result[256];
                tbInterface->PHDac(25, 256, nTrig, offset + roc.GetAoutChipPosition() * 3, result);
                std::ostringstream histoName;
                histoName << "Vsf" << vsf << "_Col" << col << "_Row" << row;
                TH1D *histo = new TH1D(histoName.str().c_str(), histoName.str().c_str(), 256, 0., 256.);
                for (int dac = 0; dac < 256; dac++) {
                    if (result[dac] == 7777) histo->SetBinContent(dac + 1, 555);
                    else histo->SetBinContent(dac + 1, result[dac]);
                }
            }
        }
    }
}
