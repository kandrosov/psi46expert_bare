/*!
 * \file AnalogReadout.h
 * \brief Definition of AnalogReadout class.
 */

#pragma once

#include "BasePixel/Test.h"

/*!
 * \brief The AnalogReadout class.
 */
class AnalogReadout : public Test {
public:
    AnalogReadout(TestRange *testRange, TBInterface *aTBInterface);

    virtual void ModuleAction();

    void AddressLevelsTest();
    void DualModeTest();
};
