/*!
 * \file takeData.cc
 * \brief Main entrence for takeData program.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new ConfigParameters class definition.
 *      - daqFrame removed due to compability issues.
 */

#include <cstdlib>
#include "BasePixel/TBAnalogInterface.h"
#include "psi46expert/TestParameters.h"
#include "psi46expert/TestControlNetwork.h"
#include "psi46expert/UsbDaq.h"
#include "psi46expert/daqLoggingManager.hh"
#include "psi46expert/histogrammer.h"
#include "BasePixel/RawPacketDecoder.h"
#include "BasePixel/DecoderCalibration.h"
#include "BasePixel/DecodedReadout.h"
#include "BasePixel/ConfigParameters.h"
#include <TString.h>
#include <TApplication.h>


// ----------------------------------------------------------------------
int main(int argc, char* argv[]) 
{ 
  int mode(7), runnumber(0), localtrigger(0);
  bool batchMode = false, trimArg = false, dacArg = false, maskArg = false;	
  char rootFile[1000], logFile[1000], dacFile[1000], trimFile[1000], directory[1000], tbName[1000], maskFile[1000];


  ConfigParameters& mtbConfigParameters = ConfigParameters::ModifiableSingleton();
  //  sprintf(mtbConfigParameters->directory, "testModule");
  mtbConfigParameters.setDirectory("./setup/mtb");
  int V=0;



  // -- command line arguments
  for (int i = 0; i < argc; i++)  {
    if (!strcmp(argv[i],"-b")) batchMode = true;
    if (!strcmp(argv[i],"-V")) V=atoi(argv[++i]);
    if (!strcmp(argv[i],"-l")) localtrigger = 1;
    if (!strcmp(argv[i],"-m")) mode = atoi(argv[++i]);
    if (!strcmp(argv[i],"-dir")) mtbConfigParameters.setDirectory(argv[++i]);
    if (!strcmp(argv[i],"-trimVcal")) 
    {
	trimArg = true;
	dacArg = true;
	int vcal = atoi(argv[++i]);
	sprintf(trimFile, "%s%i", "trimParameters", vcal);
	sprintf(dacFile, "%s%i", "dacParameters", vcal);
     }
    if (!strcmp(argv[i],"-mask"))
    {
      maskArg = true;
      sprintf(maskFile, "%s", "pixelMask.dat");
    }
  } 

  mtbConfigParameters.Read(Form("%s/configParameters.dat", mtbConfigParameters.Directory().c_str()));
  if (dacArg) mtbConfigParameters.setDacParametersFileName(dacFile);
  if (trimArg) mtbConfigParameters.setTrimParametersFileName(trimFile);
  if (maskArg) mtbConfigParameters.setMaskFileName(maskFile);
  
  //logging manager
  daqLoggingManager *lm = new daqLoggingManager("/tmp");
  lm->setRunMode(mode);
  lm->setMTBConfigParameters(&mtbConfigParameters);

  //decoder 
  RawPacketDecoder *gDecoder = RawPacketDecoder::Singleton();
  TString fileName = TString(mtbConfigParameters.Directory()).Append("/addressParameters.dat");
  std::cout << "Reading Address Level-Parameters from " << fileName << std::endl;
  DecoderCalibrationModule *decoderCalibrationModule = new DecoderCalibrationModule(fileName, 3, 0, DecodedReadoutConstants::NUM_ROCSMODULE);
  decoderCalibrationModule->Print(&std::cout);
  gDecoder->SetCalibration(decoderCalibrationModule);

  UsbDaq *daq = new UsbDaq(mode);
  if (runnumber > 0) lm->setRunNumber(runnumber);

  histogrammer *h = new histogrammer();
  daq->setHistogrammer(h);
  daq->setDecoder(gDecoder);

  // -- GUI
/*  TApplication *application = new TApplication("App", 0, 0, 0, -1);
  daqFrame *dF = new daqFrame(gClient->GetRoot(), 1000, 700, lm, batchMode);
  dF->setUsbDAQ(daq);
  dF->setLoggingManager(lm);
  if (localtrigger) dF->fLocalTrigger = 1;
//  if(V>0)dF->doVup(V);
 
  if (batchMode) {  
    dF->setRunDuration(15);
    dF->setFillMem(0);
    dF->doSetSysCommand1Text(); //exec module.ini
    if(V>0)dF->doVup(V);
    dF->doStart();
    dF->doStop();
    if(V>0)dF->doVdown(V);
    dF->doExit();
  } else  {   
    dF->doSetSysCommand1Text(); //exec module.ini
    application->Run();
    // if(V>0)dF->doVdown(V);
    dF->doExit();
  }*/
  return 0;
}
