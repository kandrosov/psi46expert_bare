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
    BumpBonding(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface);

    virtual void RocAction(TestRoc& roc);

private:
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    int nTrig, vthrComp, ctrlReg;
};
