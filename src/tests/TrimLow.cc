/*!
 * \file TrimLow.cc
 * \brief Implementation of TrimLow class.
 */

#include "psi/log.h"

#include "TrimLow.h"
#include "psi46expert/TestRoc.h"
#include "BasePixel/constants.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/Analysis.h"
#include "BasePixel/ThresholdMap.h"
#include "BasePixel/TestParameters.h"

TrimLow::TrimLow(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface)
    : Test("TrimLow", testRange), tbInterface(aTBInterface), noTrimBits(false)
{
    const TestParameters& testParameters = TestParameters::Singleton();
    doubleWbc = testParameters.TrimDoubleWbc();
    nTrig = testParameters.TrimNTrig();
    vcal = testParameters.TrimVcal();
}

void TrimLow::AddMap(TH2D* calMap)
{
    TH1D *distr = Analysis::Distribution(calMap, 255, 0., 255.);
    histograms->Add(calMap);
    histograms->Add(distr);
}


void TrimLow::RocAction(TestRoc& roc)
{
    TestPixel *maxPixel = 0;
    double thr, thrMin;
    TH2D *calMap;

    if (doubleWbc) thresholdMap.SetDoubleWbc();

    psi::LogInfo() << "[TrimLow] ROC #" << roc.GetChipId() << ": Start." << std::endl;
    SaveDacParameters(roc);

    //get VthrComp
    roc.SetTrim(15);
    roc.SetDAC(DACParameters::Vtrim, 0);

    psi::LogDebug() << "[TrimLow] Vcal " << vcal << std::endl;

    roc.SetDAC(DACParameters::Vcal, vcal);
    tbInterface->Flush();

    thrMin = MinVthrComp(roc, ThresholdMap::CalThresholdMapParameters);
    if (thrMin == -1.) return;

    thresholdMap.SetSingleWbc();
    roc.SetDAC(DACParameters::Vcal, 100);
    tbInterface->Flush();
    double thrMin2 = MinVthrComp(roc, ThresholdMap::NoiseMapParameters);
    if (doubleWbc) thresholdMap.SetDoubleWbc();

    if (thrMin2 - 10 < thrMin) thrMin = thrMin2 - 10;
    roc.SetDAC(DACParameters::VthrComp, (int)thrMin);

    psi::LogDebug() << "[TrimLow] VthrComp is set to "
                    << static_cast<int>( thrMin) << std::endl;

    tbInterface->Flush();

    tbInterface->Flush();

    //Determine minimal and maximal vcal thresholds
    calMap = thresholdMap.MeasureMap(ThresholdMap::VcalThresholdMapParameters, roc, *testRange, nTrig);
    AddMap(calMap);
    TH1D *distr = Analysis::Distribution(calMap, 255, 1., 254.);
    double vcalMaxLimit = TMath::Min(254., distr->GetMean() + 5.*distr->GetRMS());

    double vcalMin = 255., vcalMax = 0.;
    int thr255 = 0;
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

    psi::LogDebug() << "[TrimLow] There are " << thr255 << " pixels with "
                    << "Vcal 255." << std::endl;
    psi::LogDebug() << "[TrimLow] Vcal range is [ " << vcalMin << ", "
                    << vcalMax << "]." << std::endl;

    if (vcalMax == 0) {
        psi::LogInfo() << "[TrimLow] Error: Vcal max = 0. Abort test." << std::endl;

        return;
    }

    //Determine Vtrim
    roc.EnableDoubleColumn(maxPixel->GetColumn());
    int vtrim = AdjustVtrim(*maxPixel);
    roc.DisableDoubleColumn(maxPixel->GetColumn());

    if (!noTrimBits) {
        roc.SetTrim(7);
        calMap = thresholdMap.MeasureMap(ThresholdMap::VcalThresholdMapParameters, roc, *testRange, nTrig);
        AddMap(calMap);

        calMap = TrimStep(roc, 4, calMap);
        calMap = TrimStep(roc, 2, calMap);
        calMap = TrimStep(roc, 1, calMap);
        calMap = TrimStep(roc, 1, calMap);

        calMap = thresholdMap.MeasureMap(ThresholdMap::VcalThresholdMapParameters, roc, *testRange, nTrig);
        AddMap(calMap);
    }

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

double TrimLow::MinVthrComp(TestRoc& roc, const ThresholdMap::Parameters &mapParameters)
{
    //Find good VthrComp
    TH2D *calMap = thresholdMap.MeasureMap(mapParameters, roc, *testRange, nTrig);
    AddMap(calMap);
    TH1D *distr = Analysis::Distribution(calMap, 255, 1., 254.);
    double thrMinLimit = TMath::Max(1., distr->GetMean() - 5.*distr->GetRMS());

    double thrMin = 255., thrMax = 0., thr;
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

    psi::LogDebug() << "[TrimLow] There are " << thr255 << " pixels with "
                    << "threshold 255." << std::endl;
    psi::LogDebug() << "[TrimLow] Theshold range is [ " << thrMin << ", "
                    << thrMax << "]." << std::endl;

    if (thrMax == 0.) {
        psi::LogInfo() << "[TrimLow] Error: Can not find maximum threshold."
                       << std::endl;

        return -1.;
    }

    return thrMin;
}


TH2D* TrimLow::TrimStep(TestRoc& roc, int correction, TH2D *calMapOld)
{
    TH2D* betterCalMap = CreateMap("VcalThresholdMap", roc.GetChipId());
    int trim;

    //save trim map
    TH2D *trimMap = roc.TrimMap();

    //set new trim bits
    for (unsigned i = 0; i < psi::ROCNUMCOLS; i++) {
        for (unsigned k = 0; k < psi::ROCNUMROWS; k++) {
            if (testRange->IncludesPixel(roc.GetChipId(), i, k)) {
                trim = (int)trimMap->GetBinContent(i + 1, k + 1);
                if ((calMapOld->GetBinContent(i + 1, k + 1) > vcal) && (calMapOld->GetBinContent(i + 1, k + 1) != 255))
                    trim -= correction;
                else
                    trim += correction;

                if (trim < 0) trim = 0;
                if (trim > 15) trim = 15;
                roc.GetPixel(i, k).SetTrim(trim);
            }
        }
    }
    AddMap(roc.TrimMap());

    //measure new result
    TH2D *calMap = thresholdMap.MeasureMap(ThresholdMap::VcalThresholdMapParameters, roc, *testRange, nTrig);
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

    AddMap(roc.TrimMap());

    return betterCalMap;
}

int TrimLow::AdjustVtrim(TestPixel& pixel)
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
        thr = pixel.GetRoc().PixelThreshold(pixel.GetColumn(), pixel.GetRow(), 0, 1, nTrig, 2 * nTrig, 25, false,
                                            false, 0);
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

    psi::LogDebug() << "[TrimLow] Vtrim is set to " << vtrim << std::endl;

    return vtrim;
}
