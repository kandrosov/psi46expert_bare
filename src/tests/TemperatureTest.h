/*!
 * \file TemperatureTest.h
 * \brief Definition of TemperatureTest class.
 */

#pragma once

#include "BasePixel/Test.h"

class TemperatureTest : public Test {
public:
    TemperatureTest(TestRange *testRange, TBInterface *aTBInterface);
    virtual void ReadTestParameters();
    virtual void RocAction();

protected:
    int nTrig;
};
