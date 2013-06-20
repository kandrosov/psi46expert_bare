/*!
 * \file TestPixel.cc
 * \brief Implementation of TestPixel class.
 */

#include "TestPixel.h"
#include "TestRoc.h"
#include "BasePixel/CalibrationTable.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/ThresholdMap.h"
#include "BasePixel/TestRange.h"


TestPixel::TestPixel(TestRoc &aRoc, unsigned columnNumber, unsigned rowNumber)
    : column(columnNumber), row(rowNumber), trim(15), enabled(false), alive(false), masked(false), roc(&aRoc) {}

// -- Find the threshold (50% point of the SCurve)
double TestPixel::FindThreshold(int nTrig, bool doubleWbc)
{
    TestRange range;
    range.AddPixel(GetRoc().GetChipId(), column, row);

    ThresholdMap thresholdMap;
    if (doubleWbc) thresholdMap.SetDoubleWbc();
    TH2D* map = thresholdMap.MeasureMap(ThresholdMap::CalThresholdMapParameters, GetRoc(), range, nTrig);

    const double result = map->GetBinContent(column + 1, row + 1);

    delete map;

    return result;
}

void TestPixel::EnablePixel()
{
    if (masked) return;
    roc->PixTrim(column, row, trim);
    enabled = true;
}


void TestPixel::DisablePixel()
{
    roc->PixMask(column, row);
    enabled = false;
}


void TestPixel::MaskCompletely()
{
    masked = true;
}


const void TestPixel::Cal()
{
    roc->PixCal(column, row, 0);
}


const void TestPixel::Cals()
{
    roc->PixCal(column, row, 1);
}


void TestPixel::ArmPixel()
{
    EnablePixel();
    Cal();
}


void TestPixel::DisarmPixel()
{
    DisablePixel();
    roc->ClrCal();
}


void TestPixel::SetTrim(int trimBit)
{
    trim = trimBit;
}


int TestPixel::GetTrim()
{
    return trim;
}


bool TestPixel::IsAlive()
{
    return alive;
}


unsigned TestPixel::GetColumn()
{
    return column;
}


unsigned TestPixel::GetRow()
{
    return row;
}

void TestPixel::SetAlive(bool aBoolean)
{
    alive = aBoolean;
}

bool TestPixel::IsIncluded(boost::shared_ptr<const TestRange> testRange) const
{
    return testRange && testRange->IncludesPixel(roc->GetChipId(), column, row);
}
