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
    struct Parameters {
        std::string mapName;
        DACParameters::Register dacReg;
        bool reverseMode, xtalk, cals;
        Parameters() : dacReg(DACParameters::VthrComp), reverseMode(false), xtalk(false), cals(false) {}
        Parameters(const std::string& _mapName, DACParameters::Register _dacReg, bool _reverseMode, bool _xtalk,
                   bool _cals)
            : mapName(_mapName), dacReg(_dacReg), reverseMode(_reverseMode), xtalk(_xtalk), cals(_cals) {}

        int sign() const { return reverseMode ? -1 : 1; }
    };

    static const Parameters VcalThresholdMapParameters;
    static const Parameters VcalsThresholdMapParameters;
    static const Parameters XTalkMapParameters;
    static const Parameters NoiseMapParameters;
    static const Parameters CalXTalkMapParameters;
    static const Parameters CalThresholdMapParameters;

public:
    ThresholdMap() : doubleWbc(false) {}

    TH2D* MeasureMap(const Parameters& parameters, TestRoc& roc, const TestRange& testRange, int nTrig,
                     unsigned mapId = 0);

    void SetDoubleWbc() { doubleWbc = true; }
    void SetSingleWbc() { doubleWbc = false; }

private:
    bool doubleWbc;
};
