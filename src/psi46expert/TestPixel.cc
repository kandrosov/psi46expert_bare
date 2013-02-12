/*!
 * \file TestPixel.cc
 * \brief Implementation of TestPixel class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - removed deprecated conversion from string constant to char*
 */

#include "TestPixel.h"
#include "TestRoc.h"
#include "BasePixel/CalibrationTable.h"
#include "BasePixel/TBAnalogInterface.h"
#include "interface/Delay.h"
#include "ThresholdMap.h"
#include "TestRange.h"


TestPixel::TestPixel(Roc* aRoc, int columnNumber, int rowNumber)
    : Pixel(aRoc, columnNumber, rowNumber) {}

TestRoc* TestPixel::GetRoc()
{
	return (TestRoc*)roc;
}

// -- Find the threshold (50% point of the SCurve)
double TestPixel::FindThreshold(const char *mapName, int nTrig, bool doubleWbc)
{
	TestRange *range = new TestRange();
	range->AddPixel(roc->GetChipId(), column, row);
	
	ThresholdMap* thresholdMap = new ThresholdMap();
	if (doubleWbc) thresholdMap->SetDoubleWbc();
	TH2D* map = thresholdMap->GetMap(mapName, GetRoc(), range, nTrig);

	double result = map->GetBinContent(column+1, row+1);
	
	delete range;
	delete thresholdMap;
	delete map;
	
	return result;
}
