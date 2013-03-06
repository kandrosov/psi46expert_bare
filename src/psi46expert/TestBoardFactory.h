/*!
 * \file TestBoardFactory.h
 * \brief Definition of TestBoardFactory class.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 06-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#pragma once

#include "BasePixel/TBAnalogInterface.h"

namespace psi {
class TestBoardFactory
{
public:
    typedef boost::shared_ptr<TBAnalogInterface> AnalogTestBoardPtr;
    static AnalogTestBoardPtr MakeAnalog();

private:
    TestBoardFactory() {}
};

}
