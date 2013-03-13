/*!
 * \file TestControlNetwork.cc
 * \brief Implementation of TestControlNetwork class.
 *
 * \b Changelog
 * 07-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Added support for the control commands.
 *      - TestControlNetwork moved into psi::control namespace
 * 04-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - The startup current checks moved into TestControlNetwork constructor.
 * 01-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Class SysCommand removed.
 * 26-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Preparations for the further multithread support.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new ConfigParameters class definition.
 *      - MainFrame removed due to compability issues.
 *      - Adaptation for the new TestParameters class definition.
 */

#include "TestControlNetwork.h"
#include "tests/IVCurve.h"
#include "BasePixel/RawPacketDecoder.h"
#include "BasePixel/DecoderCalibration.h"
#include "BasePixel/TBAnalogInterface.h"
#include <TApplication.h>
#include <TSystem.h>

#include "BasePixel/TestParameters.h"
#include "BasePixel/DataStorage.h"

static const std::string LOG_HEAD = "TestControlNetwork";

using namespace psi::control;

// Initializes the TestControlNetwork to a give configuration
TestControlNetwork::TestControlNetwork(boost::shared_ptr<TBAnalogInterface> aTBInterface,
                                       boost::shared_ptr<BiasVoltageController> aBiasVoltageController)
    : tbInterface(aTBInterface), biasVoltageController(aBiasVoltageController)
{
    CheckCurrentsBeforeSetup();
    RawPacketDecoder *gDecoder = RawPacketDecoder::Singleton();
    const ConfigParameters& configParameters = ConfigParameters::Singleton();
    TestParameters& testParameters = TestParameters::ModifiableSingleton();
    testParameters.Read(configParameters.FullTestParametersFileName());
    tbInterface = aTBInterface;
    const unsigned nModules = configParameters.NumberOfModules();

    for (unsigned i = 0; i < nModules; i++)
        modules.push_back( boost::shared_ptr<TestModule>(new TestModule(0, tbInterface)));

    TString fileName = TString(configParameters.Directory()).Append("/addressParameters.dat");
    psi::LogInfo() << "Reading Address Level-Parameters from " << fileName << std::endl;
    //DecoderCalibrationModule* decoderCalibrationModule = new DecoderCalibrationModule(fileName, 3, 0, NUM_ROCSMODULE);
    DecoderCalibrationModule* decoderCalibrationModule = new DecoderCalibrationModule(fileName, 3, 0, configParameters.NumberOfRocs());
    std::ostringstream ss;
    decoderCalibrationModule->Print(&ss);
    psi::LogInfo() << ss.str();
    gDecoder->SetCalibration(decoderCalibrationModule);

    Initialize();
    CheckCurrentsAfterSetup();
}

void TestControlNetwork::Initialize()
{
    for (unsigned i = 0; i < modules.size(); i++)
        modules[i]->Initialize();
}

void TestControlNetwork::Execute(const commands::Bias& bias)
{
    const bool enable = bias.getData().Enable();
    const bool biasEnabled = biasVoltageController->BiasEnabled();
    if(enable && !biasEnabled)
    {
        biasVoltageController->EnableBias();
        biasVoltageController->EnableControl();
    }
    else if(!enable && biasEnabled)
    {
        biasVoltageController->DisableControl();
        biasVoltageController->DisableBias();
    }
}

void TestControlNetwork::Execute(const commands::FullTest&)
{
    for (unsigned i = 0; i < modules.size(); i++)
        modules[i]->FullTestAndCalibration();
}

void TestControlNetwork::Execute(const commands::IV&)
{
    boost::scoped_ptr<IVCurve> ivCurve(new IVCurve(0, tbInterface.get()));
    ivCurve->ModuleAction();
}

void TestControlNetwork::ShortTestAndCalibration()
{
    for (unsigned i = 0; i < modules.size(); i++)
        modules[i]->ShortTestAndCalibration();
}

void TestControlNetwork::ShortCalibration()
{
    for (unsigned i = 0; i < modules.size(); i++)
        modules[i]->ShortCalibration();
}

// Tries to automatically adjust Vana
void TestControlNetwork::AdjustVana()
{
    for (unsigned i = 0; i < modules.size(); i++)
        modules[i]->AdjustVana();
}


void TestControlNetwork::AdjustDACParameters()
{
    for (unsigned i = 0; i < modules.size(); i++)
        modules[i]->AdjustDACParameters();
}

void TestControlNetwork::CheckCurrentsBeforeSetup()
{
    const ConfigParameters& configParameters = ConfigParameters::Singleton();
    const psi::ElectricCurrent ia_before_setup = tbInterface->GetIA();
    const psi::ElectricCurrent id_before_setup = tbInterface->GetID();

    psi::LogInfo(LOG_HEAD) << "IA_before_setup = " << ia_before_setup << ", ID_before_setup = "
                           << id_before_setup << "." << std::endl;
    psi::DataStorage::Active().SaveMeasurement("ia_before_setup", ia_before_setup);
    psi::DataStorage::Active().SaveMeasurement("id_before_setup", id_before_setup);

    if(ia_before_setup > configParameters.IA_BeforeSetup_HighLimit())
        THROW_PSI_EXCEPTION("IA before setup is too high. IA limit is "
                            << configParameters.IA_BeforeSetup_HighLimit() << ".");
    if(id_before_setup > configParameters.ID_BeforeSetup_HighLimit())
        THROW_PSI_EXCEPTION("ID before setup is too high. ID limit is "
                            << configParameters.ID_BeforeSetup_HighLimit() << ".");
}

void TestControlNetwork::CheckCurrentsAfterSetup()
{
    const ConfigParameters& configParameters = ConfigParameters::Singleton();
    const psi::ElectricCurrent ia_after_setup = tbInterface->GetIA();
    const psi::ElectricCurrent id_after_setup = tbInterface->GetID();

    psi::LogInfo(LOG_HEAD) << "IA_after_setup = " << ia_after_setup << ", ID_after_setup = "
                           << id_after_setup << "." << std::endl;
    psi::DataStorage::Active().SaveMeasurement("ia_after_setup", ia_after_setup);
    psi::DataStorage::Active().SaveMeasurement("id_after_setup", id_after_setup);

    if(ia_after_setup < configParameters.IA_AfterSetup_LowLimit())
        THROW_PSI_EXCEPTION("IA after setup is too low. IA low limit is "
                            << configParameters.IA_AfterSetup_LowLimit() << ".");
    if(ia_after_setup > configParameters.IA_AfterSetup_HighLimit())
        THROW_PSI_EXCEPTION("IA after setup is too high. IA limit is "
                            << configParameters.IA_AfterSetup_HighLimit() << ".");
    if(id_after_setup < configParameters.ID_AfterSetup_LowLimit())
        THROW_PSI_EXCEPTION("ID after setup is too low. ID low limit is "
                            << configParameters.ID_AfterSetup_LowLimit() << ".");
    if(id_after_setup > configParameters.ID_AfterSetup_HighLimit())
        THROW_PSI_EXCEPTION("ID after setup is too high. ID limit is "
                            << configParameters.ID_AfterSetup_HighLimit() << ".");
}
