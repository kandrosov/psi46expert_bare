/*!
 * \file ThresholdMap.h
 * \brief Definition of ThresholdMap class.
 */

#pragma once

#include <TH2D.h>
#include "psi46expert/TestRoc.h"
#include "TestRange.h"

/*!
 * \brief Trim functions
 */
class ThresholdMap {

public:
    ThresholdMap() : doubleWbc(false) {}

    TH2D* GetMap(const std::string& mapName, TestRoc& roc, const TestRange& testRange, int nTrig);
    void MeasureMap(const std::string& mapName, TestRoc& roc, const TestRange& testRange, int nTrig);
    void SetParameters(const std::string& mapName);
    void SetCals();
    void SetXTalk();
    void SetDoubleWbc();
    void SetSingleWbc();
    void SetReverseMode();

private:
    TH2D *histo;
    int dacReg;
    bool cals, reverseMode, xtalk, doubleWbc;
};
