/*!
 * \file DacProgramming.h
 * \brief Definition of DacProgramming class.
 *
 * \b Changelog
 * 25-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version. Code moved from TestModule.
 */

#pragma once

#include "BasePixel/Test.h"
#include "BasePixel/TBAnalogInterface.h"

namespace psi {
namespace tests {
class DacProgramming : public Test
{
public:
    DacProgramming(boost::shared_ptr<TBAnalogInterface> _tbInterface,
                   const std::vector< boost::shared_ptr<TestRoc> >& _rocs);

    virtual void ModuleAction();
private:
    const std::vector< boost::shared_ptr<TestRoc> >& rocs;
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    int dacReg;
    int maxDacValue;
};
} // test
} // psi
