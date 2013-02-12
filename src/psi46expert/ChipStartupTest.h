/*!
 * \file ChipStartupTest.h
 * \brief Definition of ChipStartupTest class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new ConfigParameters class definition.
 * 22-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#pragma once

#include "Test.h"
#include "BasePixel/TBAnalogInterface.h"

/*!
 * \brief Test if chip starts correctly.
 *
 * Test procedure:
 * 1. Power on test board.
 * 2. Check if analog and digital currents is OK.
 * 3. Initialize chip.
 * 4. Check if analog and digital currents is OK.
 */
class ChipStartupTest : public Test
{
public:
    static const std::string NAME;

public:
    ChipStartupTest(TestRange *testRange, TestParameters* testParameters, TBAnalogInterface *aTBInterface);
    virtual void ModuleAction();

private:
    TBAnalogInterface* tbAnalogInterface;
};
