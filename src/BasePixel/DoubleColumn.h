/*!
 * \file DoubleColumn.h
 * \brief Definition of DoubleColumn class.
 *
 * \b Changelog
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using definitions from PsiCommon.h.
 */

#pragma once

#include "Pixel.h"
#include "PsiCommon.h"

class Roc;

/*!
 * \brief Class provides basic double column functionalities, contains the pixel objects.
 */
class DoubleColumn {
public:
    inline DoubleColumn() {}
	DoubleColumn(Roc* roc, int dColumn);
	virtual ~DoubleColumn();
	int DoubleColumnNumber();

// == DoubleColumn actions =====================================

	void EnableDoubleColumn();
	void DisableDoubleColumn();
	void Mask();

// == Pixel actions ============================================

	Pixel *GetPixel(int column, int row);
	void EnablePixel(int col, int row);
	void DisablePixel(int col, int row);
	void Cal(int col, int row);
	void Cals(int col, int row);
	void ArmPixel(int column, int row);
	void DisarmPixel(int column, int row);

	
 protected:
 	int doubleColumn;
 
    static const int NPixels = 2*psi::ROCNUMROWS;
	Pixel *pixel[NPixels];
	
	Roc* roc;
};
