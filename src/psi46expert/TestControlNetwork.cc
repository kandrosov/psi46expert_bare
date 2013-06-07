/*!
 * \file TestControlNetwork.cc
 * \brief Implementation of TestControlNetwork class.
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
#include "tests/ChipStartup.h"
#include "tests/DacProgramming.h"

static const std::string LOG_HEAD = "TestControlNetwork";

using namespace psi::control;

// Initializes the TestControlNetwork to a give configuration
TestControlNetwork::TestControlNetwork(boost::shared_ptr<TBAnalogInterface> aTBInterface,
                                       boost::shared_ptr<BiasVoltageController> aBiasVoltageController)
    : tbInterface(aTBInterface), biasVoltageController(aBiasVoltageController)
{
    ChipStartup chipStartupTest(tbInterface);
    chipStartupTest.CheckCurrentsBeforeSetup();
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
    chipStartupTest.CheckCurrentsAfterSetup();
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
    if(enable && !biasEnabled) {
        biasVoltageController->EnableBias();
        biasVoltageController->EnableControl();
    } else if(!enable && biasEnabled) {
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
    IVCurve test;
    test.ModuleAction();
}
void TestControlNetwork::Execute(const commands::TestDacProgramming&)
{
    for (unsigned i = 0; i < modules.size(); i++) {
        psi::tests::DacProgramming test(tbInterface, modules[i]->Rocs());
        test.ModuleAction();
    }
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
