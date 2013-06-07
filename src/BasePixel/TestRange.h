/*!
 * \file TestRange.h
 * \brief Definition of TestRange class.
 */

#pragma once

#include "BasePixel/constants.h"

/*!
 * \brief Defines for which entities a test should be performed
 */
class TestRange {
public:
    TestRange();

    void AddPixel(int roc, int col, int row);
    void RemovePixel(int roc, int col, int row);
    bool ExcludesColumn(int roc, int col);
    bool ExcludesRow(int roc, int row);
    bool ExcludesRoc(int roc);

    void CompleteRange();
    void CompleteRoc(int iRoc);

    bool IncludesPixel(int roc, int col, int row);
    bool IncludesRoc(int roc);
    bool IncludesDoubleColumn(int roc, int doubleColumn);
    bool IncludesColumn(int roc, int column);
    bool IncludesColumn(int column);

    void ApplyMaskFile( const char *fileName);

    void Print();

protected:
    bool pixel[psi::MODULENUMROCS][psi::ROCNUMCOLS][psi::ROCNUMROWS];
};
