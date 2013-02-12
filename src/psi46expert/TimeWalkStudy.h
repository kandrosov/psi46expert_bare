/*!
 * \file TimeWalkStudy.h
 * \brief Definition of TimeWalkStudy class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#pragma once

#include "Test.h"
#include "BasePixel/GlobalConstants.h"
#include <TF1.h>

/*!
 * \brief Time Walk Studies
 */
class TimeWalkStudy : public Test
{
public:
    TimeWalkStudy(TestRange *testRange, TBInterface *aTBInterface);
    virtual void ReadTestParameters();

    virtual void ModuleAction();
    virtual void RocAction();

    void CalDelDeltaT();
    void GetPowerSlope();
    double TimeWalk(int vcalStep);
    int FindNewVana();
    void SetThreshold(int vcal);
    int GetThreshold();

//   void TimeWalk16Chips();
//   void TimeWalk16Pixel();

protected:
    TF1 *fit;
    double powerSlope, calDelDT, zeroCurrent, meanShift, twBefore[MODULENUMROCS], twAfter[MODULENUMROCS];
    int vcalThreshold, vana[MODULENUMROCS];
};
