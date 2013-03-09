/*!
 * \file TBMUbCheck.h
 * \brief Definition of TBMUbCheck class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#pragma once

#include "BasePixel/Test.h"

/*!
 * \brief Tests ultra-black levels of tbms.
 */
class TBMUbCheck : public Test
{
public:
    TBMUbCheck(TestRange* testRange, TBInterface* aTBInterface);

    virtual void ReadTestParameters();
    virtual void ModuleAction();

protected:
    int ubTarget;
};
