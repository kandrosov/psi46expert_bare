/*!
 * \file TimeWalkStudy.h
 * \brief Definition of TimeWalkStudy class.
 */

#pragma once

#include "BasePixel/Test.h"
#include "psi/units.h"
#include <TF1.h>

/*!
 * \brief Time Walk Studies
 */
class TimeWalkStudy : public Test {
public:
    TimeWalkStudy(TestRange *testRange, TBInterface *aTBInterface);
    virtual void ReadTestParameters();

    virtual void ModuleAction();
    virtual void RocAction();

    void CalDelDeltaT();
    void GetPowerSlope();
    psi::Time TimeWalk(int vcalStep);
    int FindNewVana();
    void SetThreshold(int vcal);
    int GetThreshold();

//   void TimeWalk16Chips();
//   void TimeWalk16Pixel();

protected:
    TF1 *fit;
    double calDelDT, meanShift;
    psi::ElectricCurrent zeroCurrent;
    psi::Time twBefore[psi::MODULENUMROCS], twAfter[psi::MODULENUMROCS];
    psi::CurrentPerTime powerSlope;
    int vcalThreshold, vana[psi::MODULENUMROCS];
};
