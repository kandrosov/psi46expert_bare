/*!
 * \file ThresholdMap.cc
 * \brief Implementation of ThresholdMap class.
 */

#include "ThresholdMap.h"
#include "psi46expert/TestRoc.h"
#include "BasePixel/constants.h"
#include "BasePixel/TBAnalogInterface.h"


void ThresholdMap::SetParameters(const std::string& mapName)
{
    cals = false;
    xtalk = false;
    reverseMode = false;
    dacReg = 12; //VthrComp

    if (mapName == "VcalThresholdMap") {
        dacReg = 25; //Vcal
    } else if (mapName == "VcalsThresholdMap") {
        dacReg = 25; //Vcal
        cals = true;
    } else if (mapName == "XTalkMap") {
        dacReg = 25; //Vcal
        xtalk = true;
    } else if (mapName == "NoiseMap") {
        reverseMode = true;
    } else if (mapName == "CalXTalkMap") {
        xtalk = true;
    }
}


TH2D* ThresholdMap::GetMap(const std::string& mapName, TestRoc& roc, const TestRange& testRange, int nTrig)
{
    SetParameters(mapName);
    MeasureMap(mapName, roc, testRange, nTrig);
    return histo;
}


void ThresholdMap::MeasureMap(const std::string& mapName, TestRoc& roc, const TestRange& testRange, int nTrig)
{
    std::ostringstream totalMapName;
    totalMapName << mapName << "_C" << roc.GetChipId();
    histo = new TH2D(totalMapName.str().c_str(), totalMapName.str().c_str(), psi::ROCNUMCOLS, 0., psi::ROCNUMCOLS,
                     psi::ROCNUMROWS, 0., psi::ROCNUMROWS);

    int wbc = roc.GetDAC(DACParameters::WBC);
    if (doubleWbc) {
        roc.SetDAC(DACParameters::WBC, wbc - 1);
        roc.Flush();
    }

    int sign = 1;
    if (reverseMode) sign = -1;

    int data[4160];
    roc.ChipThreshold(100, sign, nTrig / 2, nTrig, dacReg, xtalk, cals, data);

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
            roc.ChipThreshold(100, sign, nTrig / 2, nTrig, dacReg, xtalk, cals, data2);

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
}

void ThresholdMap::SetCals()
{
    cals = true;
}

void ThresholdMap::SetXTalk()
{
    xtalk = true;
}

void ThresholdMap::SetDoubleWbc()
{
    doubleWbc = true;
}

void ThresholdMap::SetSingleWbc()
{
    doubleWbc = false;
}

void ThresholdMap::SetReverseMode()
{
    reverseMode = true;
}

