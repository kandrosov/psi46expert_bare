/*!
 * \file FullTest.h
 * \brief Definition of FullTest class.
 */

#include "BasePixel/Test.h"

/*!
 * \brief Full test of a module
 */
class FullTest : public Test {
public:
    FullTest(TestRange *testRange, TBInterface *aTBInterface, int opt);
    int Scurve;
    virtual void RocAction();
    virtual void ModuleAction();
    void DoTemperatureTest();
};
