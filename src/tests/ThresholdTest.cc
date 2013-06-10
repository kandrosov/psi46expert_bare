/*!
 * \file ThresholdTest.cc
 * \brief Implementation of ThresholdTest class.
 */

#include "ThresholdTest.h"
#include "BasePixel/ThresholdMap.h"
#include "BasePixel/Analysis.h"
#include "BasePixel/TestParameters.h"

ThresholdTest::ThresholdTest(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface)
    : Test("ThresholdTest", testRange), tbInterface(aTBInterface)
{
    const TestParameters& testParameters = TestParameters::Singleton();
    vcal = testParameters.ThresholdVcal();
    vthr = testParameters.ThresholdVthr();
    mode = testParameters.ThresholdMode();
    nTrig = testParameters.ThresholdNTrig();
}

void ThresholdTest::RocAction(TestRoc& roc)
{
    SaveDacParameters(roc);
    ThresholdMap thresholdMap;
    if (mode == 0) {
        roc.SetDAC(DACParameters::Vcal, vcal);
        roc.Flush();
        map = thresholdMap.GetMap("CalThresholdMap", roc, *testRange, nTrig);
    } else if (mode == 1) {
        if (vthr >= 0) roc.SetDAC(DACParameters::VthrComp, vthr);
        roc.Flush();
        map = thresholdMap.GetMap("VcalThresholdMap", roc, *testRange, nTrig);
    } else if (mode == 2) {
        if (vthr >= 0) roc.SetDAC(DACParameters::VthrComp, vthr);
        roc.SetDAC(DACParameters::CtrlReg, 4);
        roc.Flush();
        map = thresholdMap.GetMap("VcalsThresholdMap", roc, *testRange, nTrig);
    } else if (mode == 3) {
        if (vcal >= 0) roc.SetDAC(DACParameters::Vcal, vcal);
        roc.Flush();
        map = thresholdMap.GetMap("NoiseMap", roc, *testRange, nTrig);
    } else if (mode == 4) {
        if (vcal >= 0) roc.SetDAC(DACParameters::Vcal, vcal);
        roc.SetDAC(DACParameters::CtrlReg, 4);
        roc.Flush();
        map = thresholdMap.GetMap("CalXTalkMap", roc, *testRange, nTrig);
    }
    histograms->Add(map);
    histograms->Add(Analysis::Distribution(map));

    RestoreDacParameters(roc);
}
