/*!
 * \file TestDoubleColumn.h
 * \brief Definition of TestDoubleColumn class.
 *
 * \b Changelog
 * 26-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Removed redundant dependency from class DoubleColumn.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#pragma once

#include "BasePixel/PsiCommon.h"
#include "TestPixel.h"

#include<TH1D.h>
#include<TH2D.h>

/*!
 * \brief Implementation of the tests at DoubleColumn level
 */
class TestDoubleColumn {

 public:
    TestDoubleColumn(TestRoc* roc, int dColumn);
    ~TestDoubleColumn();
	TestPixel *GetPixel(int column, int row);
	TestPixel *GetPixel(int iPixel);

// == Tests =====================================================
	int FindGoodPixels(int count, TestPixel* pix[]);
	void TestWBCSBC();
	void TestTimeStampBuffer();
	void TestDataBuffer();
    void DoubleColumnTest();
    int DoubleColumnNumber();

// == DoubleColumn actions =====================================

    void EnableDoubleColumn();
    void DisableDoubleColumn();
    void Mask();

// == Pixel actions ============================================

    //TestPixel *GetPixel(int column, int row);
    void EnablePixel(int col, int row);
    void DisablePixel(int col, int row);
    void Cal(int col, int row);
    void Cals(int col, int row);
    void ArmPixel(int column, int row);
    void DisarmPixel(int column, int row);


 protected:
    int doubleColumn;

    static const int NPixels = 2*psi::ROCNUMROWS;
    TestPixel *pixel[NPixels];

    TestRoc* roc;

};
