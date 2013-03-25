/*!
 * \file AnalogReadout.h
 * \brief Definition of AnalogReadout class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#pragma once

#include <TH1D.h>
#include "BasePixel/Test.h"
#include "BasePixel/RawPacketDecoder.h"
#include "BasePixel/DecoderCalibration.h"

/*!
 * \brief Test of the address levels.
 */
class AddressLevels : public Test {
public:
    AddressLevels(TestRange *testRange, TBInterface *aTBInterface);
    virtual ~AddressLevels();

    virtual void ModuleAction();
    virtual void RocAction();

protected:
    void TestTBM();
    void TestROC();

    void FindDecoderLevels(TH1* adcHistogram, int& numLimits, short limits[], int maxLimits, int integralLimit);

    unsigned short count;
    short data[psi::FIFOSIZE];
    int n;

    TH1D* adcHistogramTBM;
    TH1D* adcHistogramROC;

    short fLimitsTBM[DecoderCalibrationConstants::NUM_LEVELSTBM + 1];
    short fLimitsROC[RawPacketDecoderConstants::MAX_ROCS][DecoderCalibrationConstants::NUM_LEVELSROC + 1];
    bool fTestedROC[RawPacketDecoderConstants::MAX_ROCS];

    static bool fPrintDebug;
};
