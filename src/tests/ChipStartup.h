/*!
 * \file ChipStartup.h
 * \brief Definition of ChipStartup class.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 18-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#pragma once

#include "BasePixel/Test.h"
#include "psi/units.h"
#include "BasePixel/TBAnalogInterface.h"

class ChipStartup : public Test {
public:
    ChipStartup(boost::shared_ptr<TBAnalogInterface> tbInterface);
    virtual ~ChipStartup() {
        results->Fill();
    }

    void CheckCurrentsBeforeSetup();
    void CheckCurrentsAfterSetup();
private:
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    psi::ElectricCurrent IA_BeforeSetup, IA_AfterSetup, ID_BeforeSetup, ID_AfterSetup;
    psi::ElectricCurrent IA_BeforeSetup_HighLimit, ID_BeforeSetup_HighLimit, IA_AfterSetup_LowLimit,
        ID_AfterSetup_LowLimit, IA_AfterSetup_HighLimit, ID_AfterSetup_HighLimit;

};
