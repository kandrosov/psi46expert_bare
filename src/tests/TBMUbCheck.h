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
    TBMUbCheck(TestRange* testRange, TBInterface* aTBInterface);

    virtual void ReadTestParameters();
    virtual void ModuleAction();

protected:
    int ubTarget;
};
