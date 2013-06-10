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
    SCurveTestBeam(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface);
    virtual void RocAction(TestRoc& roc);

private:
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    DecodedReadoutModule decodedModuleReadout;
    int nTrig, mode, vthr, vcal, sCurve[256];
    DACParameters::Register dacReg;
    TH2D *map;
};
