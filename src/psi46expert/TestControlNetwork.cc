/*!
 * \file TestControlNetwork.cc
 * \brief Implementation of TestControlNetwork class.
 *
 * \b Changelog
 * 26-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Preparations for the further multithread support.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new ConfigParameters class definition.
 *      - MainFrame removed due to compability issues.
 *      - Adaptation for the new TestParameters class definition.
 */

#include "TestControlNetwork.h"
#include "IVCurve.h"
#include "BasePixel/RawPacketDecoder.h"
#include "BasePixel/DecoderCalibration.h"
#include "BasePixel/TBAnalogInterface.h"
#include <TApplication.h>
#include <TSystem.h>
#include <iostream>
#include "TestParameters.h"
using namespace DecoderCalibrationConstants;
using namespace DecodedReadoutConstants;

// Initializes the TestControlNetwork to a give configuration
TestControlNetwork::TestControlNetwork(TBInterface *aTBInterface)
{
	RawPacketDecoder *gDecoder = RawPacketDecoder::Singleton();
    const ConfigParameters& configParameters = ConfigParameters::Singleton();
    TestParameters& testParameters = TestParameters::ModifiableSingleton();
    testParameters.Read(configParameters.FullTestParametersFileName());
	tbInterface = aTBInterface;
    const unsigned nModules = configParameters.NumberOfModules();

    for (unsigned i = 0; i < nModules; i++)
        modules.push_back( boost::shared_ptr<TestModule>(new TestModule(0, tbInterface)));

    TString fileName = TString(configParameters.Directory()).Append("/addressParameters.dat");
    std::cout << "Reading Address Level-Parameters from " << fileName << std::endl;
	//DecoderCalibrationModule* decoderCalibrationModule = new DecoderCalibrationModule(fileName, 3, 0, NUM_ROCSMODULE);
    DecoderCalibrationModule* decoderCalibrationModule = new DecoderCalibrationModule(fileName, 3, 0, configParameters.NumberOfRocs());
    decoderCalibrationModule->Print(&std::cout);
	gDecoder->SetCalibration(decoderCalibrationModule);

	Initialize();
}

void TestControlNetwork::Initialize()
{
    for (unsigned i = 0; i < modules.size(); i++)
        modules[i]->Initialize();
}

void TestControlNetwork::DoIV()
{
// dirty solution for a not understood problem
// from gui: IV Test works well, steps take 5s
// from commandline: IV Test slow, steps take 30s
// code is identical, gui framework somehow influences the read command in Keithley.cc


/* 	new TApplication("App",0,0, 0, -1);
	MainFrame* mf = new MainFrame(gClient->GetRoot(), 400, 400, tbInterface, this, configParameters, false);
	mf->Connect("IV()", "MainFrame", mf, "IV()");
    mf->Emit("IV()");*/
    boost::scoped_ptr<IVCurve> ivCurve(new IVCurve(0, tbInterface));
    ivCurve->ModuleAction();
}


void TestControlNetwork::FullTestAndCalibration()
{
    for (unsigned i = 0; i < modules.size(); i++)
        modules[i]->FullTestAndCalibration();
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

void TestControlNetwork::Execute(SysCommand &command)
{
	if (command.Keyword("IV")) {DoIV();}
    else modules[command.module]->Execute(command);
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
