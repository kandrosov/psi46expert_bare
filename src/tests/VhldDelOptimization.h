/*!
 * \file VhldDelOptimization.h
 * \brief Definition of VhldDelOptimization class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#pragma once

#include "BasePixel/Test.h"

/*!
 * \brief Adjust VhldDel by optimizing the Linearity in the low range
 */
class VhldDelOptimization : public Test
{
 public:
	VhldDelOptimization();
    VhldDelOptimization(TestRange *testRange, TBInterface *aTBInterface);

	void Initialize();	
    virtual void ReadTestParameters();
	virtual void RocAction();
	int AdjustVhldDel(TestRange *pixelRange);
	void PixelLoop();

 private:
	int hldDelValue;
};
