/*!
 * \file TestPixel.cc
 * \brief Implementation of TestPixel class.
 *
 * \b Changelog
 * 02-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using psi::Sleep instead interface/Delay.
 * 26-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Removed redundant dependency from class Pixel.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - removed deprecated conversion from string constant to char*
 */

#include "TestPixel.h"
#include "TestRoc.h"
#include "BasePixel/CalibrationTable.h"
#include "BasePixel/TBAnalogInterface.h"
#include "ThresholdMap.h"
#include "TestRange.h"


TestPixel::TestPixel(TestRoc* aRoc, int columnNumber, int rowNumber)
    : column(columnNumber), row(rowNumber), roc(aRoc)
{
    trim = 15;
    enabled = false;
    alive = false;
    masked = false;
}

TestRoc* TestPixel::GetRoc()
{
	return (TestRoc*)roc;
}

// -- Find the threshold (50% point of the SCurve)
double TestPixel::FindThreshold(const char *mapName, int nTrig, bool doubleWbc)
{
	TestRange *range = new TestRange();
    range->AddPixel(GetRoc()->GetChipId(), column, row);
	
	ThresholdMap* thresholdMap = new ThresholdMap();
	if (doubleWbc) thresholdMap->SetDoubleWbc();
	TH2D* map = thresholdMap->GetMap(mapName, GetRoc(), range, nTrig);

	double result = map->GetBinContent(column+1, row+1);
	
	delete range;
	delete thresholdMap;
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


int TestPixel::GetColumn()
{
    return column;
}


int TestPixel::GetRow()
{
    return row;
}

void TestPixel::SetAlive(bool aBoolean)
{
    alive = aBoolean;
}
