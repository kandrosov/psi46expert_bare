/*!
 * \file TrimVcal.cc
 * \brief Implementation of TrimVcal class.
 *
 * \b Changelog
 * 02-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using psi::Sleep instead interface/Delay.
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using definitions from PsiCommon.h.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#include "psi/log.h"

#include "TrimVcal.h"
#include "psi46expert/TestRoc.h"
#include "BasePixel/constants.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/Analysis.h"
#include "BasePixel/ThresholdMap.h"
#include "BasePixel/TestParameters.h"

TrimVcal::TrimVcal(TestRange *aTestRange, TBInterface *aTBInterface)
{
	testRange = aTestRange;
	tbInterface = aTBInterface;
    ReadTestParameters();
	debug = true;
}

void TrimVcal::ReadTestParameters()
{
    nTrig = TestParameters::Singleton().TrimNTrig();
}

void TrimVcal::AddMap(TH2D* calMap)
{
	TH1D *distr = gAnalysis->Distribution(calMap, 255, 0., 255.);
	calMap->Write();
	distr->Write();
	histograms->Add(calMap);
	histograms->Add(distr);
}

void TrimVcal::RocAction()
{
  psi::LogInfo() << "[TrimVcal] Roc #" << chipId << ": Start." << std::endl;
  psi::LogInfo().PrintTimestamp();
	SaveDacParameters();
	
	roc->SetTrim(15);
	SetDAC("Vtrim", 0);
	SetDAC("Vcal", 200);
	((TBAnalogInterface*)tbInterface)->SetEnableAll(0);
	Flush();
// 	
// 	//Find good VthrComp
	thresholdMap = new ThresholdMap();
	TH2D *calMap = thresholdMap->GetMap("NoiseMap", roc, testRange, nTrig);
	AddMap(calMap);
	TH1D *distr = gAnalysis->Distribution(calMap, 255, 1., 254.);
	double thrMinLimit = TMath::Max(1., distr->GetMean() - 5.*distr->GetRMS());
	
	double thrMin = 255., thr;
    for (int i = 0; i < psi::ROCNUMCOLS; i++)
	{
        for (int k = 0; k < psi::ROCNUMROWS; k++)
		{
			if (testRange->IncludesPixel(roc->GetChipId(), i, k))
			{
				thr = calMap->GetBinContent(i+1, k+1);
				if ((thr < thrMin) && (thr > thrMinLimit)) thrMin = thr;
			}
		}
	}
	SetDAC("VthrComp", (int)thrMin - 10);
	Flush();

  psi::LogDebug() << "[TrimVcal] Minimum Threshold is " << thrMin << std::endl;
  psi::LogDebug() << "[TrimVcal] VtrhComp is set to "
                  << static_cast<int>( thrMin - 10) << std::endl;
	
// 	SetDAC("VthrComp", 120);
	SetDAC("Vtrim", 120);
	Flush();

    short trim[psi::ROCNUMCOLS*psi::ROCNUMROWS];
    for (int i = 0; i < psi::ROCNUMCOLS*psi::ROCNUMROWS; i++) trim[i] = 15;
	
	int mode = 2; //mode 0: no cal 1: cal in same column 2: cal in same doublecolumn	
	roc->TrimAboveNoise(10, 1, mode, trim); 
	
	//Determine Vcal
	
	((TBAnalogInterface*)tbInterface)->SetEnableAll(0);
    for (int i = 0; i < psi::ROCNUMCOLS; i++)
	{
        for (int k = 0; k < psi::ROCNUMROWS; k++)
		{
            roc->SetTrim(i, k, trim[i*psi::ROCNUMROWS + k]);
		}
	}
	
	thresholdMap->SetDoubleWbc();
	calMap = thresholdMap->GetMap("VcalThresholdMap", roc, testRange, nTrig);
	AddMap(calMap);
	distr = gAnalysis->Distribution(calMap, 255, 1., 254.);
	double vcalMaxLimit = TMath::Min(254., distr->GetMean() + 5.*distr->GetRMS());
		
	double vcalMin = 255., vcalMax = 0.;
	int thr255 = 0;
    for (int i = 0; i < psi::ROCNUMCOLS; i++)
	{
        for (int k = 0; k < psi::ROCNUMROWS; k++)
		{
			if (testRange->IncludesPixel(roc->GetChipId(), i, k))
			{
				thr = calMap->GetBinContent(i+1, k+1);
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

    RestoreDacParameters();
    psi::LogInfo().PrintTimestamp();
}

