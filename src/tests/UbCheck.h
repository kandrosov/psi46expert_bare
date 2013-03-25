/*!
 * \file UbCheck.h
 * \brief Definition of UbCheck class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#pragma once

#include "BasePixel/Test.h"

/*!
 * \brief Ensures that the pulse height is below the ultra black
 */
class UbCheck : public Test {
public:
    UbCheck();
    UbCheck(TestRange *testRange, TBInterface *aTBInterface);

    void Initialize();
    virtual void ReadTestParameters();
    virtual void RocAction();
    void Measure(int nTriggers);
    int Ultrablack();
    void AdjustOpR0();

private:
    int nTrig, minPixel;
};
