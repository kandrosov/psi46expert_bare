/*!
 * \file PHTest.cc
 * \brief Implementation of PHTest class.
 *
 * \b Changelog
 * 12-04-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Defined enum DacParameters::Register.
 * 09-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Corrected questionable language constructions, which was found using -Wall g++ option.
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using definitions from PsiCommon.h.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - removed deprecated conversion from string constant to char*
 */

#include "PHTest.h"
#include "psi46expert/TestRoc.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/TestParameters.h"

PHTest::PHTest(TestRange *aTestRange, TBInterface *aTBInterface)
{
    testRange = aTestRange;
    tbInterface = aTBInterface;
    ReadTestParameters();
}

void PHTest::ReadTestParameters()
{
    const TestParameters& testParameters = TestParameters::Singleton();
    mode = testParameters.PHMode();
    nTrig = testParameters.PHNTrig();
}

void PHTest::RocAction()
{
    SaveDacParameters();
    if (mode == 0) {
        map = new TH2D(Form("PH_C%d", chipId), Form("PH_C%d", chipId), psi::ROCNUMCOLS, 0, psi::ROCNUMCOLS, psi::ROCNUMROWS,
                       0, psi::ROCNUMROWS);
        int data[psi::ROCNUMROWS * psi::ROCNUMCOLS], offset;
        if (((TBAnalogInterface*)tbInterface)->TBMPresent()) offset = 16;
        else offset = 9;
        roc->AoutLevelChip(offset + aoutChipPosition * 3, nTrig, data);
        for (unsigned col = 0; col < psi::ROCNUMCOLS; col++) {
            for (unsigned row = 0; row < psi::ROCNUMROWS; row++) map->SetBinContent(col + 1, row + 1, data[col * psi::ROCNUMROWS + row]);
        }
        histograms->Add(map);

    }
    Test::RocAction();
    RestoreDacParameters();
}


void PHTest::PixelAction()
{
    if (mode == 0) {}
    else {
        const std::string& dacName = DACParameters::GetRegisterName((DACParameters::Register)mode);
        PhDac(dacName.c_str());
    }
}


void PHTest::PhDac(const char *dacName)
{
    TH1D *histo = new TH1D(Form("Ph%s_c%dr%d_C%d", dacName, pixel->GetColumn(), pixel->GetRow(), roc->GetChipId()), Form("Ph%s_c%dr%d_C%d", dacName, pixel->GetColumn(), pixel->GetRow(), roc->GetChipId()), 256, 0, 256);
    TH1D *ubHist = new TH1D("ubHist", "ubHist", 256, 0, 256);
    ubHist->SetLineColor(kRed);

    EnablePixel();
    Cal();
    Flush();

    short result[256], data[10000];
    int offset;
    int ubPosition = 8 + aoutChipPosition * 3;
    unsigned short count;

    if (((TBAnalogInterface*)tbInterface)->TBMPresent()) offset = 16;
    else offset = 9;
    ((TBAnalogInterface*)tbInterface)->PHDac(mode, 256, nTrig, offset + aoutChipPosition * 3, result);

    ((TBAnalogInterface*)tbInterface)->ADCData(data, count);

    int ubLevel = data[ubPosition];

    for (int dac = 0; dac < 256; dac++) {
        if (result[dac] == 7777) histo->SetBinContent(dac + 1, 0);
        else histo->SetBinContent(dac + 1, result[dac]);
        ubHist->SetBinContent(dac + 1, ubLevel);
    }

    roc->ClrCal();
    DisablePixel();
    histograms->Add(histo);
    histograms->Add(ubHist);
}
