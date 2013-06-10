/*!
 * \file TrimBits.cc
 * \brief Implementation of TrimBits class.
 */

#include "TrimBits.h"
#include "BasePixel/ThresholdMap.h"
#include "BasePixel/Analysis.h"
#include "psi/log.h"
#include "BasePixel/TestParameters.h"

TrimBits::TrimBits(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface)
    : Test("TrimBits", testRange), tbInterface(aTBInterface)
{
    const TestParameters& testParameters = TestParameters::Singleton();
    vtrim14 = testParameters.TrimBitsVtrim14();
    vtrim13 = testParameters.TrimBitsVtrim13();
    vtrim11 = testParameters.TrimBitsVtrim11();
    vtrim7 = testParameters.TrimBitsVtrim7();
    nTrig = testParameters.TrimBitsNTrig();
}

void TrimBits::RocAction(TestRoc& roc)
{
    ThresholdMap thresholdMap;
    TH2D *map, *thrMap;
    int trim, vtrim;

    SaveDacParameters(roc);

    roc.SetDAC(DACParameters::Vtrim, 0);

    thrMap = thresholdMap.GetMap("CalThresholdMap", roc, *testRange, nTrig);
    histograms->Add(thrMap);

    for (int i = 0; i < 4; i++) {
        if (i == 0) {
            trim = 14;
            vtrim = vtrim14;
        } else if (i == 1) {
            trim = 13;
            vtrim = vtrim13;
        } else if (i == 2) {
            trim = 11;
            vtrim = vtrim11;
        } else if (i == 3) {
            trim = 7;
            vtrim = vtrim7;
        }

        roc.SetDAC(DACParameters::Vtrim, vtrim);
        roc.SetTrim(trim);

        map = thresholdMap.GetMap("CalThresholdMap", roc, *testRange, nTrig);
        histograms->Add(map);
        histograms->Add(Analysis::TrimBitTest(thrMap, map, Form("TrimBit%i_C%i", trim, roc.GetChipId())));
    }

    RestoreDacParameters(roc);
}
