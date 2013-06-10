/*!
 * \file VsfScan.h
 * \brief Definition of VsfScan class.
 */

#pragma once

#include <utility>
#include "PhDacScan.h"
#include "BasePixel/Test.h"

/*!
 * Test Scans a whole range of Vsf and plots
 * - PulseHeight linearity parameter in low Vcal range vs Vsf
 * - Digital Current vs Vsf
 */
class VsfScan: public Test {
public:
    VsfScan(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface);
    virtual void RocAction(TestRoc& roc);

private:
    void scan(TestRoc& roc);
    int  getTestColumn(TestRoc& roc);

    // Group Input Parameters
    struct Input {
        int start;
        int stop;
        int steps;
    };

    boost::shared_ptr<TBAnalogInterface> tbInterface;
    PhDacScan phDacScan;

    Input vsf;

    const DACParameters::Register VSF_DAC_REGISTER;

    // Vcal range used for PH scan
    const std::pair<const int, const int> PH_VCAL_RANGE;
};
