/*!
 * \file AnalogReadout.h
 * \brief Definition of AnalogReadout class.
 */

#pragma once

#include <TH1D.h>
#include "BasePixel/constants.h"
#include "BasePixel/Test.h"
#include "BasePixel/RawPacketDecoder.h"
#include "BasePixel/DecoderCalibration.h"

/*!
 * \brief Test of the address levels.
 */
class AddressLevels : public Test {
public:
    AddressLevels(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface);
    virtual ~AddressLevels();

    virtual void ModuleAction(TestModule& module);
    virtual void RocAction(TestRoc& roc);

private:
    void TestTBM();

    void FindDecoderLevels(TH1* adcHistogram, int& numLimits, short limits[], int maxLimits, int integralLimit);

    boost::shared_ptr<TBAnalogInterface> tbInterface;
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
