/*!
 * \file TestBoardFactory.h
 * \brief Definition of TestBoardFactory class.
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 */

#pragma once

#include "BasePixel/TBAnalogInterface.h"

namespace psi {
class TestBoardFactory {
public:
    typedef boost::shared_ptr<TBAnalogInterface> AnalogTestBoardPtr;
    static AnalogTestBoardPtr MakeAnalog();

private:
    TestBoardFactory() {}
};

}
