/*!
 * \file VsfScan.h
 * \brief Definition of VsfScan class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#pragma once

#include <utility>
#include "PhDacScan.h"

class TestRange;
class TBInterface;

/*!
 * Test Scans a whole range of Vsf and plots
 * - PulseHeight linearity parameter in low Vcal range vs Vsf
 * - Digital Current vs Vsf
 */
class VsfScan: public PhDacScan {
public:
    VsfScan( TestRange *_range, TBInterface *_interface);
    virtual ~VsfScan() {}

    virtual void RocAction();

private:
    void scan();
    int  getTestColumn();

    // Group Input Parameters
    struct Input {
        int start;
        int stop;
        int steps;
    };

    Input vsf;

    const int VSF_DAC_REGISTER;

    // Vcal range used for PH scan
    const std::pair<const int, const int> PH_VCAL_RANGE;
};
