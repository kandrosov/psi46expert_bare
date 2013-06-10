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
    PixelAlive(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface);

    virtual void RocAction(TestRoc& roc);

private:
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    int nTrig;
    double efficiency;
};
