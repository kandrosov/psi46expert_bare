/*!
 * \file BumpBounding.h
 * \brief Definition of BumpBounding class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#pragma once

#include "BasePixel/Test.h"

/*!
 * \brief Bumpbonding test
 */
class BumpBonding : public Test {

public:
    BumpBonding(TestRange *testRange, TBInterface *aTBInterface);

    virtual void ReadTestParameters();
    virtual void RocAction();

protected:

    int nTrig, vthrComp, ctrlReg;
};

