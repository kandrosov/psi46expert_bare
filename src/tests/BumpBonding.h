/*!
 * \file BumpBounding.h
 * \brief Definition of BumpBounding class.
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

