/*!
 * \file PHTest.h
 * \brief Definition of PHTest class.
 */

#pragma once

#include "BasePixel/Test.h"
#include <TH2D.h>
#include <TH1D.h>

class PHTest : public Test {
public:
    PHTest(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface);
    virtual void RocAction(TestRoc& roc);
    virtual void PixelAction(TestPixel& pixel);

private:
    void PhDac(TestPixel& pixel, const std::string& dacName);

private:
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    int nTrig, mode;
    TH2D *map;
};
