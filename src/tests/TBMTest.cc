/*!
 * \file TBMTest.cc
 * \brief Implementation of TBMTest class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new ConfigParameters class definition.
 *      - Adaptation for the new TestParameters class definition.
 */

#include "psi/log.h"

#include "TBMTest.h"
#include "psi46expert/TestModule.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/ConfigParameters.h"
#include <TParameter.h>


// e5 f5

TBMTest::TBMTest(TestRange *aTestRange, TBInterface *aTBInterface)
{
  psi::Log<psi::Debug>() << "[TBMTest] Initialization." << std::endl;

	testRange = aTestRange;
	tbInterface = aTBInterface;
	debug = false;
}

void TBMTest::ModuleAction()
{
    const ConfigParameters& configParameters = ConfigParameters::Singleton();

	result[0] = 0;
	result[1] = 0;

	if (!tbInterface->TBMIsPresent()) return;
    if (configParameters.HalfModule() == 0) DualModeTest();
	ReadoutTest();
	
	TParameter<int> *parameter0 = new TParameter<int>("TBM1", result[0]);
	TParameter<int> *parameter1 = new TParameter<int>("TBM2", result[1]);
	parameter0->Write();
	parameter1->Write();
}


void TBMTest::ReadoutTest()
{
	TBM *tbm = module->GetTBM();
	
	int value;
	bool res = tbm->GetReg(229, value);
	if (debug) printf("value %i\n", value);

	if (!res)
	{
		result[0] += 4;
    psi::Log<psi::Info>() << "[TBMTest] Error: TBM1 readout test failed." << std::endl;
	}
	
	res = tbm->GetReg(245, value);
	if (debug) printf("value2 %i\n", value);

	if (!res)
	{
		result[1] += 4;
    psi::Log<psi::Info>() << "[TBMTest] Error: TBM2 readout test failed." << std::endl;
	}
	
		
}

void TBMTest::DualModeTest()
{
	TBM *tbm = module->GetTBM();
	TBAnalogInterface *anaInterface = (TBAnalogInterface*)tbInterface;
    const ConfigParameters& configParameters = ConfigParameters::Singleton();
	
	unsigned short count;
	short data[10000];

  psi::Log<psi::Info>() << "[TBMTest] Start." << std::endl;

	int channel = anaInterface->GetTBMChannel();
	int singleDual = tbm->GetDAC(0);
	
    int dtlOrig = configParameters.DataTriggerLevel(), dtl;
	anaInterface->DataTriggerLevel(dtl);
	
	for (int k = 0; k < 2; k++)
	{
		module->SetTBMSingle(k);
		anaInterface->SetTBMChannel(k);
		
		dtl = dtlOrig;
		do
		{
			anaInterface->DataTriggerLevel(dtl);
			anaInterface->Flush();
			anaInterface->ADCData(data, count);
			dtl+=50;
		}
		while((count != anaInterface->GetEmptyReadoutLengthADC()) && (dtl < 0));
		
		if (count != anaInterface->GetEmptyReadoutLengthADC())
		{
			result[k] += 1;

      psi::Log<psi::Info>() << "[TBMTest] Error: test failed for TBM #" << k
                     << ". No valid analog readout." << std::endl;

			if (k == 1)
        psi::Log<psi::Info>() << "[TBMTest] Error: test failed. Does testboard "
                       << "have 2 ADCs?" << std::endl;

			for (int i = 0; i < count; i++) printf(" %i", data[i]);
			printf("\n");
		}
			
		tbm->setDualMode();
		
		dtl = dtlOrig;
		do
		{
			anaInterface->DataTriggerLevel(dtl);
			anaInterface->Flush();
			anaInterface->ADCData(data, count);
			dtl+=50;
		}
		while((count != anaInterface->GetEmptyReadoutLengthADCDual()) && (dtl < 0));
	
		if (count != anaInterface->GetEmptyReadoutLengthADCDual()) 
		{
			result[k] += 2;

      psi::Log<psi::Info>() << "[TBMTest] Error: test failed for TBM #" << k
                     << ". No valid dual analog readout." << std::endl;

			if (k == 1)
        psi::Log<psi::Info>() << "[TBMTest] Error: test failed. Does testboard "
                       << "have 2 ADCs?" << std::endl;

			for (int i = 0; i < count; i++) printf(" %i", data[i]);
			printf("\n");
		}
	}

	anaInterface->SetTBMChannel(channel);
	tbm->SetDAC(0, singleDual);
	anaInterface->DataTriggerLevel(dtlOrig);
	Flush();
}