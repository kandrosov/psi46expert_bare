/*!
 * \file debugData.cpp
 * \brief Provides some functionality to debug test board.
 *
 * \b Changelog
 * 15-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Switching to use GNU readline library instead getline.c
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new ConfigParameters class definition.
 *      - MainFrame removed due to compability issues.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - removed deprecated conversion from string constant to char*
 */

#include <time.h>
#include <stdio.h>

#include <TFile.h>
#include <TString.h>
#include <TApplication.h>
#include <TStyle.h>

#include "interface/Delay.h"
#include "psi46expert/TestParameters.h"
#include "psi46expert/TestControlNetwork.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/SysCommand.h"
#include "BasePixel/ConfigParameters.h"
#include "BasePixel/GlobalConstants.h"
#include "interface/Log.h"


TBAnalogInterface* tbInterface;
TestControlNetwork *controlNetwork;
SysCommand sysCommand;

std::string testMode = "";
char cmdFile[1000];
bool guiMode(false);

const char *fullTest = "full";
const char *calTest = "cal";

void runGUI()
{
//  TApplication *application = new TApplication("App",0,0, 0, -1);
//  MainFrame MainFrame(gClient->GetRoot(), 400, 400, tbInterface, controlNetwork, configParameters);
//  application->Run();
}


void execute(SysCommand &command)
{
  do
  {
    if (command.Keyword("gui"))
    {
      runGUI();
    }
    else if (command.TargetIsTB()) {tbInterface -> Execute(command);}
    else  {controlNetwork->Execute(command);}
  }
  while (command.Next());
  tbInterface->Flush();
}



void runTest()
{
  if (tbInterface->IsPresent() < 1)
  {
    psi::LogInfo() << "[DebugData] Error: Testboard is not present. Abort."
                   << psi::endl; 

    return;
  }
  gDelay->Timestamp();
  if (strcmp(testMode.c_str(), fullTest) == 0)
  {
    psi::LogInfo() << "[DebugData] SvFullTest: start." << psi::endl; 

    sysCommand.Read("test.sys");
    execute(sysCommand);

    psi::LogInfo() << "[DebugData] SvFullTest: end." << psi::endl; 
  }
  if (strcmp(testMode.c_str(), calTest) == 0)
  {
    sysCommand.Read("cal.sys");
    execute(sysCommand);
  }
  gDelay->Timestamp();
}


void runFile()
{
  if (tbInterface->IsPresent() < 1)
  {
    psi::LogInfo() << "[DebugData] Error: Testboard is not present. Abort."
                   << psi::endl; 

    return;
  }
  
  gDelay->Timestamp();
  
  psi::LogInfo() << "[DebugData] Executing file '" << cmdFile
                 << "'." << psi::endl; 

  sysCommand.Read(cmdFile);
  execute(sysCommand);
  
  gDelay->Timestamp();
}


void parameters(int argc, char* argv[])
{
  ConfigParameters& configParameters = ConfigParameters::ModifiableSingleton();
  char rootFile[1000], logFile[1000], dacFile[1000], trimFile[1000], directory[1000], tbName[1000];
        sprintf(directory, "testModule");
  bool rootFileArg(false), dacArg(false), trimArg(false), tbArg(false), logFileArg(false), cmdFileArg(false);

  // == command line arguments ======================================================
  for (int i = 0; i < argc; i++)
  {
    if (!strcmp(argv[i],"-dir")) 
    {
      sprintf(directory, argv[++i]);
    }
    if (!strcmp(argv[i],"-c")) 
    {
      rootFileArg = true;
      sprintf(rootFile, Form("test-%s.root", argv[++i]));
    }
    if (!strcmp(argv[i],"-d"))
    {
      dacArg = true;
      sprintf(dacFile, "%s", argv[++i]);
    }
    if (!strcmp(argv[i],"-r"))
    {
      rootFileArg = true;
      sprintf(rootFile, "%s", argv[++i]);
    }
    if (!strcmp(argv[i],"-f"))
    {
      cmdFileArg = true;
      sprintf(cmdFile, "%s", argv[++i]);
    }
    if (!strcmp(argv[i],"-log"))
    {
      logFileArg = true;
      sprintf(logFile, "%s", argv[++i]);
    }
    if (!strcmp(argv[i],"-trim")) 
    {
      trimArg = true;
      sprintf(trimFile, "%s", argv[++i]);
    }
    if (!strcmp(argv[i],"-trimVcal")) 
    {
      trimArg = true;
      dacArg = true;
      int vcal = atoi(argv[++i]);
      sprintf(trimFile, "%s%i", "trimParameters", vcal);
      sprintf(dacFile, "%s%i", "dacParameters", vcal);
    }
    if (!strcmp(argv[i],"-tb")) 
    {
      tbArg = true;
      sprintf(tbName, "%s", argv[++i]);
    }
    if (!strcmp(argv[i],"-t")) testMode = argv[++i];
    if (!strcmp(argv[i],"-g")) guiMode = true;
  }
  configParameters.setDirectory(directory);
  
  if (strcmp(testMode.c_str(), fullTest) == 0)
  {
    logFileArg = true;
    sprintf(logFile, "FullTest.log");
    rootFileArg = true;
    sprintf(rootFile, "FullTest.root");   
  }
  else if (strcmp(testMode.c_str(), calTest) == 0)
  {
    logFileArg = true;
    sprintf(logFile, "Calibration.log");
    rootFileArg = true;
    sprintf(rootFile, "Calibration.root");    
  }
  
  if (logFileArg) configParameters.setLogFileName(logFile);
  else configParameters.setLogFileName("log.txt");

  configParameters.setDebugFileName( "debug.log");

  psi::LogInfo ().setOutput( configParameters.FullFileName(configParameters.LogFileName()) );
  psi::LogDebug().setOutput( configParameters.FullFileName(configParameters.DebugFileName()) );

  psi::LogInfo() << "[DebugData] --------- psi46expert ---------" << psi::endl;
  psi::LogInfo() << "[DebugData] " << TDatime().AsString() << psi::endl;

  
  configParameters.Read(Form("%s/configParameters.dat", directory));
  if (rootFileArg) configParameters.setRootFileName(rootFile);
  if (dacArg) configParameters.setDacParametersFileName(dacFile);
  if (tbArg) configParameters.setTestboardName(tbName);
  if (trimArg) configParameters.setTrimParametersFileName(trimFile);
}


int main(int argc, char* argv[])
{
  sprintf(cmdFile, "");
  parameters(argc, argv);

  // == Initialization =====================================================================
  const ConfigParameters& configParameters = ConfigParameters::Singleton();

  TFile* histoFile = new TFile(configParameters.FullFileName(configParameters.RootFileName()).c_str(), "RECREATE");
  gStyle->SetPalette(1,0);
  
  tbInterface = new TBAnalogInterface();
  if (!tbInterface->IsPresent()) return -1;
  controlNetwork = new TestControlNetwork(tbInterface);

  // == Main ========================================================================

  const int dataBuffer_numWords = 1000;

  cout << "starting data-taking" << endl;
  unsigned int dataBuffer_fpga = tbInterface->getCTestboard()->Daq_Init(dataBuffer_numWords*sizeof(unsigned short));
  tbInterface->getCTestboard()->Daq_Enable();

  tbInterface->DataCtrl(false, false, true);
  tbInterface->SetReg(41, 0x4A);
  tbInterface->Flush();

  while ( !tbInterface->getCTestboard()->Daq_Ready() ){
    cout << "words written = " << (tbInterface->getCTestboard()->Daq_GetPointer() - dataBuffer_fpga)/sizeof(unsigned short) << endl;
    cout << tbInterface->getCTestboard()->Daq_GetPointer() << endl;
    sleep(1.);
  }

  cout << "data-taking finished." << endl;
  
  unsigned char dataBuffer_char[dataBuffer_numWords*sizeof(unsigned short)];
  tbInterface->getCTestboard()->MemRead((unsigned int)dataBuffer_fpga, dataBuffer_numWords*sizeof(unsigned short), dataBuffer_char);

  unsigned short dataBuffer_short[dataBuffer_numWords];
  for ( int iword = 0; iword < dataBuffer_numWords; iword++ ){
    //int word = (dataBuffer_char[2*iword] << 8) + dataBuffer_char[2*iword + 1];
          int word = dataBuffer_char[2*iword] + (dataBuffer_char[2*iword + 1] << 8);
    int data = (word & 0x0fff);
    if ( data & 0x0800 ) data -= 4096;
    //cout << hex << data << " ";
    cout << hex << word << " ";
    //cout << hex << (int)dataBuffer_char[2*iword] << " " << hex << (int)dataBuffer_char[2*iword + 1] << " ";
    dataBuffer_short[iword] = word;
  }
  cout << endl;

  unsigned short* adcData_bufferEnd  = dataBuffer_short + dataBuffer_numWords;
  unsigned short* adcData_eventStart = dataBuffer_short;

//--- decode "event" information contained in data buffer
  while ( adcData_eventStart < adcData_bufferEnd ){

//--- find header
    while ( adcData_eventStart < adcData_bufferEnd && ((*adcData_eventStart) & 0x8000) == 0 ) adcData_eventStart++;

//--- find next header
    unsigned short* adcData_eventEnd = adcData_eventStart + 4; 
    while ( adcData_eventEnd < adcData_bufferEnd && ((*adcData_eventEnd) & 0x8000) == 0 ) adcData_eventEnd++;

//--- set "event" stop pointer one position before header of next "event"
    adcData_eventEnd -= 1;

//--- print ADC data
    if ( adcData_eventEnd < adcData_bufferEnd && ((*adcData_eventStart) & 0x0001) != 0 ){
      cout << "found ADC data (" << dec << adcData_eventEnd - adcData_eventStart - 3 << " ADC words): " << endl;
      cout << " ";
      unsigned short* adcData_p = adcData_eventStart + 4;
      while ( adcData_p <= adcData_eventEnd ){
        int word = (*adcData_p);
        int data = (word & 0x0fff);
        if ( data & 0x0800 ) data -= 4096;
//        cout << hex << word << " ";
        cout << dec << data << " ";
        adcData_p++;
      }
      cout << endl;
    }
    
//--- set start pointer to next "event"
    adcData_eventStart = adcData_eventEnd + 1;
  }
  
  tbInterface->SetReg(41, 0x12);
  tbInterface->Flush();
  
  tbInterface->getCTestboard()->Daq_Done();

  // == Exit ========================================================================

  tbInterface->HVoff();
  tbInterface->Poff();
  tbInterface->Cleanup();

  delete controlNetwork;
  delete tbInterface;

  histoFile->Write();
  histoFile->Close();
  delete histoFile;
  
  return 0;
}
