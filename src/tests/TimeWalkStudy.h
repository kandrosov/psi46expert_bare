/*!
 * \file TimeWalkStudy.h
 * \brief Definition of TimeWalkStudy class.
 */

#pragma once

#include <TF1.h>
#include "psi/units.h"
#include "BasePixel/constants.h"
#include "BasePixel/Test.h"

/*!
 * \brief Time Walk Studies
 */
class TimeWalkStudy : public Test {
public:
    TimeWalkStudy(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface);

    virtual void ModuleAction(TestModule& module);
    virtual void RocAction(TestRoc& roc);

private:
    void CalDelDeltaT(TestRoc &roc);
    void GetPowerSlope(TestPixel &pixel);
    psi::Time TimeWalk(TestRoc &roc, int vcalStep);
    int FindNewVana(TestPixel &pixel);
    void SetThreshold(TestPixel &pixel, int vcal);
    int GetThreshold(TestPixel& pixel);

private:
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    TF1 *fit;
    double calDelDT, meanShift;
    psi::ElectricCurrent zeroCurrent;
    psi::Time twBefore[psi::MODULENUMROCS], twAfter[psi::MODULENUMROCS];
    psi::CurrentPerTime powerSlope;
    int vcalThreshold, vana[psi::MODULENUMROCS];
};
