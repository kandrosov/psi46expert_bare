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
    AnalogReadout(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface);

    virtual void ModuleAction(TestModule& module);

private:
    void AddressLevelsTest();
    void DualModeTest();

private:
    boost::shared_ptr<TBAnalogInterface> tbInterface;
};
