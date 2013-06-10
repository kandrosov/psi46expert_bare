/*!
 * \file TestPixel.h
 * \brief Definition of TestPixel class.
 */

#pragma once

#include <boost/shared_ptr.hpp>

#include <TH1D.h>
#include <TH2D.h>
#include <TGraph.h>

#include "BasePixel/TestRange.h"

class TestRoc;

/*!
 * \brief Implementation of the tests at pixel level
 */
class TestPixel {
public:
    TestPixel(TestRoc& roc, unsigned columnNumber, unsigned rowNumber);
    TestRoc& GetRoc() const { return *roc; }
    double FindThreshold(const std::string& mapName, int nTrig, bool doubleWbc = false);

    void EnablePixel();
    void DisablePixel();
    void MaskCompletely();  //to be distinguished from DisablePixel
    const void Cal();
    const void Cals();
    void ArmPixel();
    void DisarmPixel();
    void SetTrim(int trimBit);
    int GetTrim();
    bool IsAlive();
    unsigned GetColumn();
    unsigned GetRow();
    void SetAlive(bool aBoolean);

    bool IsIncluded(boost::shared_ptr<const TestRange> testRange) const;

private:
    unsigned column, row;
    int trim;
    bool enabled, alive, masked;
    TestRoc* roc;
};
