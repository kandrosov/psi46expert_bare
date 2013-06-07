/*!
 * \file ThrComp.h
 * \brief Definition of ThrComp class.
 */

#pragma once

#include "BasePixel/Test.h"

class ThrComp : public Test {
public:
    ThrComp(TestRange *testRange, TBInterface *aTBInterface);
    virtual void ReadTestParameters();
    virtual void RocAction();
    void RocActionAuxiliary(double data[], double dataMax[]);
};
