/*!
 * \file TrimBits.cc
 * \brief Implementation of TrimBits class.
 */

#include "TrimBits.h"
#include "BasePixel/ThresholdMap.h"
#include "analysis/Analysis.h"
#include "psi/log.h"
#include "BasePixel/TestParameters.h"

namespace {
const std::string TEST_NAME = "TrimBits";
}

TrimBits::TrimBits(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface)
    : Test(TEST_NAME, testRange), tbInterface(aTBInterface)
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
    static const size_t NUMBER_OF_RUNS = 4;
    int trim[NUMBER_OF_RUNS] = { 14, 13, 11, 7 };
    int vtrim[NUMBER_OF_RUNS] = { vtrim14, vtrim13, vtrim11, vtrim7 };

    SaveDacParameters(roc);

    roc.SetDAC(DACParameters::Vtrim, 0);

    thrMap = thresholdMap.MeasureMap(ThresholdMap::CalThresholdMapParameters, roc, *testRange, nTrig, 1);
    histograms->Add(thrMap);

    for (size_t i = 0; i < NUMBER_OF_RUNS; ++i) {
        roc.SetDAC(DACParameters::Vtrim, vtrim[i]);
        roc.SetTrim(trim[i]);

        map = thresholdMap.MeasureMap(ThresholdMap::CalThresholdMapParameters, roc, *testRange, nTrig, i+2);
        histograms->Add(map);
        std::stringstream testHistoName;
        testHistoName << "TrimBit" << trim[i] << "_C" << roc.GetChipId() << "_nb" << (i+2);
        histograms->Add(Analysis::TrimBitTest(thrMap, map, testHistoName.str()));
    }

    RestoreDacParameters(roc);
}
