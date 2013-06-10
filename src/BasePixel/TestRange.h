/*!
 * \file TestRange.h
 * \brief Definition of TestRange class.
 */

#pragma once

#include <vector>

/*!
 * \brief Defines for which entities a test should be performed
 */
class TestRange {
public:
    TestRange();

    void AddPixel(unsigned roc, unsigned col, unsigned row);

    void CompleteRange();
    void CompleteRoc(unsigned roc);

    bool IncludesPixel(unsigned roc, unsigned col, unsigned row) const;
    bool IncludesRoc(unsigned roc) const;
    bool IncludesDoubleColumn(unsigned roc, unsigned doubleColumn) const;
    bool IncludesColumn(unsigned roc, unsigned column) const;
    bool IncludesColumn(unsigned column) const;

    void Print() const;

private:
    static unsigned Index(unsigned roc, unsigned col, unsigned row);
    bool& Get(unsigned roc, unsigned col, unsigned row);
    bool IncludesAtLeastOnePixelInRange(unsigned firstIndex, unsigned lastIndex) const;
    std::vector<bool> pixels;
};
