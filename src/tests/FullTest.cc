/*!
 * \file FullTest.cc
 * \brief Implementation of FullTest class.
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

FullTest::FullTest(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface)
    : Test("FullTest", testRange), tbInterface(aTBInterface) {}

void FullTest::ModuleAction(TestModule& module)
{
    DoTest<TemperatureTest>(module);
    DoTest<SCurveTest>(module);
    if(!ConfigParameters::Singleton().TbmEmulator())
        DoTest<TBMTest>(module);
    DoTest<AnalogReadout>(module);
    Test::ModuleAction(module);
    DoTest<TemperatureTest>(module);
}

void FullTest::RocAction(TestRoc& roc)
{
    psi::LogDebug() << "[FullTest] Chip #" << roc.GetChipId() << ".\n";

    DoTest<PixelAlive>(roc);
    DoTest<BumpBonding>(roc);
//    DoTest<TrimBits>(roc);
//    DoTest<TemperatureTest>(roc);
//    DoTest<AddressDecoding>(roc);
//    DoTest<AddressLevels>(roc);

    psi::LogDebug() << "[FullTest] done for chip " << roc.GetChipId() << ".\n";
}

void FullTest::CollectHistograms(Test& test)
{
    TIter next(test.GetHistos().get());
    while (TH1 *histo = (TH1*)next()) histograms->Add(histo);
}
