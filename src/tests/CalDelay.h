/*!
 * \file CalDelay.h
 * \brief Definition of CalDelay class.
 */

#pragma once

#include "BasePixel/Test.h"
#include "BasePixel/TestRange.h"

class CalDelay : public Test {
public:
    CalDelay(TestRange *testRange, TBInterface *aTBInterface);
    virtual void ReadTestParameters();
    virtual void RocAction();

protected:
    TObjArray* GetEfficiency(const char* testName, TestRange* testRange);
};
