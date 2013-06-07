/*!
 * \file PhNoise.h
 * \brief Definition of PhNoise class.
 */

#pragma once

#include <TH1D.h>
#include "BasePixel/Test.h"

/*!
 * \brief Measure the noise on the pulse height in the analog out signal
 */
class PhNoise : public Test {
public:
    PhNoise(TestRange *testRange, TBInterface *aTBInterface);
    virtual void ModuleAction();
    virtual void RocAction();

protected:
    unsigned short count;
    short data[psi::FIFOSIZE];
    static const int nReadouts = 1000;
    static bool debug;
};
