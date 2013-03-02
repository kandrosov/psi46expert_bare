/*!
 * \file BareTest.cc
 * \brief Implementation of BareTest class.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 02-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using psi::Sleep instead interface/Delay.
 * 15-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Removed ChipStartupTest because it functionality alredy implemented in psi46expert.cpp.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new ConfigParameters class definition.
 *      - Adaptation for the new TestParameters class definition.
 * 22-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#include <map>

#include "psi/log.h"

#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/ConfigParameters.h"
#include "BareTest.h"
#include "PixelAlive.h"
#include "BumpBonding.h"
#include "AddressLevels.h"
#include "AddressDecoding.h"
#include "Trim.h"
#include "SCurveTest.h"
#include "DacDependency.h"
#include "TrimBits.h"
#include "PHTest.h"
#include "ThresholdMap.h"
#include "TemperatureTest.h"
#include "TBMTest.h"
#include "AnalogReadout.h"

BareTest::BareTest(TestRange *aTestRange, TBAnalogInterface *aTBInterface, const char* _subTestName)
{
  psi::Log<psi::Debug>() << "[BareTest] Initialization." << std::endl;
  testRange = aTestRange;
  tbInterface = aTBInterface;
  tbAnalogInterface = aTBInterface;
  subTestName = _subTestName;
}

void BareTest::ModuleAction()
{
    psi::Log<psi::Info>() << "[BareTest] Start." << std::endl;
    TestMap tests = CreateSubTests();
    if(subTestName.c_str())
    {
        TestMap::const_iterator iter = tests.find(subTestName);
        if(iter != tests.end())
            iter->second->ModuleAction();
        else
            psi::Log<psi::Info>() << "[BareTest] ERROR: subtest '" << subTestName << "' not found." << std::endl;
    }
    else
    {
        for(TestMap::const_iterator iter = tests.begin(); iter != tests.end(); ++iter)
            iter->second->ModuleAction();
    }
    psi::Log<psi::Info>() << "[BareTest] End." << std::endl;
}

BareTest::TestMap BareTest::CreateSubTests()
{
    TestMap tests;
    return tests;
}
