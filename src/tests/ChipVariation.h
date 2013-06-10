/*!
 * \file ChipVariation.h
 * \brief Definition of ChipVariation class.
 */

#pragma once

#include "BasePixel/Test.h"
#include "PhDacScan.h"
#include <TH2D.h>
#include <TH1D.h>
#include <TArrayD.h>

/*!
 * \brief pulse height - Vcal overview for different chips / pixels
 */
class ChipVariation : public Test {
public:
    ChipVariation(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface);

    virtual void RocAction(TestRoc& roc);
    virtual void PixelAction(TestPixel& pixel);
    virtual void ModuleAction(TestModule& module);

private:
    void Scan(TestPixel& pixel);

private:
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    PhDacScan phDacScan;
    int NumberOfSteps;
    TH1D *linRange;
};
