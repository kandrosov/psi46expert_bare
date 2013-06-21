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

namespace {
const std::string TEST_NAME = "BumpBonding";
}

BumpBonding::BumpBonding(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface)
    : Test(TEST_NAME, testRange), tbInterface(aTBInterface)
{
    const TestParameters& testParameters = TestParameters::Singleton();
    thrLevel = testParameters.BumpBondingThrLevel();
    nTrig = testParameters.BumpBondingNTrig();
    calXTalkThrLevel = testParameters.BumpBondingCalXTalkThrLevel();
    calXTalkNTrig = testParameters.BumpBondingCalXTalkNTrig();
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

    TH2D* calXtalk = thresholdMap.MeasureMap(ThresholdMap::CalXTalkMapParameters, roc, *testRange,
                                             calXTalkThrLevel, calXTalkNTrig, 0);
    TH1D* calXtalkDistribution = Analysis::Distribution(calXtalk);
    vthrComp = static_cast<int>( calXtalkDistribution->GetMean() + 3. * calXtalkDistribution->GetRMS() );

    psi::LogInfo(TEST_NAME) << "Setting VthrComp to " << vthrComp << ".\n";

    roc.SetDAC(DACParameters::VthrComp, vthrComp);
    tbInterface->Flush();

    TH2D* vcals = thresholdMap.MeasureMap(ThresholdMap::VcalsThresholdMapParameters, roc, *testRange, thrLevel, nTrig,
                                          0);
    TH2D* xtalk = thresholdMap.MeasureMap(ThresholdMap::XTalkMapParameters, roc, *testRange, thrLevel, nTrig, 0);
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
