/*!
 * \file TestPixel.h
 * \brief Definition of TestPixel class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - removed deprecated conversion from string constant to char*
 */

#pragma once

#include "BasePixel/Pixel.h"
#include "SCurve.h"
#include <TH1D.h>
#include <TH2D.h>
#include <TGraph.h>

class TestRoc;

/*!
 * \brief Implementation of the tests at pixel level
 */
class TestPixel: public Pixel {
public:
    TestPixel(Roc *roc, int columnNumber, int rowNumber);
    TestRoc *GetRoc();
    double FindThreshold(const char* mapName, int nTrig, bool doubleWbc = false);
};
