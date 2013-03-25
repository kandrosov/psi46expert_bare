/*!
 * \file TBMTest.h
 * \brief Definition of TBMTest class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#pragma once

#include "BasePixel/Test.h"

/*!
 * \brief Tests address levels and both tbms
 */
class TBMTest : public Test {
public:
    TBMTest(TestRange *testRange, TBInterface *aTBInterface);
    virtual void ModuleAction();
    void DualModeTest();
    void ReadoutTest();

protected:
    int result[2];
};
