/*!
 * \file Xray.h
 * \brief Definition of Xray class.
 *
 * \b Changelog
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using definitions from PsiCommon.h.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#pragma once

#include "BasePixel/Test.h"

/*!
 * \brief Vcal calibration with xray
 */
class Xray : public Test
{
public:
  Xray(TestRange *testRange, TBInterface *aTBInterface);
  virtual void ReadTestParameters();
  virtual void ModuleAction();
  virtual void RocAction();
	
protected:
  int nTrig, vthrCompMin, vthrCompMax;
  double maxEff;
  TH1F *histo[psi::MODULENUMROCS];
};
