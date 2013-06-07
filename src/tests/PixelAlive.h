/*!
 * \file PixelAlive.h
 * \brief Definition of PixelAlive class.
 */

#pragma once

#include "BasePixel/Test.h"

/*!
 * \brief Pixel alive test, analog testboard version.
 */
class PixelAlive : public Test {
public:
    PixelAlive(TestRange *testRange, TBInterface *aTBInterface);

    virtual void ReadTestParameters();
    virtual void RocAction();

protected:
    int nTrig;
    double efficiency;
};
