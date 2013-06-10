/*!
 * \file CalDelay.h
 * \brief Definition of CalDelay class.
 */

#pragma once

#include "BasePixel/Test.h"
#include "BasePixel/TestRange.h"

class CalDelay : public Test {
public:
    CalDelay(PTestRange testRange);
    virtual void RocAction(TestRoc& roc);

private:
    TObjArray* GetEfficiency(const std::string& testName, TestRoc& roc, PTestRange testRange);
};
