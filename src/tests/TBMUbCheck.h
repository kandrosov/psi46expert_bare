/*!
 * \file TBMUbCheck.h
 * \brief Definition of TBMUbCheck class.
 */

#pragma once

#include "BasePixel/Test.h"

/*!
 * \brief Tests ultra-black levels of tbms.
 */
class TBMUbCheck : public Test {
public:
    TBMUbCheck(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface);
    virtual void ModuleAction(TestModule& module);

private:
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    int ubTarget;
};
