/*!
 * \file TestPixel.h
 * \brief Definition of TestPixel class.
 */

#pragma once

#include <TH1D.h>
#include <TH2D.h>
#include <TGraph.h>

class TestRoc;

/*!
 * \brief Implementation of the tests at pixel level
 */
class TestPixel {
public:
    TestPixel(TestRoc *roc, int columnNumber, int rowNumber);
    TestRoc *GetRoc();
    double FindThreshold(const char* mapName, int nTrig, bool doubleWbc = false);
    // == Actions ==========================================================

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
    int GetColumn();
    int GetRow();
    void SetAlive(bool aBoolean);


protected:

    const int column, row;
    int trim;
    bool enabled, alive, masked;

    TestRoc* const roc;
};
