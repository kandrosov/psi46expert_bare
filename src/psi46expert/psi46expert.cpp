/*!
 * \file psi46expert.cpp
 * \brief Main entrence for psi46expert.
 *
 * \b Changelog
 * 23-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Removed global variables
 *      - Pointers wrapped with boost::scoped_ptr
 *      - Added support of psi_exception.
 *      - Added current checks before and after chip startup.
 */

#include <time.h>
#include <stdio.h>

#include <TFile.h>
#include <TString.h>
#include <TApplication.h>
#include <TStyle.h>

#include <boost/scoped_ptr.hpp>

#include "interface/Delay.h"
#include "psi46expert/TestParameters.h"
#include "psi46expert/TestControlNetwork.h"
#include "psi46expert/MainFrame.h"
#include "psi46expert/Xray.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/SysCommand.h"
#include "BasePixel/ConfigParameters.h"
#include "BasePixel/GlobalConstants.h"
#include "BasePixel/Getline.c"
#include "BasePixel/Keithley.h"
#include "interface/Log.h"
#include "BasePixel/psi_exception.h"

static const char *fullTest = "full";
static const char *shortTest = "short";
static const char *shortCalTest = "shortCal";
static const char *calTest = "cal";
static const char *phCalTest = "phCal";
static const char *dtlTest = "dtlScan";
static const char *xrayTest = "xray";

static const char *guiTest = "GUI";
static const char *scurveTest = "scurves";
static const char *preTest = "preTest";
static const char *TrimTest = "trimTest";
static const char *ThrMaps ="ThrMaps";

void runGUI(TBInterface* tbInterface, TestControlNetwork* controlNetwork, ConfigParameters* configParameters)
{
  boost::scoped_ptr<TApplication> application(new TApplication("App",0,0, 0, -1));
  MainFrame mainFrame(gClient->GetRoot(), 400, 400, tbInterface, controlNetwork, configParameters);
  application->Run();
}


void execute(SysCommand &command, TBInterface* tbInterface, TestControlNetwork* controlNetwork,
             ConfigParameters* configParameters)
{
  do
  {
    if (command.Keyword("gui"))
    {
      runGUI(tbInterface, controlNetwork, configParameters);
    }
    else if (command.TargetIsTB()) {tbInterface -> Execute(command);}
    else  {controlNetwork->Execute(command);}
  }
  while (command.Next());
  tbInterface->Flush();
}



void runTest(TBInterface* tbInterface, TestControlNetwork* controlNetwork, ConfigParameters* configParameters,
             SysCommand& sysCommand, const char* testMode)
{
  if (tbInterface->IsPresent() < 1)
  {
    cout << "Error!! Testboard not present. Aborting" << endl;
    return;
  }
  gDelay->Timestamp();
  if (strcmp(testMode, fullTest) == 0)
  {
    psi::LogInfo() << "[psi46expert] SvFullTest and Calibration: start." << psi::endl; 

    controlNetwork->FullTestAndCalibration();

    psi::LogInfo() << "[psi46expert] SvFullTest and Calibration: end." << psi::endl; 
  }
  if (strcmp(testMode, shortTest) == 0)
  {
    psi::LogInfo() << "[psi46expert] SvShortTest: start." << psi::endl; 

    controlNetwork->ShortCalibration();

    psi::LogInfo() << "[psi46expert] SvShortTest: end." << psi::endl; 
  }
  if (strcmp(testMode, shortCalTest) == 0)
  {
    psi::LogInfo() << "[psi46expert] SvShortTest and Calibration: start." << psi::endl; 

    controlNetwork->ShortTestAndCalibration();

    psi::LogInfo() << "[psi46expert] SvShortTest and Calibration: end." << psi::endl; 
  } 
  if (strcmp(testMode, xrayTest) == 0)
  {
    TestRange *testRange = new TestRange();
    testRange->CompleteRange();
    Test *test = new Xray(testRange, controlNetwork->GetTestParameters(), tbInterface);
    test->ControlNetworkAction(controlNetwork);
  } 
  if (strcmp(testMode, calTest) == 0)
  {
    sysCommand.Read("cal.sys");
    execute(sysCommand, tbInterface, controlNetwork, configParameters);
  }
  if (strcmp(testMode, phCalTest) == 0)
  {
    sysCommand.Read("phCal.sys");
    execute(sysCommand, tbInterface, controlNetwork, configParameters);
  }
  if (strcmp(testMode, dtlTest) == 0)
  {
    sysCommand.Read("dtlTest.sys");
    execute(sysCommand, tbInterface, controlNetwork, configParameters);
  }
        
        if (strcmp(testMode, guiTest) == 0)
        {
          sysCommand.Read("gui.sys");
          execute(sysCommand, tbInterface, controlNetwork, configParameters);
        }
        
        if (strcmp(testMode, ThrMaps) == 0)
        {
          sysCommand.Read("ThrMaps.sys");
          execute(sysCommand, tbInterface, controlNetwork, configParameters);
        }
 	if (strcmp(testMode,scurveTest ) == 0)
        {
          sysCommand.Read("scurve.sys");
          execute(sysCommand, tbInterface, controlNetwork, configParameters);
        }

  gDelay->Timestamp();
}


void runFile(TBInterface* tbInterface, TestControlNetwork* controlNetwork, ConfigParameters* configParameters,
             SysCommand& sysCommand, const char* cmdFile)
{
  if (tbInterface->IsPresent() < 1)
  {
    psi::LogInfo() << "[psi46expert] Error: Testboard is not present. Abort.";

    return;
  }
  
  gDelay->Timestamp();
  
  psi::LogInfo() << "[psi46expert] Executing file '" << cmdFile
                 << "'." << psi::endl; 

  sysCommand.Read(cmdFile);
  execute(sysCommand, tbInterface, controlNetwork, configParameters);
  
  gDelay->Timestamp();
}


void parameters(int argc, char* argv[], ConfigParameters *configParameters, char* cmdFile, char* testMode,
                bool& guiMode)
{
  int hubId;
  char rootFile[1000], logFile[1000], dacFile[1000], trimFile[1000], directory[1000], tbName[1000], maskFile[1000];
        sprintf(directory, "testModule");
  bool rootFileArg(false), dacArg(false), trimArg(false), tbArg(false), logFileArg(false), cmdFileArg(false), hubIdArg(false),
	     maskArg(false);

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
		if (!strcmp(argv[i],"-mask")) 
		{
			maskArg = true;
			sprintf(maskFile, "%s","pixelMask.dat" );//argv[++i]);
		}		
    if (!strcmp(argv[i],"-tb")) 
    {
      tbArg = true;
      sprintf(tbName, "%s", argv[++i]);
    }
    if (!strcmp(argv[i],"-t")) 
    {
      testMode = argv[++i];
      if (strcmp(testMode, dtlTest) == 0)
      {
        hubIdArg = true;
        hubId = -1;
      }
    }
    if (!strcmp(argv[i],"-g")) guiMode = true;

  } 
  sprintf(configParameters->directory, directory);
  
  if (strcmp(testMode, fullTest) == 0)
  {
    logFileArg = true;
    sprintf(logFile, "FullTest.log");
    rootFileArg = true;
    sprintf(rootFile, "FullTest.root");   
  }
  if (strcmp(testMode, shortTest) == 0 || strcmp(testMode, shortCalTest) == 0)
  {
    logFileArg = true;
    sprintf(logFile, "ShortTest.log");
    rootFileArg = true;
    sprintf(rootFile, "ShortTest.root");    
  }
  else if (strcmp(testMode, calTest) == 0)
  {
    logFileArg = true;
    sprintf(logFile, "Calibration.log");
    rootFileArg = true;
    sprintf(rootFile, "Calibration.root");    
  }
  
  if (logFileArg) configParameters->SetLogFileName(logFile);
  else configParameters->SetLogFileName( "log.txt");

  configParameters->SetDebugFileName( "debug.log");

  psi::LogInfo ().setOutput( configParameters->GetLogFileName() );
  psi::LogDebug().setOutput( configParameters->GetDebugFileName() );

  psi::LogInfo() << "[psi46expert] --------- psi46expert ---------" 
                 << psi::endl;
  psi::LogInfo() << "[psi46expert] " << TDatime().AsString() << psi::endl;
  
  configParameters->ReadConfigParameterFile(Form("%s/configParameters.dat", directory));
  if (rootFileArg) configParameters->SetRootFileName(rootFile);
  if (dacArg) configParameters->SetDacParameterFileName(dacFile);
  if (tbArg) sprintf(configParameters->testboardName, tbName);
  if (trimArg) configParameters->SetTrimParameterFileName(trimFile);
	if (maskArg) configParameters->SetMaskFileName(maskFile);
  if (hubIdArg) configParameters->hubId = hubId;
}


int main(int argc, char* argv[])
{
    try
    {
        int V=0;
        for (int i = 0; i < argc; i++)
        {
            if (!strcmp(argv[i],"-V"))
                V=atoi(argv[++i]);
        }

        boost::scoped_ptr<ConfigParameters> configParameters(ConfigParameters::Singleton());
        char* testMode(""), cmdFile[1000] = "";
        bool guiMode(false);
        parameters(argc, argv, configParameters.get(), cmdFile, testMode, guiMode);

        // == Initialization =====================================================================

        boost::scoped_ptr<TFile> histoFile(new TFile(configParameters->GetRootFileName(), "RECREATE"));
        gStyle->SetPalette(1,0);

        boost::scoped_ptr<TBAnalogInterface> tbInterface(new TBAnalogInterface(configParameters.get()));

        static const double IA_BEFORE_SETUP_HIGH_LIMIT = 0.120; // A
        static const double ID_BEFORE_SETUP_HIGH_LIMIT = 0.120; // A
        static const double IA_AFTER_SETUP_LOW_LIMIT = 0.010; // A
        static const double ID_AFTER_SETUP_LOW_LIMIT = 0.010; // A
        static const double IA_AFTER_SETUP_HIGH_LIMIT = 0.100; // A
        static const double ID_AFTER_SETUP_HIGH_LIMIT = 0.100; // A

        const double ia_before_setup = tbInterface->GetIA();
        const double id_before_setup = tbInterface->GetID();
        psi::LogInfo() << "IA_before_setup = " << ia_before_setup << " A, ID_before_setup = "
                       << id_before_setup << " A." << psi::endl;
        if(ia_before_setup > IA_BEFORE_SETUP_HIGH_LIMIT)
            THROW_PSI_EXCEPTION("ERROR: IA before setup is too high. IA limit is " << IA_BEFORE_SETUP_HIGH_LIMIT << " A.");
        if(id_before_setup > ID_BEFORE_SETUP_HIGH_LIMIT)
            THROW_PSI_EXCEPTION("ERROR: ID before setup is too high. ID limit is " << ID_BEFORE_SETUP_HIGH_LIMIT << " A.");

        if (!tbInterface->IsPresent()) return -1;
        boost::scoped_ptr<TestControlNetwork> controlNetwork(new TestControlNetwork(tbInterface.get(),
                                                                                    configParameters.get()));
        const double ia_after_setup = tbInterface->GetIA();
        const double id_after_setup = tbInterface->GetID();
        psi::LogInfo() << "IA_after_setup = " << ia_after_setup << " A, ID_after_setup = "
                       << id_after_setup << " A." << psi::endl;
        if(ia_after_setup < IA_AFTER_SETUP_LOW_LIMIT)
            THROW_PSI_EXCEPTION("ERROR: IA after setup is too low. IA low limit is " << IA_AFTER_SETUP_LOW_LIMIT << " A.");
        if(ia_after_setup > IA_AFTER_SETUP_HIGH_LIMIT)
            THROW_PSI_EXCEPTION("ERROR: IA after setup is too high. IA limit is " << IA_AFTER_SETUP_HIGH_LIMIT << " A.");
        if(id_after_setup < ID_AFTER_SETUP_LOW_LIMIT)
            THROW_PSI_EXCEPTION("ERROR: ID after setup is too low. ID low limit is " << ID_AFTER_SETUP_LOW_LIMIT << " A.");
        if(id_after_setup > ID_AFTER_SETUP_HIGH_LIMIT)
            THROW_PSI_EXCEPTION("ERROR: ID after setup is too high. ID limit is " << ID_AFTER_SETUP_HIGH_LIMIT << " A.");


        //  sysCommand.Read("start.sys");
        //  execute(sysCommand);

        boost::scoped_ptr<Keithley> Power_supply(new Keithley());
        if(V>0)
        {
            Power_supply->Open();
            Power_supply->Init();
            int volt=25,step=25;
            while (volt<V-25)
            {
                Power_supply->SetVoltage(volt,1);
                volt=volt+step;
                if(volt>400)
                    step=10;
                if(volt>600)
                    step=5;
            }
            Power_supply->SetVoltage(V,4);
        }

        SysCommand sysCommand;

        if (guiMode) runGUI(tbInterface.get(), controlNetwork.get(), configParameters.get());
        else if (strcmp(testMode, "") != 0) runTest(tbInterface.get(), controlNetwork.get(), configParameters.get(),
                                                    sysCommand, testMode);
        else if (strcmp(cmdFile, "") != 0) runFile(tbInterface.get(), controlNetwork.get(), configParameters.get(),
                                                   sysCommand, cmdFile);
        else
        {
            // == CommandLine ================================================================

            char *p;
            Gl_histinit("../.hist");
            do
            {
                p = Getline("psi46expert> ");
                Gl_histadd(p);

                psi::LogDebug() << "psi46expert> " << p << psi::endl;

                if (sysCommand.Parse(p)) execute(sysCommand, tbInterface.get(), controlNetwork.get(),
                                                 configParameters.get());
            }
            while ((strcmp(p,"exit\n") != 0) && (strcmp(p,"q\n") != 0));
        }

        // == Exit ========================================================================

        if (!strcmp(testMode, phCalTest) == 0)
        {
            tbInterface->HVoff();
            tbInterface->Poff();
            tbInterface->Cleanup();
        }

        if(V>0)
            Power_supply->ShutDown();

        histoFile->Write();
        histoFile->Close();

        return 0;
    }
    catch(psi_exception& e)
    {
        psi::LogError() << e.what() << psi::endl;
        return 1;
    }
}
