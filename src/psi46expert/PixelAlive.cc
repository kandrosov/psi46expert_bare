/*!
 * \file PixelAlive.cc
 * \brief Implementation of PixelAlive class.
 *
 * \b Changelog
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using definitions from PsiCommon.h.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#include "interface/Log.h"

#include "PixelAlive.h"
#include "TestRoc.h"
#include "BasePixel/PsiCommon.h"
#include "BasePixel/TBAnalogInterface.h"
#include "TestParameters.h"

PixelAlive::PixelAlive(TestRange *aTestRange, TBInterface *aTBInterface)
{
  psi::Log<psi::Debug>() << "[PixelAlive] Initialization." << std::endl;

  testRange = aTestRange;
  tbInterface = aTBInterface;
  ReadTestParameters();
}


void PixelAlive::ReadTestParameters()
{
    const TestParameters& testParameters = TestParameters::Singleton();
    nTrig = testParameters.PixelMapReadouts();
    efficiency = testParameters.PixelMapEfficiency() / 100.0 ;
}


void PixelAlive::RocAction()
{
  psi::Log<psi::Debug>() << "[PixelAlive] Chip #" << chipId << '.' << std::endl;

  TH2D *histo = GetMap("PixelMap");
  histo->SetMaximum(nTrig);
  histo->SetMinimum(0);

  short mask[psi::ROCNUMROWS*psi::ROCNUMCOLS];
  roc->MaskTest(1, mask);
          
  for (int i = 0; i < psi::ROCNUMCOLS; i++)
  {
    for (int k = 0; k < psi::ROCNUMROWS; k++)
    {
      int n = mask[i*psi::ROCNUMROWS + k];
      if (n != 0)
      {
        GetPixel(i,k)->SetAlive(false);

        psi::Log<psi::Info>() << "[PixelAlive] Error: Mask Defect. n = " << n
                       << " for Pixel( " << i << ", " << k << ")." << std::endl;

        histo->SetBinContent(i+1, k+1, -1);
      }
      else histo->SetBinContent(i+1, k+1, 0);
    }
  }
  
  double data[psi::ROCNUMROWS*psi::ROCNUMCOLS];
  roc->ChipEfficiency(nTrig, data);
  
  for (int i = 0; i < psi::ROCNUMROWS*psi::ROCNUMCOLS; i++)
  {
    double value = data[i]*nTrig;
    if (value == 0)
      psi::Log<psi::Info>() << "[PixelAlive] Error: Dead Pixel( "
                     << ( i / psi::ROCNUMROWS) << ", " << ( i % psi::ROCNUMROWS)
                     << ") with n = " << static_cast<int>( value) << std::endl;
    if (value < 0) value = -2;  // to distinguish this problem from mask defects
    if (histo->GetBinContent(i/psi::ROCNUMROWS+1, i%psi::ROCNUMROWS+1) == 0)
        histo->SetBinContent(i/psi::ROCNUMROWS+1,i%psi::ROCNUMROWS+1, value);
  }
  
  histograms->Add(histo);
}
