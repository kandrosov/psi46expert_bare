/*!
 * \file ChipStartupTest.cc
 * \brief Implementation of ChipStartupTest class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new ConfigParameters class definition.
 *      - Adaptation for the new TestParameters class definition.
 * 22-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#include "interface/Log.h"

#include "ChipStartupTest.h"

const std::string ChipStartupTest::NAME = "chip_startup";

ChipStartupTest::ChipStartupTest(TestRange *aTestRange, TBAnalogInterface *aTBInterface)
{
  psi::LogDebug() << "[ChipStartupTest] Initialization." << psi::endl;
  testRange = aTestRange;
  tbInterface = aTBInterface;
  tbAnalogInterface = aTBInterface;
}

void ChipStartupTest::ModuleAction()
{
    psi::LogInfo() << "[ChipStartupTest] Start." << psi::endl;
    const double ia_off = tbAnalogInterface->GetIA();
    const double id_off = tbAnalogInterface->GetID();
    psi::LogInfo() << "IA_off = " << ia_off << ", ID_off = " << id_off << "." << psi::endl;
    tbAnalogInterface->SetEnableAll(1);
    const double ia_on = tbAnalogInterface->GetIA();
    const double id_on = tbAnalogInterface->GetID();
    psi::LogInfo() << "IA_on = " << ia_on << ", ID_on = " << id_on << "." << psi::endl;
    psi::LogInfo() << "[ChipStartupTest] End." << psi::endl;
}
