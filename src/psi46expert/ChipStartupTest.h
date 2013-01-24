/*!
 * \class ChipStartupTest
 * \brief Test if chip starts correctly.
 *
 * Test procedure:
 * 1. Power on test board.
 * 2. Check if analog and digital currents is OK.
 * 3. Initialize chip.
 * 4. Check if analog and digital currents is OK.
 *
 * \b Changelog
 * 22-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#pragma once

#include "Test.h"
#include "BasePixel/TBAnalogInterface.h"

class ChipStartupTest : public Test
{
public:
    static const std::string NAME;

public:
    ChipStartupTest(ConfigParameters *configParameters, TestRange *testRange, TestParameters* testParameters, TBAnalogInterface *aTBInterface);
    virtual void ModuleAction();

private:
    TBAnalogInterface* tbAnalogInterface;
};
