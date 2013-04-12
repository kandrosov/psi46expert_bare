/*!
 * \file PhDacOverview.cc
 * \brief Implementation of PhDacOverview class.
 *
 * \b Changelog
 * 12-04-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Defined enum TBMParameters::Register.
 *      - Defined enum DacParameters::Register.
 * 13-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - TBMParameters class now inherit psi::BaseConifg class.
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using definitions from PsiCommon.h.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - removed deprecated conversion from string constant to char*
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

PhDacOverview::PhDacOverview(TestRange *aTestRange, TBInterface *aTBInterface)
    : PhDacScan(aTestRange, aTBInterface)
{
    testRange = aTestRange;
    tbInterface = aTBInterface;
    ReadTestParameters();
    debug = true;
}


void PhDacOverview::ReadTestParameters()
{
    PhDacScan::ReadTestParameters();
    NumberOfSteps = TestParameters::Singleton().PHNumberOfSteps();
}

void PhDacOverview::RocAction()
{
    SaveDacParameters();
    Test::RocAction();
    RestoreDacParameters();
}


void PhDacOverview::PixelAction()
{
    ArmPixel();
    Flush();
    DoDacScan();
    //  DoVsfScan(); // xxx test me!!!
    DisarmPixel();
}

void PhDacOverview::DoDacScan()
{
    psi::LogInfo() << " ************************* DAC SCAN **************************" << std::endl;

    int offset;
    if (((TBAnalogInterface*)tbInterface)->TBMPresent()) offset = 16;
    else offset = 9;

    psi::LogInfo() << "chipId = " << chipId << ", col = " << column << ", row = " << row << std::endl;

    for (int DacRegister = 1; DacRegister < 28; DacRegister++) {
        psi::LogInfo() << "DAC set to " << DacRegister << std::endl;
        int scanMax;
        if ((DacRegister == 1) || (DacRegister == 4) || (DacRegister == 6) || (DacRegister == 8) || (DacRegister == 14)) scanMax = 16;
        else scanMax = 256;
        int defaultValue = GetDAC((DACParameters::Register)DacRegister);
        // int defaultValue2 = GetDAC(DacRegister+2);
        int loopNumber = 0;
        for (int scanValue = 0; scanValue < scanMax; scanValue += ((int)scanMax / NumberOfSteps)) {
            loopNumber++;
            const std::string& dacName = DACParameters::GetRegisterName((DACParameters::Register)DacRegister);

            TH1D *histo = new TH1D(Form("DAC%i_Value%i", DacRegister, loopNumber), Form("%s=%d", dacName.c_str(), scanValue), 256, 0, 256);
            psi::LogInfo() << "default value = " << defaultValue << std::endl;
            //psi::LogInfo() << "default value2 = " << defaultValue2 << endl;
            SetDAC((DACParameters::Register)DacRegister, scanValue);
            //SetDAC(DacRegister+2, scanValue);
            short result[256];
            ((TBAnalogInterface*)tbInterface)->PHDac(25, 256, nTrig, offset + aoutChipPosition * 3, result);
            for (int dac = 0; dac < 256; dac++) {
                if (result[dac] == 7777) histo->SetBinContent(dac + 1, 0);
                else histo->SetBinContent(dac + 1, result[dac]);
            }
            histograms->Add(histo);
        }
        SetDAC((DACParameters::Register)DacRegister, defaultValue);
        //SetDAC(DacRegister+2, defaultValue2);
    }

    std::set<TBMParameters::Register> tbmRegistersToScan;
    tbmRegistersToScan.insert(TBMParameters::Inputbias);
    tbmRegistersToScan.insert(TBMParameters::Outputbias);
    tbmRegistersToScan.insert(TBMParameters::Dacgain);
    for(std::set<TBMParameters::Register>::const_iterator iter = tbmRegistersToScan.begin();
        iter != tbmRegistersToScan.end(); ++iter)
        DoTBMRegScan(*iter, offset);
}

void PhDacOverview::DoTBMRegScan(TBMParameters::Register DacRegister, int offset)
{
    psi::LogInfo() << "DAC set to " << DacRegister << std::endl;
    int scanMax = 256;
    int defaultValue = 0;
    const bool haveDefaultValue = module->GetTBM(DacRegister, defaultValue);
    int loopNumber = 0;
    const std::string dacName = TBMParameters::GetRegisterName(DacRegister);
    for (int scanValue = 0; scanValue < scanMax; scanValue += ((int)scanMax / NumberOfSteps)) {
        loopNumber++;

        TH1D *histo = new TH1D(Form("TBM_DAC%i_Value%i", DacRegister, loopNumber), Form("%s=%d", dacName.c_str(), scanValue), 256, 0, 256);
        psi::LogInfo() << "default value = " << defaultValue << std::endl;
        module->SetTBM(chipId, DacRegister, scanValue);
        short result[256];
        ((TBAnalogInterface*)tbInterface)->PHDac(25, 256, nTrig, offset + aoutChipPosition * 3, result); ///!!!
        for (int dac = 0; dac < 256; dac++) {
            if (result[dac] == 7777) histo->SetBinContent(dac + 1, 0);
            else histo->SetBinContent(dac + 1, result[dac]);
        }
        histograms->Add(histo);
    }
    if(haveDefaultValue)
        module->SetTBM(chipId, DacRegister, defaultValue);
}


void PhDacOverview::DoVsfScan()
{
    int offset;
    if (((TBAnalogInterface*)tbInterface)->TBMPresent()) offset = 16;
    else offset = 9;
    int nTrig = 10;

    SetDAC(DACParameters::CtrlReg, 4);

    for (int col = 0; col < 2; col++) {
        psi::LogInfo() << "col = " << col << std::endl;
        for (int row = 0; row < 2; row++) {
            for (int vsf = 150; vsf < 255; vsf += 20) {
                GetDAC(DACParameters::Vsf);
                SetDAC(DACParameters::Vsf, vsf);
                Flush();
                short result[256];
                ((TBAnalogInterface*)tbInterface)->PHDac(25, 256, nTrig, offset + aoutChipPosition * 3, result);
                TH1D *histo = new TH1D(Form("Vsf%d_Col%d_Row%d", vsf, col, row), Form("Vsf%d_Col%d_Row%d", vsf, col, row), 256, 0., 256.);
                for (int dac = 0; dac < 256; dac++) {
                    if (result[dac] == 7777) histo->SetBinContent(dac + 1, 555);
                    else histo->SetBinContent(dac + 1, result[dac]);
                }
            }
        }
    }
}
