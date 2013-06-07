/*!
 * \file SCurveTestBeam.h
 * \brief Definition of SCurveTestBeam class.
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
