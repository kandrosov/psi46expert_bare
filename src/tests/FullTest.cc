/*!
 * \file FullTest.cc
 * \brief Implementation of FullTest class.
 *
 * \b Changelog
 * 09-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Corrected questionable language constructions, which was found using -Wall g++ option.
 * 02-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using psi::Sleep instead interface/Delay.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new ConfigParameters class definition.
 *      - Adaptation for the new TestParameters class definition.
 */

#include "psi/log.h"

#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/ConfigParameters.h"
#include "FullTest.h"
#include "PixelAlive.h"
#include "BumpBonding.h"
#include "AddressLevels.h"
#include "AddressDecoding.h"
#include "Trim.h"
#include "SCurveTest.h"
#include "DacDependency.h"
#include "TrimBits.h"
#include "PHTest.h"
#include "BasePixel/ThresholdMap.h"
#include "TemperatureTest.h"
#include "TBMTest.h"
#include "AnalogReadout.h"

FullTest::FullTest(TestRange *aTestRange, TBInterface *aTBInterface, int opt)
{
    psi::LogDebug() << "[FullTest] Initialization." << std::endl;
    testRange = aTestRange;
    tbInterface = aTBInterface;
    Scurve = opt;
}

void FullTest::ModuleAction()
{
    Test *test = 0;

    if(Scurve != 0) {
        psi::LogInfo() << "[FullTest] Start." << std::endl;

        psi::LogInfo().PrintTimestamp();
    }

    DoTemperatureTest();
    for (int iTest = 0; iTest < 3; iTest++) {
        if(Scurve == 0) {
            test = new SCurveTest(testRange, tbInterface);
            test->ModuleAction(module);
            break;
        }
        psi::LogInfo().PrintTimestamp();
        if (iTest == 0) test = new SCurveTest(testRange, tbInterface);
        if (iTest == 1 && !(ConfigParameters::Singleton().TbmEmulator())) test = new TBMTest(testRange, tbInterface);
        else if (iTest == 1) continue;
        if (iTest == 2) test = new AnalogReadout(testRange, tbInterface);
        test->ModuleAction(module);
        TIter next(test->GetHistos());
        while (TH1 *histo = (TH1*)next()) histograms->Add(histo);
    }

    if(Scurve != 0) {
        Test::ModuleAction();
        DoTemperatureTest();

        psi::LogInfo() << "[FullTest] End." << std::endl;
    }
}


void FullTest::RocAction()
{
    if(Scurve != 0) {
        psi::LogDebug() << "[FullTest] Chip #" << chipId << '.' << std::endl;
        Test *test = 0;

        for (int iTest = 0; iTest < 6; iTest++) {
            psi::LogInfo().PrintTimestamp();
            if (iTest == 0) test = new PixelAlive(testRange, tbInterface);
            if (iTest == 1) test = new BumpBonding(testRange, tbInterface);
            if (iTest == 2) test = new TrimBits(testRange, tbInterface);
            if (iTest == 3) test = new TemperatureTest(testRange, tbInterface);
            if (iTest == 4) test = new AddressDecoding(testRange, tbInterface);
            if (iTest == 5) test = new AddressLevels(testRange, tbInterface);
            test->RocAction(roc);
            TIter next(test->GetHistos());
            while (TH1 *histo = (TH1*)next()) histograms->Add(histo);
        }

        psi::LogDebug() << "[FullTest] done for chip " << chipId << '.'
                        << std::endl;
    }
}


void FullTest::DoTemperatureTest()
{
    psi::LogInfo().PrintTimestamp();
    psi::LogDebug() << "[FullTest] Temperature Test." << std::endl;

    Test* test = new TemperatureTest(testRange, tbInterface);
    test->ModuleAction(module);
    TIter next(test->GetHistos());
    while (TH1 *histo = (TH1*)next()) histograms->Add(histo);
}
