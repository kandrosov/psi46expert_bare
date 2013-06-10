/*!
 * \file DacProgramming.h
 * \brief Definition of DacProgramming class.
 */

#pragma once

#include "BasePixel/Test.h"
#include "BasePixel/TBAnalogInterface.h"

namespace psi {
namespace tests {
class DacProgramming : public Test {
public:
    DacProgramming(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface);

    virtual void RocAction(TestRoc& roc);
private:
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    DACParameters::Register dacReg;
    int maxDacValue;
};
} // test
} // psi
