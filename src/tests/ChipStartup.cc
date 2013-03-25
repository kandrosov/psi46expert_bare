/*!
 * \file ChipStartup.cc
 * \brief Implementation of ChipStartup class.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 18-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#include "ChipStartup.h"
#include "BasePixel/ConfigParameters.h"

static const std::string LOG_HEAD = "ChipStartup";


ChipStartup::ChipStartup(boost::shared_ptr<TBAnalogInterface> _tbInterface)
    : Test("ChipStartup"), tbInterface(_tbInterface)
{
    results->Branch("IA_BeforeSetup", const_cast<double*>(&IA_BeforeSetup.value()));
    results->Branch("IA_AfterSetup", const_cast<double*>(&IA_AfterSetup.value()));
    results->Branch("ID_BeforeSetup", const_cast<double*>(&ID_BeforeSetup.value()));
    results->Branch("ID_AfterSetup", const_cast<double*>(&ID_AfterSetup.value()));

    params->Branch("IA_BeforeSetup_HighLimit", const_cast<double*>(&IA_BeforeSetup_HighLimit.value()));
    params->Branch("ID_BeforeSetup_HighLimit", const_cast<double*>(&ID_BeforeSetup_HighLimit.value()));
    params->Branch("IA_AfterSetup_LowLimit", const_cast<double*>(&IA_AfterSetup_LowLimit.value()));
    params->Branch("ID_AfterSetup_LowLimit", const_cast<double*>(&ID_AfterSetup_LowLimit.value()));
    params->Branch("IA_AfterSetup_HighLimit", const_cast<double*>(&IA_AfterSetup_HighLimit.value()));
    params->Branch("ID_AfterSetup_HighLimit", const_cast<double*>(&ID_AfterSetup_HighLimit.value()));

    const ConfigParameters& configParameters = ConfigParameters::Singleton();
    IA_BeforeSetup_HighLimit = configParameters.IA_BeforeSetup_HighLimit();
    ID_BeforeSetup_HighLimit = configParameters.ID_BeforeSetup_HighLimit();
    IA_AfterSetup_LowLimit = configParameters.IA_AfterSetup_LowLimit();
    ID_AfterSetup_LowLimit = configParameters.ID_AfterSetup_LowLimit();
    IA_AfterSetup_HighLimit = configParameters.IA_AfterSetup_HighLimit();
    ID_AfterSetup_HighLimit = configParameters.ID_AfterSetup_HighLimit();
    params->Fill();
}

void ChipStartup::CheckCurrentsBeforeSetup()
{
    IA_BeforeSetup = tbInterface->GetIA();
    ID_BeforeSetup = tbInterface->GetID();

    psi::LogInfo(LOG_HEAD) << "IA_BeforeSetup = " << IA_BeforeSetup << ", ID_BeforeSetup = "
                           << ID_BeforeSetup << "." << std::endl;

    if(IA_BeforeSetup > IA_BeforeSetup_HighLimit)
        THROW_PSI_EXCEPTION("IA before setup is too high. IA limit is "
                            << IA_BeforeSetup_HighLimit << ".");
    if(ID_BeforeSetup > ID_BeforeSetup_HighLimit)
        THROW_PSI_EXCEPTION("ID before setup is too high. ID limit is "
                            << ID_BeforeSetup_HighLimit << ".");
}

void ChipStartup::CheckCurrentsAfterSetup()
{
    IA_AfterSetup = tbInterface->GetIA();
    ID_AfterSetup = tbInterface->GetID();

    psi::LogInfo(LOG_HEAD) << "IA_AfterSetup = " << IA_AfterSetup << ", ID_AfterSetup = "
                           << ID_AfterSetup << "." << std::endl;

    if(IA_AfterSetup < IA_AfterSetup_LowLimit)
        THROW_PSI_EXCEPTION("IA after setup is too low. IA low limit is "
                            << IA_AfterSetup_LowLimit << ".");
    if(IA_AfterSetup > IA_AfterSetup_HighLimit)
        THROW_PSI_EXCEPTION("IA after setup is too high. IA limit is "
                            << IA_AfterSetup_HighLimit << ".");
    if(ID_AfterSetup < ID_AfterSetup_LowLimit)
        THROW_PSI_EXCEPTION("ID after setup is too low. ID low limit is "
                            << ID_AfterSetup_LowLimit << ".");
    if(ID_AfterSetup > ID_AfterSetup_HighLimit)
        THROW_PSI_EXCEPTION("ID after setup is too high. ID limit is "
                            << ID_AfterSetup_HighLimit << ".");
}
