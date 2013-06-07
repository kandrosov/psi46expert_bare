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
    DacProgramming(boost::shared_ptr<TBAnalogInterface> _tbInterface,
                   const std::vector< boost::shared_ptr<TestRoc> >& _rocs);

    virtual void ModuleAction();
private:
    const std::vector< boost::shared_ptr<TestRoc> >& rocs;
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    DACParameters::Register dacReg;
    int maxDacValue;
};
} // test
} // psi
