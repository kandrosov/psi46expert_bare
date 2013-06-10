/*!
 * \file UbCheck.h
 * \brief Definition of UbCheck class.
 */

#pragma once

#include "BasePixel/Test.h"

/*!
 * \brief Ensures that the pulse height is below the ultra black
 */
class UbCheck : public Test {
public:
    UbCheck(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface);
    virtual void RocAction(TestRoc& roc);

private:
    void Initialize();
    void Measure(int nTriggers);
    int Ultrablack();
    void AdjustOpR0(TestRoc& roc);

private:
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    int nTrig, minPixel;
};
