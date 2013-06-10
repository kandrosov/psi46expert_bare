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
    TBMTest(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface);
    virtual void ModuleAction(TestModule& module);

private:
    void DualModeTest(TestModule& module);
    void ReadoutTest(TestModule& module);

private:
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    int result[2];
};
