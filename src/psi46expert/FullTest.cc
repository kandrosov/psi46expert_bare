/*!
 * \file FullTest.cc
 * \brief Implementation of FullTest class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new ConfigParameters class definition.
 *      - Adaptation for the new TestParameters class definition.
 */

#include "interface/Log.h"

#include "interface/Delay.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/ConfigParameters.h"
#include "FullTest.h"
#include "PixelAlive.h"
#include "BumpBonding.h"
#include "AddressLevels.h"
#include "AddressDecoding.h"
#include "Trim.h"
#include "SCurveTest.h"
#include "DacDependency.h"
#include "TrimBits.h"
#include "PHTest.h"
#include "ThresholdMap.h"
#include "TemperatureTest.h"
#include "TBMTest.h"
#include "AnalogReadout.h"

FullTest::FullTest(TestRange *aTestRange, TBInterface *aTBInterface, int opt)
{
  psi::Log<psi::Debug>() << "[FullTest] Initialization." << std::endl;
  testRange = aTestRange;
  tbInterface = aTBInterface;
  Scurve = opt;
}

void FullTest::ModuleAction()
{
	Test *test;

  if(Scurve != 0)
  {	
    psi::Log<psi::Info>() << "[FullTest] Start." << std::endl;

    //Log::Current()->printf("==>sv> Start FullTest\n");
    //Log::Current()->printf("==>sv> Start Test\n");
    gDelay->Timestamp();
  }
	
	DoTemperatureTest();
	for (int iTest = 0; iTest < 3; iTest++)
	{
    if(Scurve == 0)
    {
      test = new SCurveTest(testRange, tbInterface);
      test->ModuleAction(module);
      break;
    }

		gDelay->Timestamp();
        if (iTest == 0) test = new SCurveTest(testRange, tbInterface);
        if (iTest == 1 && !(ConfigParameters::Singleton().TbmEmulator())) test = new TBMTest(testRange, tbInterface);
		else if (iTest == 1) continue;
        if (iTest == 2) test = new AnalogReadout(testRange, tbInterface);
		test->ModuleAction(module);
		TIter next(test->GetHistos());
		while (TH1 *histo = (TH1*)next()) histograms->Add(histo);
	}

  if(Scurve != 0)
  {
    Test::ModuleAction();
    DoTemperatureTest();
    
    psi::Log<psi::Info>() << "[FullTest] End." << std::endl;
    // Log::Current()->printf("==>sv> End Test\n");
    // Log::Current()->printf("==>sv> End FullTest\n");
  }
}


void FullTest::RocAction()
{
  if(Scurve != 0)
  {
    psi::Log<psi::Debug>() << "[FullTest] Chip #" << chipId << '.' << std::endl;
    Test *test;

    histograms->Add(roc->DACHisto());
    
	for (int iTest = 0; iTest < 6; iTest++)
    {
      gDelay->Timestamp();
      if (iTest == 0) test = new PixelAlive(testRange, tbInterface);
      if (iTest == 1) test = new BumpBonding(testRange, tbInterface);
      if (iTest == 2) test = new TrimBits(testRange, tbInterface);
      if (iTest == 3) test = new TemperatureTest(testRange, tbInterface);
      if (iTest == 4) test = new AddressDecoding(testRange, tbInterface);
      if (iTest == 5) test = new AddressLevels(testRange, tbInterface);
      test->RocAction(roc);
      TIter next(test->GetHistos());
      while (TH1 *histo = (TH1*)next()) histograms->Add(histo);
    }

  //  	SaveDacParameters();
  //  	Log::Current()->printf("NoiseMap\n");
  //  	ThresholdMap *thresholdMap = new ThresholdMap();
  //  	TH2D *noiseMap = thresholdMap->GetMap("NoiseMap", roc, testRange, 10);
  //  	histograms->Add(noiseMap);
  //  	delete thresholdMap;
  //  	RestoreDacParameters();	

  //	Log::Current()->printf("FullTest  %i\n", chipId);
    psi::Log<psi::Debug>() << "[FullTest] done for chip " << chipId << '.'
                    << std::endl;
  }
}


void FullTest::DoTemperatureTest()
{
	gDelay->Timestamp();
  psi::Log<psi::Debug>() << "[FullTest] Temperature Test."<< std::endl;

    Test* test = new TemperatureTest(testRange, tbInterface);
	test->ModuleAction(module);
	TIter next(test->GetHistos());
	while (TH1 *histo = (TH1*)next()) histograms->Add(histo);
}
