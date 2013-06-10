/*!
 * \file TrimLow.h
 * \brief Definition of TrimLow class.
 */

#pragma once

#include "BasePixel/Test.h"
#include <TH2D.h>
#include "BasePixel/ThresholdMap.h"

/*!
 * \brief Trim functions
 */
class TrimLow : public Test {
public:
    TrimLow(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface);
    virtual void RocAction(TestRoc& roc);
    void SetVcal(int _vcal) { vcal = _vcal; }

private:
    double MinVthrComp(TestRoc& roc, const std::string& mapName);
    int AdjustVtrim(TestPixel& pixel);
    void AddMap(TH2D* calMap);
    TH2D* TrimStep(TestRoc& roc, int correction, TH2D *calMapOld);
    void NoTrimBits(bool aBool) { noTrimBits = aBool; }


private:
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    int vthrComp, doubleWbc, nTrig, vcal;
    bool noTrimBits;
    ThresholdMap thresholdMap;
};
