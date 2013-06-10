/*!
 * \file TrimBits.h
 * \brief Definition of TrimBits class.
 */

#pragma once

#include "BasePixel/Test.h"

/*!
 * \brief Trim bit test
 */
class TrimBits : public Test {
public:
    TrimBits(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface);
    virtual void RocAction(TestRoc& roc);

private:
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    int nTrig, vtrim14, vtrim13, vtrim11, vtrim7;
};
