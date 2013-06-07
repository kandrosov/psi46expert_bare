/*!
 * \file BareTest.h
 * \brief Definition of BareTest class.
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 */

#pragma once

#include <map>
#include "BasePixel/Test.h"

/*!
 * \brief Set of tests for bare module.
 */
class BareTest : public Test {
private:
    typedef std::map<std::string, Test*> TestMap;

public:
    BareTest(TestRange *testRange, TBAnalogInterface *aTBInterface, const char* subTestName);
    virtual void ModuleAction();

private:
    TestMap CreateSubTests();

private:
    TBAnalogInterface *tbAnalogInterface;
    std::string subTestName;
};
