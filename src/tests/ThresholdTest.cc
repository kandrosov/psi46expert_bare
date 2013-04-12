/*!
 * \file ThresholdTest.cc
 * \brief Implementation of ThresholdTest class.
 *
 * \b Changelog
 * 12-04-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Defined enum DacParameters::Register.
 * 09-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Corrected questionable language constructions, which was found using -Wall g++ option.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#include "ThresholdTest.h"
#include "BasePixel/ThresholdMap.h"
#include "BasePixel/Analysis.h"
#include "BasePixel/TestParameters.h"

ThresholdTest::ThresholdTest(TestRange *aTestRange, TBInterface *aTBInterface)
{
    psi::LogInfo() << "Threshold test\n";
    testRange = aTestRange;
    tbInterface = aTBInterface;
    ReadTestParameters();
}

void ThresholdTest::ReadTestParameters()
{
    const TestParameters& testParameters = TestParameters::Singleton();
    vcal = testParameters.ThresholdVcal();
    vthr = testParameters.ThresholdVthr();
    mode = testParameters.ThresholdMode();
    nTrig = testParameters.ThresholdNTrig();
}

void ThresholdTest::RocAction()
{
    SaveDacParameters();
    ThresholdMap *thresholdMap = new ThresholdMap();
    if (mode == 0) {
        SetDAC(DACParameters::Vcal, vcal);
        Flush();
        map = thresholdMap->GetMap("CalThresholdMap", roc, testRange, nTrig);
    } else if (mode == 1) {
        if (vthr >= 0) SetDAC(DACParameters::VthrComp, vthr);
        Flush();
        map = thresholdMap->GetMap("VcalThresholdMap", roc, testRange, nTrig);
    } else if (mode == 2) {
        if (vthr >= 0) SetDAC(DACParameters::VthrComp, vthr);
        SetDAC(DACParameters::CtrlReg, 4);
        Flush();
        map = thresholdMap->GetMap("VcalsThresholdMap", roc, testRange, nTrig);
    } else if (mode == 3) {
        if (vcal >= 0) SetDAC(DACParameters::Vcal, vcal);
        Flush();
        map = thresholdMap->GetMap("NoiseMap", roc, testRange, nTrig);
    } else if (mode == 4) {
        if (vcal >= 0) SetDAC(DACParameters::Vcal, vcal);
        SetDAC(DACParameters::CtrlReg, 4);
        Flush();
        map = thresholdMap->GetMap("CalXTalkMap", roc, testRange, nTrig);
    }
    histograms->Add(map);
    histograms->Add(Analysis::Distribution(map));

    RestoreDacParameters();
}
