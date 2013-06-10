/*!
 * \file TestRange.cc
 * \brief Implementation of TestRange class.
 */

#include "psi/log.h"
#include "BasePixel/constants.h"
#include "TestRange.h"

unsigned TestRange::Index(unsigned roc, unsigned col, unsigned row)
{
    return row + psi::ROCNUMROWS * ( col + psi::ROCNUMCOLS * roc );
}

TestRange::TestRange()
{
    pixels.assign(psi::MODULENUMROCS * psi::ROCNUMCOLS * psi::ROCNUMROWS, false);
}

void TestRange::CompleteRange()
{
    pixels.assign(pixels.size(), true);
}

void TestRange::CompleteRoc(unsigned roc)
{
    const unsigned lastIndex = Index(roc + 1, 0, 0);
    for(unsigned n = Index(roc, 0, 0); n < lastIndex; ++n)
        pixels.at(n) = true;
}

void TestRange::AddPixel(unsigned roc, unsigned col, unsigned row)
{
    pixels.at(Index(roc, col, row)) = true;
}

bool TestRange::IncludesPixel(unsigned roc, unsigned col, unsigned row) const
{
    return pixels.at(Index(roc, col, row));
}

bool TestRange::IncludesAtLeastOnePixelInRange(unsigned firstIndex, unsigned lastIndex) const
{
    for(unsigned n = firstIndex; n < lastIndex; ++n)
        if(pixels.at(n)) return true;
    return false;
}

bool TestRange::IncludesRoc(unsigned roc) const
{
    const unsigned firstIndex = Index(roc, 0, 0);
    const unsigned lastIndex = Index(roc + 1, 0, 0);
    return IncludesAtLeastOnePixelInRange(firstIndex, lastIndex);
}

bool TestRange::IncludesDoubleColumn(unsigned roc, unsigned doubleColumn) const
{
    const unsigned firstIndex = Index(roc, doubleColumn * 2, 0);
    const unsigned lastIndex = Index(roc, (doubleColumn + 1) * 2, 0);
    return IncludesAtLeastOnePixelInRange(firstIndex, lastIndex);
}

bool TestRange::IncludesColumn(unsigned column) const
{
    for (unsigned roc = 0; roc < psi::MODULENUMROCS; ++roc)
        if(IncludesColumn(roc, column)) return true;
    return false;
}

bool TestRange::IncludesColumn(unsigned roc, unsigned column) const
{
    const unsigned firstIndex = Index(roc, column, 0);
    const unsigned lastIndex = Index(roc, column + 1, 0);
    return IncludesAtLeastOnePixelInRange(firstIndex, lastIndex);
}

void TestRange::Print() const
{
    for (unsigned i = 0; i < psi::MODULENUMROCS; i++) {
        for (unsigned k = 0; k < psi::ROCNUMCOLS; k++) {
            for (unsigned l = 0; l < psi::ROCNUMROWS; l++) {
                if (pixels.at(Index(i, k, l)))
                    psi::LogInfo() << "pixel " << i << " " << k << " " << l << std::endl;
            }
        }
    }
}
