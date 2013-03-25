/*!
 * \file ThrComp.h
 * \brief Definition of ThrComp class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
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
