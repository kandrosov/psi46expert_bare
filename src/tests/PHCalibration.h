/*!
 * \file PHCalibration.h
 * \brief Definition of PHCalibration class.
 */

#pragma once

#include "BasePixel/Test.h"

/*!
 * \brief Pulse height calibration functions.
 */
class PHCalibration : public Test {
public:
    PHCalibration();
    PHCalibration(TestRange *testRange, TBInterface *aTBInterface);

    void Initialize();
    virtual void ReadTestParameters();
    virtual void RocAction();
    void Measure(int nTriggers);
    int GetCalDel(int vcalStep);
    int GetVthrComp(int vcalStep);

private:
    int vcal[512], ctrlReg[512];
    int mode, vcalSteps, nTrig, numPixels, calDelVthrComp;
    int calDel50, calDel100, calDel200, vthrComp50, vthrComp100, vthrComp200;
};
