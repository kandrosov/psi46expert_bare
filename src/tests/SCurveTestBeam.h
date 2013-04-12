/*!
 * \file SCurveTestBeam.h
 * \brief Definition of SCurveTestBeam class.
 *
 * \b Changelog
 * 12-04-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Defined enum DacParameters::Register.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - removed deprecated conversion from string constant to char*
 */

#pragma once

#include "BasePixel/Test.h"
#include <TH2D.h>

#include "BasePixel/DecodedReadout.h"

class RawPacketDecoder;

class SCurveTestBeam : public Test {
public:
    SCurveTestBeam(TestRange *testRange, TBInterface *aTBInterface);

    virtual void ReadTestParameters();
    virtual void RocAction();
    virtual void PixelAction();

protected:
    DecodedReadoutModule decodedModuleReadout;
    int nTrig, mode, vthr, vcal, sCurve[256];
    DACParameters::Register dacReg;
    TH2D *map;
    RawPacketDecoder *gDecoder;
};
