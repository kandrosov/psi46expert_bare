/*!
 * \file TrimVcal.cc
 * \brief Implementation of TrimVcal class.
 */

#include "psi/log.h"

#include "TrimVcal.h"
#include "psi46expert/TestRoc.h"
#include "BasePixel/constants.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/Analysis.h"
#include "BasePixel/ThresholdMap.h"
#include "BasePixel/TestParameters.h"

TrimVcal::TrimVcal(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface)
    : Test("TrimVcal", testRange), tbInterface(aTBInterface)
{
    nTrig = TestParameters::Singleton().TrimNTrig();
    debug = true;
}

void TrimVcal::AddMap(TH2D* calMap)
{
    TH1D *distr = Analysis::Distribution(calMap, 255, 0., 255.);
    histograms->Add(calMap);
    histograms->Add(distr);
}

void TrimVcal::RocAction(TestRoc& roc)
{
    psi::LogInfo() << "[TrimVcal] Roc #" << roc.GetChipId() << ": Start." << std::endl;
    SaveDacParameters(roc);

    roc.SetTrim(15);
    roc.SetDAC(DACParameters::Vtrim, 0);
    roc.SetDAC(DACParameters::Vcal, 200);
    tbInterface->SetEnableAll(0);
    tbInterface->Flush();
//
// 	//Find good VthrComp
    TH2D *calMap = thresholdMap.MeasureMap(ThresholdMap::NoiseMapParameters, roc, *testRange, nTrig);
    AddMap(calMap);
    TH1D *distr = Analysis::Distribution(calMap, 255, 1., 254.);
    double thrMinLimit = TMath::Max(1., distr->GetMean() - 5.*distr->GetRMS());

    double thrMin = 255., thr;
    for (unsigned i = 0; i < psi::ROCNUMCOLS; i++) {
        for (unsigned k = 0; k < psi::ROCNUMROWS; k++) {
            if (testRange->IncludesPixel(roc.GetChipId(), i, k)) {
                thr = calMap->GetBinContent(i + 1, k + 1);
                if ((thr < thrMin) && (thr > thrMinLimit)) thrMin = thr;
            }
        }
    }
    roc.SetDAC(DACParameters::VthrComp, (int)thrMin - 10);
    tbInterface->Flush();

    psi::LogDebug() << "[TrimVcal] Minimum Threshold is " << thrMin << std::endl;
    psi::LogDebug() << "[TrimVcal] VtrhComp is set to "
                    << static_cast<int>( thrMin - 10) << std::endl;

// 	SetDAC("VthrComp", 120);
    roc.SetDAC(DACParameters::Vtrim, 120);
    tbInterface->Flush();

    short trim[psi::ROCNUMCOLS * psi::ROCNUMROWS];
    for (unsigned i = 0; i < psi::ROCNUMCOLS * psi::ROCNUMROWS; i++) trim[i] = 15;

    int mode = 2; //mode 0: no cal 1: cal in same column 2: cal in same doublecolumn
    roc.TrimAboveNoise(10, 1, mode, trim);

    //Determine Vcal

    tbInterface->SetEnableAll(0);
    for (unsigned i = 0; i < psi::ROCNUMCOLS; i++) {
        for (unsigned k = 0; k < psi::ROCNUMROWS; k++) {
            roc.SetTrim(i, k, trim[i * psi::ROCNUMROWS + k]);
        }
    }

    thresholdMap.SetDoubleWbc();
    calMap = thresholdMap.MeasureMap(ThresholdMap::VcalThresholdMapParameters, roc, *testRange, nTrig);
    AddMap(calMap);
    distr = Analysis::Distribution(calMap, 255, 1., 254.);
    double vcalMaxLimit = TMath::Min(254., distr->GetMean() + 5.*distr->GetRMS());

    double vcalMin = 255., vcalMax = 0.;
    int thr255 = 0;
    for (unsigned i = 0; i < psi::ROCNUMCOLS; i++) {
        for (unsigned k = 0; k < psi::ROCNUMROWS; k++) {
            if (testRange->IncludesPixel(roc.GetChipId(), i, k)) {
                thr = calMap->GetBinContent(i + 1, k + 1);
                if ((thr > vcalMax) && (thr < vcalMaxLimit)) vcalMax = thr;
                if ((thr < vcalMin) && (thr > 1.)) vcalMin = thr;
                if (thr == 255.) thr255++;
            }
        }
    }

    psi::LogDebug() << "[TrimVcal] There are " << thr255 << " pixels with "
                    << "Vcal 255." << std::endl;
    psi::LogDebug() << "[TrimVcal] Vcal range is [ " << vcalMin << ", "
                    << vcalMax << "]." << std::endl;

    RestoreDacParameters(roc);
}
