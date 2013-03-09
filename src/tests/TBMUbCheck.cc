/*!
 * \file TBMUbCheck.cc
 * \brief Implementation of TBMUbCheck class.
 *
 * \b Changelog
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using definitions from PsiCommon.h.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new ConfigParameters class definition.
 *      - Adaptation for the new TestParameters class definition.
 */

#include "TBMUbCheck.h"
#include "BasePixel/constants.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/TBM.h"
#include "psi46expert/TestModule.h"

#include "BasePixel/TestParameters.h"

TBMUbCheck::TBMUbCheck(TestRange* aTestRange, TBInterface* aTBInterface)
{	
  testRange = aTestRange;
  tbInterface = aTBInterface;
  ReadTestParameters();
  debug = false;
}


void TBMUbCheck::ReadTestParameters()
{
    ubTarget = TestParameters::Singleton().TBMUbLevel();
}


void TBMUbCheck::ModuleAction()
{
  psi::LogInfo() << "Starting TBMUbCheck" << std::endl;

  TBM* tbm = module->GetTBM();
  TBAnalogInterface* anaInterface = (TBAnalogInterface*)tbInterface;
  const ConfigParameters& configParameters = ConfigParameters::Singleton();
  bool halfModule = !(configParameters.HalfModule() == 0);

  short data[psi::FIFOSIZE];
  const int nTrig = 10;
  bool tbmChannelOk;
  
  int dtlOrig = configParameters.DataTriggerLevel();
  int dtl = TMath::Min(ubTarget + 300, -200);
  anaInterface->DataTriggerLevel(dtl);
    
  int tbmChannel_saved = anaInterface->GetTBMChannel();
  int tbmMode_saved    = tbm->GetDAC(0);
  int tbmGain_saved    = tbm->GetDAC(4);
  
  int tbmGain_target = 0;

  int nChannels;
  if (halfModule) nChannels = 1; else nChannels = 2;  
  
  for ( int itbm = 0; itbm < nChannels; itbm++ )
  {
    if (!halfModule)
    {
      anaInterface->SetTBMChannel(itbm);
      module->SetTBMSingle(itbm);
    }
    tbmChannelOk = false;

    int tbmGain = 255;
    for ( int ibit = 7; ibit >= 0; ibit-- ){
      int bitValue = (ibit >= 1) ? (2 << (ibit - 1)) : 1;
      int tbmGain_new  = tbmGain - bitValue;

      tbm->SetDAC(4, tbmGain_new);

      unsigned short count = 0;
      ((TBAnalogInterface*)tbInterface)->ADCRead(data, count, nTrig);

      if ( count > 0 ){
        tbmChannelOk = true;
	double ubLevel = (data[0] + data[1] + data[2])/3.;
	if ( ubLevel < ubTarget ) tbmGain = tbmGain_new;
      }
    }

    unsigned short count = 0;
    ((TBAnalogInterface*)tbInterface)->ADCRead(data, count, nTrig);
    if ( count > 0 ){
      double ubLevel = data[0];
      psi::LogInfo() << "tbmGain = " << tbmGain << ", ubLevel = " << ubLevel << std::endl;
    }

//--- save maximum Dacgain of both TBMs
//    (can only set one Dacgain value for both)
    if ( tbmChannelOk && (tbmGain > tbmGain_target) ) tbmGain_target = tbmGain;
  }
  
  if (tbmGain_target == 0) tbmGain_target = tbmGain_saved;

  psi::LogInfo() << "setting tbmGain to " << tbmGain_target << std::endl;
  tbm->SetDAC(4, tbmGain_target);

//--- restore previous TBM settings
  anaInterface->SetTBMChannel(tbmChannel_saved);
  tbm->SetDAC(0, tbmMode_saved);
  anaInterface->DataTriggerLevel(dtlOrig);
  Flush();

  tbm->WriteTBMParameterFile(configParameters.FullTbmParametersFileName().c_str());
}
