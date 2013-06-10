/*!
 * \file TemperatureTest.h
 * \brief Definition of TemperatureTest class.
 */

#pragma once

#include "BasePixel/Test.h"

class TemperatureTest : public Test {
public:
    TemperatureTest(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface);
    virtual void RocAction(TestRoc& roc);

private:
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    int nTrig;
};
