/*!
 * \file BumpBounding.cc
 * \brief Implementation of BumpBounding class.
 */

#include "psi/log.h"

#include "BumpBonding.h"
#include "psi46expert/TestRoc.h"
#include "BasePixel/Analysis.h"
#include "BasePixel/ThresholdMap.h"
#include "BasePixel/TestParameters.h"

BumpBonding::BumpBonding(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface)
    : Test("BumpBonding", testRange), tbInterface(aTBInterface)
{
    nTrig = TestParameters::Singleton().BumpBondingNTrig();
}

void BumpBonding::RocAction(TestRoc& roc)
{
    ThresholdMap thresholdMap;

    SaveDacParameters(roc);
    roc.ClrCal();
    roc.Mask();
    roc.SetDAC(DACParameters::Vcal, 200);
    roc.SetDAC(DACParameters::CtrlReg, 4);
    tbInterface->Flush();

    TH2D* calXtalk = thresholdMap.GetMap("CalXTalkMap", roc, *testRange, 5);
    TH1D* calXtalkDistribution = Analysis::Distribution(calXtalk);
    vthrComp = static_cast<int>( calXtalkDistribution->GetMean() + 3. * calXtalkDistribution->GetRMS() );

    psi::LogDebug() << "[BumpBonding] Setting VthrComp to " << vthrComp << ".\n";

    roc.SetDAC(DACParameters::VthrComp, vthrComp);

    roc.Flush();

    TH2D* vcals = thresholdMap.GetMap("VcalsThresholdMap", roc, *testRange, nTrig);
    TH2D* xtalk = thresholdMap.GetMap("XTalkMap", roc, *testRange, nTrig);
    TH2D *difference = Analysis::DifferenceMap(vcals, xtalk, Form("vcals_xtalk_C%i", roc.GetChipId()));

    RestoreDacParameters(roc);

    histograms->Add(calXtalk);
    histograms->Add(vcals);
    histograms->Add(xtalk);
    histograms->Add(difference);

    histograms->Add(Analysis::Distribution(vcals));
    histograms->Add(Analysis::Distribution(xtalk));
    histograms->Add(Analysis::Distribution(difference));
    histograms->Add(calXtalkDistribution);
}
