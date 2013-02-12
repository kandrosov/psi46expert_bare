/*!
 * \file TestControlNetwork.cc
 * \brief Implementation of TestControlNetwork class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new ConfigParameters class definition.
 *      - MainFrame removed due to compability issues.
 */

#include "TestControlNetwork.h"
#include "IVCurve.h"
#include "BasePixel/RawPacketDecoder.h"
#include "BasePixel/DecoderCalibration.h"
#include "BasePixel/TBAnalogInterface.h"
#include <TApplication.h>
#include <TSystem.h>
#include <iostream>

using namespace DecoderCalibrationConstants;
using namespace DecodedReadoutConstants;

// Initializes the TestControlNetwork to a give configuration
TestControlNetwork::TestControlNetwork(TBInterface *aTBInterface)
{
	RawPacketDecoder *gDecoder = RawPacketDecoder::Singleton();
    const ConfigParameters& configParameters = ConfigParameters::Singleton();
    testParameters = new TestParameters(configParameters.TestParametersFileName().c_str());
	tbInterface = aTBInterface;
    nModules = configParameters.NumberOfModules();

	for (int i = 0; i < nModules; i++)
	{
        module[i] = new TestModule(0, tbInterface, testParameters);
	}

    TString fileName = TString(configParameters.Directory()).Append("/addressParameters.dat");
	cout << "Reading Address Level-Parameters from " << fileName << endl;
	//DecoderCalibrationModule* decoderCalibrationModule = new DecoderCalibrationModule(fileName, 3, 0, NUM_ROCSMODULE);
    DecoderCalibrationModule* decoderCalibrationModule = new DecoderCalibrationModule(fileName, 3, 0, configParameters.NumberOfRocs());
	decoderCalibrationModule->Print(&cout);
	gDecoder->SetCalibration(decoderCalibrationModule);

	Initialize();
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
}


void TestControlNetwork::FullTestAndCalibration()
{
	for (int i = 0; i < nModules; i++) GetModule(i)->FullTestAndCalibration();
}


void TestControlNetwork::ShortTestAndCalibration()
{
	for (int i = 0; i < nModules; i++) GetModule(i)->ShortTestAndCalibration();
}


void TestControlNetwork::ShortCalibration()
{
	for (int i = 0; i < nModules; i++) GetModule(i)->ShortCalibration();
}


TestParameters *TestControlNetwork::GetTestParameters()
{
	return testParameters;
}


TestModule* TestControlNetwork::GetModule(int iModule)
{
	return (TestModule*)module[iModule];
}


void TestControlNetwork::Execute(SysCommand &command)
{
	if (command.Keyword("IV")) {DoIV();}
	else GetModule(command.module)->Execute(command);
}


// Tries to automatically adjust Vana
void TestControlNetwork::AdjustVana()
{
	for (int i = 0; i < nModules; i++)
	{
		GetModule(i)->AdjustVana();
	}
}


void TestControlNetwork::AdjustDACParameters()
{
	for (int i = 0; i < nModules; i++)
	{
		GetModule(i)->AdjustDACParameters();
	}
}             
