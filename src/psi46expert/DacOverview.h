/*!
 * \file DacOverview.h
 * \brief Definition of DacOverview class.
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

class DacOverview : public PhDacScan
{
 public:

  DacOverview(TestRange *testRange, TBInterface *aTBInterface);
	
  virtual void ReadTestParameters();
  virtual void RocAction();

  void DoDacScan();

 protected:
  int NumberOfSteps, DacType;
};
