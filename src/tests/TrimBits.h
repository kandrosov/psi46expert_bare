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
    TrimBits(TestRange *testRange, TBInterface *aTBInterface);
    virtual void ReadTestParameters();
    virtual void RocAction();

protected:
    int nTrig, vtrim14, vtrim13, vtrim11, vtrim7;
};
