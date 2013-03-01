/*!
 * \file BumpBounding.cc
 * \brief Implementation of BumpBounding class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#include "interface/Log.h"

#include "BumpBonding.h"
#include "TestRoc.h"
#include "Analysis.h"
#include "ThresholdMap.h"
#include "TestParameters.h"

BumpBonding::BumpBonding(TestRange *aTestRange, TBInterface *aTBInterface)
{
  psi::Log<psi::Debug>() << "[BumpBonding] Initialization." << std::endl;

	testRange = aTestRange;
	tbInterface = aTBInterface;
    ReadTestParameters();
}

void BumpBonding::ReadTestParameters()
{
    nTrig = TestParameters::Singleton().BumpBondingNTrig();
}

void BumpBonding::RocAction()
{
	ThresholdMap *thresholdMap = new ThresholdMap();
	
	SaveDacParameters();
	ClrCal();
	Mask();
	SetDAC("Vcal", 200);
	SetDAC("CtrlReg", 4);
	Flush();
	
	TH2D* calXtalk = thresholdMap->GetMap("CalXTalkMap", roc, testRange, 5);
	TH1D* calXtalkDistribution = gAnalysis->Distribution(calXtalk);
	vthrComp = static_cast<int>( calXtalkDistribution->GetMean() + 3. * calXtalkDistribution->GetRMS() );

  psi::Log<psi::Debug>() << "[BumpBonding] Setting VthrComp to " << vthrComp << '.'
                  << std::endl;

	SetDAC("VthrComp", vthrComp);

	Flush();
	
	TH2D* vcals = thresholdMap->GetMap("VcalsThresholdMap", roc, testRange, nTrig);
	TH2D* xtalk = thresholdMap->GetMap("XTalkMap", roc, testRange, nTrig);
	TH2D *difference = gAnalysis->DifferenceMap(vcals, xtalk, Form("vcals_xtalk_C%i", roc->GetChipId()));
	
	RestoreDacParameters();
	
	histograms->Add(calXtalk);
	histograms->Add(vcals);
	histograms->Add(xtalk);
	histograms->Add(difference);

	histograms->Add(gAnalysis->Distribution(calXtalk));
	histograms->Add(gAnalysis->Distribution(vcals));
	histograms->Add(gAnalysis->Distribution(xtalk));
	histograms->Add(gAnalysis->Distribution(difference));
	histograms->Add(calXtalkDistribution);
}
