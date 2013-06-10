/*!
 * \file PhNoise.h
 * \brief Definition of PhNoise class.
 */

#pragma once

#include <TH1D.h>
#include "BasePixel/constants.h"
#include "BasePixel/Test.h"

/*!
 * \brief Measure the noise on the pulse height in the analog out signal
 */
class PhNoise : public Test {
public:
    PhNoise(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface);
    virtual void ModuleAction(TestModule& module);
    virtual void RocAction(TestRoc& roc);

private:
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    unsigned short count;
    short data[psi::FIFOSIZE];
    static const int nReadouts = 1000;
    static bool debug;
};
