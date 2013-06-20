/*!
 * \file ThresholdMap.cc
 * \brief Implementation of ThresholdMap class.
 */

#include "ThresholdMap.h"
#include "psi46expert/TestRoc.h"
#include "BasePixel/constants.h"
#include "BasePixel/TBAnalogInterface.h"

const ThresholdMap::Parameters ThresholdMap::VcalThresholdMapParameters("VcalThresholdMap", DACParameters::Vcal,
                                                                        false, false, false);
const ThresholdMap::Parameters ThresholdMap::VcalsThresholdMapParameters("VcalsThresholdMap", DACParameters::Vcal,
                                                                         false, false, true);
const ThresholdMap::Parameters ThresholdMap::XTalkMapParameters("XTalkMap", DACParameters::Vcal,
                                                                false, true, false);
const ThresholdMap::Parameters ThresholdMap::NoiseMapParameters("NoiseMap", DACParameters::VthrComp,
                                                                true, false, false);
const ThresholdMap::Parameters ThresholdMap::CalXTalkMapParameters("CalXTalkMap", DACParameters::VthrComp,
                                                                   false, true, false);
const ThresholdMap::Parameters ThresholdMap::CalThresholdMapParameters("CalThresholdMap", DACParameters::VthrComp,
                                                                       false, false, false);

TH2D* ThresholdMap::MeasureMap(const Parameters& parameters, TestRoc& roc, const TestRange& testRange, unsigned nTrig,
                              unsigned mapId)
{
    return MeasureMap(parameters, roc, testRange, nTrig / 2, nTrig, mapId);
}

TH2D* ThresholdMap::MeasureMap(const Parameters& parameters, TestRoc& roc, const TestRange& testRange,
                               unsigned thrLevel, unsigned nTrig, unsigned mapId)
{
    std::ostringstream totalMapName;
    totalMapName << parameters.mapName << "_C" << roc.GetChipId();
    if(mapId)
        totalMapName << "_nb" << mapId;
    TH2D* histo = new TH2D(totalMapName.str().c_str(), totalMapName.str().c_str(), psi::ROCNUMCOLS, 0., psi::ROCNUMCOLS,
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
