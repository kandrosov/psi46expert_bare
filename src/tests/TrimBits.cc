/*!
 * \file TrimBits.cc
 * \brief Implementation of TrimBits class.
 *
 * \b Changelog
 * 09-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Corrected questionable language constructions, which was found using -Wall g++ option.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#include "TrimBits.h"
#include "BasePixel/ThresholdMap.h"
#include "BasePixel/Analysis.h"
#include "psi/log.h"
#include "BasePixel/TestParameters.h"

TrimBits::TrimBits(TestRange *aTestRange, TBInterface *aTBInterface)
{
    psi::LogDebug() << "[TrimBits] Initialization." << std::endl;

    testRange = aTestRange;
    tbInterface = aTBInterface;
    ReadTestParameters();
}


void TrimBits::ReadTestParameters()
{
    const TestParameters& testParameters = TestParameters::Singleton();
    vtrim14 = testParameters.TrimBitsVtrim14();
    vtrim13 = testParameters.TrimBitsVtrim13();
    vtrim11 = testParameters.TrimBitsVtrim11();
    vtrim7 = testParameters.TrimBitsVtrim7();
    nTrig = testParameters.TrimBitsNTrig();
}


void TrimBits::RocAction()
{
    ThresholdMap *thresholdMap = new ThresholdMap();
    TH2D *map, *thrMap;
    int trim, vtrim;

    SaveDacParameters();

    SetDAC("Vtrim", 0);

    thrMap = thresholdMap->GetMap("CalThresholdMap", roc, testRange, nTrig);
    thrMap->Write();
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

        SetDAC("Vtrim", vtrim);
        roc->SetTrim(trim);

        map = thresholdMap->GetMap("CalThresholdMap", roc, testRange, nTrig);
        map->Write();
        histograms->Add(map);
        histograms->Add(Analysis::TrimBitTest(thrMap, map, Form("TrimBit%i_C%i", trim, roc->GetChipId())));

    }

    RestoreDacParameters();
}


