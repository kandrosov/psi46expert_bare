/*!
 * \file TimeWalkStudy.h
 * \brief Definition of TimeWalkStudy class.
 *
 * \b Changelog
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using definitions from PsiCommon.h.
 * 15-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using boost::units::quantity to represent physical values.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#pragma once

#include "BasePixel/Test.h"
#include "psi/units.h"
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
