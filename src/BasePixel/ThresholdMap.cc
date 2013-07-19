/*!
 * \file ThresholdMap.cc
 * \brief Implementation of ThresholdMap class.
 */

#include "ThresholdMap.h"
#include "data/HistogramNameProvider.h"
#include "psi46expert/TestRoc.h"
#include "BasePixel/constants.h"
#include "BasePixel/TBAnalogInterface.h"

const ThresholdMap::Parameters ThresholdMap::VcalThresholdMapParameters(
        psi::data::HistogramNameProvider::VcalThresholdMapName(), DACParameters::Vcal, false, false, false);
const ThresholdMap::Parameters ThresholdMap::VcalsThresholdMapParameters(
        psi::data::HistogramNameProvider::VcalsThresholdMapName(), DACParameters::Vcal, false, false, true);
const ThresholdMap::Parameters ThresholdMap::XTalkMapParameters(
        psi::data::HistogramNameProvider::XTalkMapName(), DACParameters::Vcal, false, true, false);
const ThresholdMap::Parameters ThresholdMap::NoiseMapParameters(
        psi::data::HistogramNameProvider::NoiseMapName(), DACParameters::VthrComp, true, false, false);
const ThresholdMap::Parameters ThresholdMap::CalXTalkMapParameters(
        psi::data::HistogramNameProvider::CalXTalkMapName(), DACParameters::VthrComp, false, true, false);
const ThresholdMap::Parameters ThresholdMap::CalThresholdMapParameters(
        psi::data::HistogramNameProvider::CalThresholdMapName(), DACParameters::VthrComp, false, false, false);

TH2D* ThresholdMap::MeasureMap(const Parameters& parameters, TestRoc& roc, const TestRange& testRange, unsigned nTrig,
                              unsigned mapId)
{
    return MeasureMap(parameters, roc, testRange, nTrig / 2, nTrig, mapId);
}

TH2D* ThresholdMap::MeasureMap(const Parameters& parameters, TestRoc& roc, const TestRange& testRange,
                               unsigned thrLevel, unsigned nTrig, unsigned mapId)
{
    const std::string fullMapName =
            psi::data::HistogramNameProvider::FullMapName(parameters.mapName, roc.GetChipId(), mapId);
    TH2D* histo = new TH2D(fullMapName.c_str(), fullMapName.c_str(), psi::ROCNUMCOLS, 0., psi::ROCNUMCOLS,
                     psi::ROCNUMROWS, 0., psi::ROCNUMROWS);

    int wbc = roc.GetDAC(DACParameters::WBC);
    if (doubleWbc) {
        roc.SetDAC(DACParameters::WBC, wbc - 1);
        roc.Flush();
    }

    int data[4160];
    roc.ChipThreshold(100, parameters.sign(), thrLevel, nTrig, parameters.dacReg, parameters.xtalk, parameters.cals,
                      data);

    for (unsigned iCol = 0; iCol < psi::ROCNUMCOLS ; iCol++) {
        for (unsigned iRow = 0; iRow < psi::ROCNUMROWS ; iRow++) {
            if (testRange.IncludesPixel(roc.GetChipId(), iCol, iRow)) {
                histo->SetBinContent(iCol + 1, iRow + 1, data[iCol * psi::ROCNUMROWS + iRow]);
            }
        }
    }

    if (doubleWbc) {
        roc.SetDAC(DACParameters::WBC, wbc);
        roc.Flush();

        if (histo->GetMaximum() == 255) { // if there are pixels where no threshold could be found, test other wbc
            int data2[4160];
            roc.ChipThreshold(100, parameters.sign(), thrLevel, nTrig, parameters.dacReg, parameters.xtalk,
                              parameters.cals, data2);

            for (unsigned iCol = 0; iCol < psi::ROCNUMCOLS ; iCol++) {
                for (unsigned iRow = 0; iRow < psi::ROCNUMROWS ; iRow++) {
                    if (testRange.IncludesPixel(roc.GetChipId(), iCol, iRow)) {
                        int index = iCol * psi::ROCNUMROWS + iRow;
                        if (data2[index] < data[index]) histo->SetBinContent(iCol + 1, iRow + 1, data2[index]);
                    }
                }
            }
        }
    }

    roc.SetDAC(DACParameters::WBC, wbc); // restore original wbc
    return histo;
}
