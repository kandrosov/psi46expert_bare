/*!
 * \file BareTest.h
 * \brief Definition of BareTest class.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new ConfigParameters class definition.
 *      - Adaptation for the new TestParameters class definition.
 * 22-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#pragma once

#include <map>
#include "Test.h"

/*!
 * \brief Set of tests for bare module.
 */
class BareTest : public Test
{
private:
    typedef std::map<std::string, Test*> TestMap;

public:
    BareTest(TestRange *testRange, TBAnalogInterface *aTBInterface,const char* subTestName);
    virtual void ModuleAction();

private:
    TestMap CreateSubTests();

private:
    TBAnalogInterface *tbAnalogInterface;
    std::string subTestName;
};
