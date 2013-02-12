/*!
 * \file TestDoubleColumn.h
 * \brief Definition of TestDoubleColumn class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#pragma once

#include "BasePixel/DoubleColumn.h"
#include "TestPixel.h"

#include<TH1D.h>
#include<TH2D.h>

/*!
 * \brief Implementation of the tests at DoubleColumn level
 */
class TestDoubleColumn : public DoubleColumn {

 public:
    TestDoubleColumn(Roc* roc, int dColumn);
	TestPixel *GetPixel(int column, int row);
	TestPixel *GetPixel(int iPixel);

// == Tests =====================================================
	int FindGoodPixels(int count, TestPixel* pix[]);
	void TestWBCSBC();
	void TestTimeStampBuffer();
	void TestDataBuffer();
	void DoubleColumnTest();
};
