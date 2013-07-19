/*!
 * \file Trim.cc
 * \brief Implementation of Trim class.
 */

#include "psi/log.h"

#include "Trim.h"
#include "psi46expert/TestRoc.h"
#include "BasePixel/constants.h"
#include "BasePixel/TBAnalogInterface.h"
#include "analysis/Analysis.h"
#include "BasePixel/ThresholdMap.h"
#include "BasePixel/TestParameters.h"

Trim::Trim(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface)
    : Test("Trim", testRange), tbInterface(aTBInterface), numberOfVcalThresholdMaps(0), numberOfTrimMaps(0)
{
    const TestParameters& testParameters = TestParameters::Singleton();
    doubleWbc = testParameters.TrimDoubleWbc();
    nTrig = testParameters.TrimNTrig();
    vcal = testParameters.TrimVcal();
}

void Trim::AddMap(TH2D* calMap)
{
    TH1D *distr = Analysis::Distribution(calMap, 255, 0., 255.);
    histograms->Add(calMap);
    histograms->Add(distr);
}

void Trim::RocAction(TestRoc& roc)
{
    TestPixel *maxPixel = 0;
    double thrMax, thr, thrMin;
    TH2D *calMap;

    if (doubleWbc) thresholdMap.SetDoubleWbc();

    psi::LogInfo() << "[Trim] Roc #" << roc.GetChipId() << ": Start." << std::endl;
    SaveDacParameters(roc);

    roc.SetTrim(15);
    roc.SetDAC(DACParameters::Vtrim, 0);

    psi::LogDebug() << "[Trim] Setting Vcal to " << vcal << std::endl;

    roc.SetDAC(DACParameters::Vcal, vcal);
    roc.Flush();

    //Find good VthrComp
    calMap = thresholdMap.MeasureMap(ThresholdMap::CalThresholdMapParameters, roc, *testRange, nTrig);
    AddMap(calMap);
    TH1D *distr = Analysis::Distribution(calMap, 255, 1., 254., 2);
    double mean = distr->GetMean();
    double rms = distr->GetRMS();
    double thrMinLimit = TMath::Max(1., mean - 5.*rms);

    thrMin = 255.;
    thrMax = 0.;
    int thr255 = 0;
    for (unsigned i = 0; i < psi::ROCNUMCOLS; i++) {
        for (unsigned k = 0; k < psi::ROCNUMROWS; k++) {
            if (testRange->IncludesPixel(roc.GetChipId(), i, k)) {
                thr = calMap->GetBinContent(i + 1, k + 1);
                if ((thr > thrMax) && (thr < 255.)) thrMax = thr;
                if ((thr < thrMin) && (thr > thrMinLimit)) thrMin = thr;
                if (thr == 255.) thr255++;
            }
        }
    }
    psi::LogDebug() << "[Trim] There are " << thr255 << " pixels with "
                    << "threshold 255." << std::endl;
    psi::LogDebug() << "[Trim] Theshold range is [ " << thrMin << ", "
                    << thrMax << "]." << std::endl;

    if (thrMax == 0.) {
        psi::LogInfo() << "[Trim] Error: Can not find maximum threshold."
                       << std::endl;

        return;
    }

    roc.SetDAC(DACParameters::VthrComp, (int)thrMin);
    psi::LogDebug() << "[Trim] VthrComp is set to " << static_cast<int>( thrMin)
                    << std::endl;
    roc.Flush();

    //Determine minimal and maximal thresholds
    calMap = thresholdMap.MeasureMap(ThresholdMap::VcalThresholdMapParameters, roc, *testRange, nTrig,
                                     ++numberOfVcalThresholdMaps);
    AddMap(calMap);
    distr = Analysis::Distribution(calMap, 255, 1., 254., 2);
    mean = distr->GetMean();
    rms = distr->GetRMS();
    double vcalMaxLimit = TMath::Min(254., mean + 5.*rms);

    double vcalMin = 255.;
    double vcalMax = 0.;
    thr255 = 0;
    for (unsigned i = 0; i < psi::ROCNUMCOLS; i++) {
        for (unsigned k = 0; k < psi::ROCNUMROWS; k++) {
            if (testRange->IncludesPixel(roc.GetChipId(), i, k)) {
                thr = calMap->GetBinContent(i + 1, k + 1);
                if ((thr > vcalMax) && (thr < vcalMaxLimit)) {
                    vcalMax = thr;
                    maxPixel = &roc.GetPixel(i, k);
                }
                if ((thr < vcalMin) && (thr > 1.)) vcalMin = thr;
                if (thr == 255.) thr255++;
            }
        }
    }

    psi::LogDebug() << "[Trim] There are " << thr255 << " pixels with "
                    << "Vcal 255." << std::endl;
    psi::LogDebug() << "[Trim] Vcal range is [ " << vcalMin << ", "
                    << vcalMax << "]." << std::endl;

    if (vcalMax == 0) {
        psi::LogInfo() << "[Trim] Error: Vcal max = 0. Abort test." << std::endl;

        return;
    }

    //Determine Vtrim
    roc.EnableDoubleColumn(maxPixel->GetColumn());
    int vtrim = AdjustVtrim(*maxPixel);
    roc.DisableDoubleColumn(maxPixel->GetColumn());

    roc.SetTrim(7);
    calMap = thresholdMap.MeasureMap(ThresholdMap::VcalThresholdMapParameters, roc, *testRange, nTrig,
                                     ++numberOfVcalThresholdMaps);
    AddMap(calMap);

    calMap = TrimStep(roc, 4, calMap);
    calMap = TrimStep(roc, 2, calMap);
    calMap = TrimStep(roc, 1, calMap);
    calMap = TrimStep(roc, 1, calMap);

    calMap = thresholdMap.MeasureMap(ThresholdMap::VcalThresholdMapParameters, roc, *testRange, nTrig,
                                     ++numberOfVcalThresholdMaps);
    AddMap(calMap);

    RestoreDacParameters(roc);

    roc.SetDAC(DACParameters::Vtrim, vtrim);
    roc.SetDAC(DACParameters::VthrComp, (int)thrMin);

    const ConfigParameters& configParameters = ConfigParameters::Singleton();
    char dacFileName[100], trimFileName[100];

    //writing files

    char dacParametersFileName[100];
    strcpy(dacParametersFileName, configParameters.FullDacParametersFileName().c_str());
    int length = strlen(dacParametersFileName);
    if (strstr(dacParametersFileName, ".dat")) {
        sprintf(dacFileName, "%s%i.dat", strncpy(dacParametersFileName, dacParametersFileName, length - 4), vcal);
    } else {
        sprintf(dacFileName, "%s%i_C%i.dat", strncpy(dacParametersFileName, dacParametersFileName, length - 4), vcal,
                roc.GetChipId());
    }
    roc.WriteDACParameterFile(dacFileName);

    char trimParametersFileName[100];
    strcpy(trimParametersFileName, configParameters.FullTrimParametersFileName().c_str());
    length = strlen(dacParametersFileName);
    if (strstr(trimParametersFileName, ".dat")) {
        sprintf(trimFileName, "%s%i.dat", strncpy(trimParametersFileName, trimParametersFileName, length - 4), vcal);
    } else {
        sprintf(trimFileName, "%s%i_C%i.dat", strncpy(trimParametersFileName, trimParametersFileName, length - 4), vcal,
                roc.GetChipId());
    }
    roc.WriteTrimConfiguration(trimFileName);
}


TH2D* Trim::TrimStep(TestRoc& roc, int correction, TH2D *calMapOld)
{
    TH2D* betterCalMap = CreateMap("VcalThresholdMap", roc.GetChipId(), ++numberOfVcalThresholdMaps);
    int trim;

    //save trim map
    TH2D *trimMap = roc.TrimMap(++numberOfTrimMaps);

    //set new trim bits
    for (unsigned i = 0; i < psi::ROCNUMCOLS; i++) {
        for (unsigned k = 0; k < psi::ROCNUMROWS; k++) {
            if (testRange->IncludesPixel(roc.GetChipId(), i, k)) {
                trim = (int)trimMap->GetBinContent(i + 1, k + 1);

                if (calMapOld->GetBinContent(i + 1, k + 1) > vcal) trim -= correction;
                else trim += correction;

                if (trim < 0) trim = 0;
                if (trim > 15) trim = 15;
                roc.GetPixel(i, k).SetTrim(trim);
            }
        }
    }
    AddMap(roc.TrimMap(++numberOfTrimMaps));

    //measure new result
    TH2D *calMap = thresholdMap.MeasureMap(ThresholdMap::VcalThresholdMapParameters, roc, *testRange, nTrig,
                                           ++numberOfVcalThresholdMaps);
    AddMap(calMap);

    // test if the result got better
    for (unsigned i = 0; i < psi::ROCNUMCOLS; i++) {
        for (unsigned k = 0; k < psi::ROCNUMROWS; k++) {
            if (testRange->IncludesPixel(roc.GetChipId(), i, k)) {
                trim = roc.GetPixel(i, k).GetTrim();

                if (TMath::Abs(calMap->GetBinContent(i + 1, k + 1) - vcal) <=
                        TMath::Abs(calMapOld->GetBinContent(i + 1, k + 1) - vcal)) {
                    // it's better now
                    betterCalMap->SetBinContent(i + 1, k + 1, calMap->GetBinContent(i + 1, k + 1));
                } else {
                    // it's worse
                    betterCalMap->SetBinContent(i + 1, k + 1, calMapOld->GetBinContent(i + 1, k + 1));
                    roc.GetPixel(i, k).SetTrim((int)trimMap->GetBinContent(i + 1, k + 1));
                }
            }
        }
    }

    AddMap(roc.TrimMap(++numberOfTrimMaps));

    return betterCalMap;
}


int Trim::AdjustVtrim(TestPixel& pixel)
{
    int vtrim = -1;
    int thr = 255, thrOld;
    int wbc = pixel.GetRoc().GetDAC(DACParameters::WBC);
    psi::LogInfo() << "Adjust Vtrim col " << pixel.GetColumn() << ", row " << pixel.GetRow() << std::endl;
    do {
        vtrim++;
        pixel.GetRoc().SetDAC(DACParameters::Vtrim, vtrim);
        tbInterface->Flush();
        thrOld = thr;
        thr = pixel.GetRoc().PixelThreshold(pixel.GetColumn(), pixel.GetRow(), 0, 1, nTrig, 2 * nTrig, 25, false, false,
                                            0);
        if (debug)
            psi::LogInfo() << "thr " << thr << std::endl;
        if (doubleWbc) {
            pixel.GetRoc().SetDAC(DACParameters::WBC, wbc - 1);
            tbInterface->Flush();

            int thr2 = pixel.GetRoc().PixelThreshold(pixel.GetColumn(), pixel.GetRow(), 0, 1, nTrig, 2 * nTrig, 25,
                                                     false, false, 0);
            if (debug)
                psi::LogInfo() << "thr 2 " << thr2 << std::endl;
            if (thr2 < thr) thr = thr2;
            pixel.GetRoc().SetDAC(DACParameters::WBC, wbc);
            tbInterface->Flush();
        }
        psi::LogInfo() << vtrim << " thr " << thr << std::endl;
    } while (((thr > vcal) || (thrOld > vcal) || (thr < 10)) && (vtrim < 200));
    vtrim += 5;
    pixel.GetRoc().SetDAC(DACParameters::Vtrim, vtrim);

    psi::LogDebug() << "[Trim] Vtrim is set to " << vtrim << std::endl;

    return vtrim;
}
