/*!
 * \file TBMTest.h
 * \brief Definition of TBMTest class.
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
