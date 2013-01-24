/*!
 * \class BareTest
 * \brief Set of tests for bare module.
 *
 * \b Changelog
 * 22-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#pragma once

#include <map>
#include "Test.h"


class BareTest : public Test
{
private:
    typedef std::map<std::string, Test*> TestMap;

public:
    BareTest(ConfigParameters *configParameters, TestRange *testRange, TestParameters* testParameters, TBAnalogInterface *aTBInterface,const char* subTestName);
    virtual void ModuleAction();

private:
    TestMap CreateSubTests();

private:
    TBAnalogInterface *tbAnalogInterface;
    std::string subTestName;
};
