/*!
 * \file PhDacOverview.h
 * \brief Definition of PhDacOverview class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#pragma once

#include "Test.h"
#include "PhDacScan.h"
#include <TH2D.h>
#include <TH1D.h>
#include <TArrayD.h>

/*!
 * \brief pulse height - Vcal overview for different DACs
 */
class PhDacOverview : public PhDacScan
{
public:
  PhDacOverview(TestRange *testRange, TBInterface *aTBInterface);
	
  virtual void ReadTestParameters();
  virtual void RocAction();
  virtual void PixelAction();

  void DoDacScan();
  void DoVsfScan();

 protected:
  int NumberOfSteps;
};
