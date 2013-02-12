/*!
 * \file psi46expert.cpp
 * \brief Main entrence for psi46expert.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new ConfigParameters class definition.
 *      - MainFrame removed due to compability issues.
 *      - Adaptation for the new TestParameters class definition.
 * 10-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - IVoltageSource interface was changed.
 * 30-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Changed to support IVoltageSource interface.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - removed deprecated conversion from string constant to char*
 *      - current limits for 'id' and 'ia' are now stored in the configuration file
 *      - currents 'id' and 'ia' measured before and after chip startup are now saved into the output ROOT file.
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
#include <boost/shared_ptr.hpp>

#include "interface/Delay.h"
#include "psi46expert/TestControlNetwork.h"
#include "psi46expert/Xray.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/SysCommand.h"
#include "BasePixel/ConfigParameters.h"
#include "BasePixel/GlobalConstants.h"
#include "BasePixel/Getline.c"
#include "BasePixel/Keithley.h"
#include "interface/Log.h"
#include "BasePixel/psi_exception.h"
#include "BasePixel/Keithley237.h"

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

static const IVoltageSource::ElectricPotential VOLTAGE_FACTOR = 1.0 * boost::units::si::volts;
static const IVoltageSource::ElectricCurrent CURRENT_FACTOR = 1.0 * boost::units::si::ampere;
static const IVoltageSource::ElectricCurrent DEFAULT_COMPLIANCE = 1.0e-4 * boost::units::si::ampere;

void runGUI(TBInterface* tbInterface, TestControlNetwork* controlNetwork, ConfigParameters* configParameters)
{
/*  boost::scoped_ptr<TApplication> application(new TApplication("App",0,0, 0, -1));
  MainFrame mainFrame(gClient->GetRoot(), 400, 400, tbInterface, controlNetwork, configParameters);
  application->Run();*/
}

void execute(SysCommand &command, TBInterface* tbInterface, TestControlNetwork* controlNetwork)
{
  do
  {
//    if (command.Keyword("gui"))
//    {
//      runGUI(tbInterface, controlNetwork, configParameters);
//    }
    if (command.TargetIsTB()) {tbInterface -> Execute(command);}
    else  {controlNetwork->Execute(command);}
  }
  while (command.Next());
  tbInterface->Flush();
}

void runTest(TBInterface* tbInterface, TestControlNetwork* controlNetwork, SysCommand& sysCommand, const char* testMode)
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
    Test *test = new Xray(testRange, tbInterface);
    test->ControlNetworkAction(controlNetwork);
  } 
  if (strcmp(testMode, calTest) == 0)
  {
    sysCommand.Read("cal.sys");
    execute(sysCommand, tbInterface, controlNetwork);
  }
  if (strcmp(testMode, phCalTest) == 0)
  {
    sysCommand.Read("phCal.sys");
    execute(sysCommand, tbInterface, controlNetwork);
  }
  if (strcmp(testMode, dtlTest) == 0)
  {
    sysCommand.Read("dtlTest.sys");
    execute(sysCommand, tbInterface, controlNetwork);
  }
        
        if (strcmp(testMode, guiTest) == 0)
        {
          sysCommand.Read("gui.sys");
          execute(sysCommand, tbInterface, controlNetwork);
        }
        
        if (strcmp(testMode, ThrMaps) == 0)
        {
          sysCommand.Read("ThrMaps.sys");
          execute(sysCommand, tbInterface, controlNetwork);
        }
 	if (strcmp(testMode,scurveTest ) == 0)
        {
          sysCommand.Read("scurve.sys");
          execute(sysCommand, tbInterface, controlNetwork);
        }

  gDelay->Timestamp();
}


void runFile(TBInterface* tbInterface, TestControlNetwork* controlNetwork, SysCommand& sysCommand, const char* cmdFile)
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
  execute(sysCommand, tbInterface, controlNetwork);
  
  gDelay->Timestamp();
}


void parameters(int argc, char* argv[], std::string& cmdFile, std::string& testMode, bool& guiMode)
{
    ConfigParameters& configParameters = ConfigParameters::ModifiableSingleton();

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
      std::stringstream ss;
      ss << argv[++i];
      cmdFile = ss.str();
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
      if (strcmp(testMode.c_str(), dtlTest) == 0)
      {
        hubIdArg = true;
        hubId = -1;
      }
    }
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
  if (strcmp(testMode.c_str(), shortTest) == 0 || strcmp(testMode.c_str(), shortCalTest) == 0)
  {
    logFileArg = true;
    sprintf(logFile, "ShortTest.log");
    rootFileArg = true;
    sprintf(rootFile, "ShortTest.root");    
  }
  else if (strcmp(testMode.c_str(), calTest) == 0)
  {
    logFileArg = true;
    sprintf(logFile, "Calibration.log");
    rootFileArg = true;
    sprintf(rootFile, "Calibration.root");    
  }
  
  if (logFileArg) configParameters.setLogFileName(logFile);
  else configParameters.setLogFileName( "log.txt");

  configParameters.setDebugFileName( "debug.log");

  psi::LogInfo ().setOutput( configParameters.LogFileName() );
  psi::LogDebug().setOutput( configParameters.DebugFileName() );

  psi::LogInfo() << "[psi46expert] --------- psi46expert ---------" 
                 << psi::endl;
  psi::LogInfo() << "[psi46expert] " << TDatime().AsString() << psi::endl;
  
  configParameters.Read(Form("%s/configParameters.dat", directory));
  if (rootFileArg) configParameters.setRootFileName(rootFile);
  if (dacArg) configParameters.setDacParametersFileName(dacFile);
  if (tbArg) configParameters.setTestboardName(tbName);
  if (trimArg) configParameters.setTrimParametersFileName(trimFile);
    if (maskArg) configParameters.setMaskFileName(maskFile);
    if (hubIdArg) configParameters.setHubId(hubId);
}

void check_currents_before_setup(TBAnalogInterface& tbInterface)
{
    const ConfigParameters& configParameters = ConfigParameters::Singleton();
    const double ia_before_setup = tbInterface.GetIA();
    const double id_before_setup = tbInterface.GetID();

    psi::LogInfo() << "IA_before_setup = " << ia_before_setup << " A, ID_before_setup = "
                   << id_before_setup << " A." << psi::endl;
    Test::SaveMeasurement("ia_before_setup", ia_before_setup);
    Test::SaveMeasurement("id_before_setup", id_before_setup);

    if(ia_before_setup > configParameters.IA_BeforeSetup_HighLimit())
        THROW_PSI_EXCEPTION("[psi46expert] ERROR: IA before setup is too high. IA limit is "
                            << configParameters.IA_BeforeSetup_HighLimit() << " A.");
    if(id_before_setup > configParameters.ID_BeforeSetup_HighLimit())
        THROW_PSI_EXCEPTION("[psi46expert] ERROR: ID before setup is too high. ID limit is "
                            << configParameters.ID_BeforeSetup_HighLimit() << " A.");
}

void check_currents_after_setup(TBAnalogInterface& tbInterface)
{
    const ConfigParameters& configParameters = ConfigParameters::Singleton();
    const double ia_after_setup = tbInterface.GetIA();
    const double id_after_setup = tbInterface.GetID();

    psi::LogInfo() << "IA_after_setup = " << ia_after_setup << " A, ID_after_setup = "
                   << id_after_setup << " A." << psi::endl;
    Test::SaveMeasurement("ia_after_setup", ia_after_setup);
    Test::SaveMeasurement("id_after_setup", id_after_setup);

    if(ia_after_setup < configParameters.IA_AfterSetup_LowLimit())
        THROW_PSI_EXCEPTION("[psi46expert] ERROR: IA after setup is too low. IA low limit is "
                            << configParameters.IA_AfterSetup_LowLimit() << " A.");
    if(ia_after_setup > configParameters.IA_AfterSetup_HighLimit())
        THROW_PSI_EXCEPTION("[psi46expert] ERROR: IA after setup is too high. IA limit is "
                            << configParameters.IA_AfterSetup_HighLimit() << " A.");
    if(id_after_setup < configParameters.ID_AfterSetup_LowLimit())
        THROW_PSI_EXCEPTION("[psi46expert] ERROR: ID after setup is too low. ID low limit is "
                            << configParameters.ID_AfterSetup_LowLimit() << " A.");
    if(id_after_setup > configParameters.ID_AfterSetup_HighLimit())
        THROW_PSI_EXCEPTION("[psi46expert] ERROR: ID after setup is too high. ID limit is "
                            << configParameters.ID_AfterSetup_HighLimit() << " A.");
}

class TFileWrapper : public TFile
{
public:
    TFileWrapper(TFile* _file)
        : file(_file) {}
    virtual ~TFileWrapper()
    {
        file->Write();
        file->Close();
        delete file;
    }
private:
    TFile* file;
};

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

        std::string testMode = "";
        std::string cmdFile = "";

        bool guiMode(false);
        parameters(argc, argv, cmdFile, testMode, guiMode);
        const ConfigParameters& configParameters = ConfigParameters::Singleton();

        TFileWrapper histoFile(new TFile(configParameters.RootFileName().c_str(), "RECREATE"));
        gStyle->SetPalette(1,0);

        boost::scoped_ptr<TBAnalogInterface> tbInterface(new TBAnalogInterface());
        if (!tbInterface->IsPresent()) return -1;

        check_currents_before_setup(*tbInterface);

        boost::scoped_ptr<TestControlNetwork> controlNetwork(new TestControlNetwork(tbInterface.get()));
        check_currents_after_setup(*tbInterface);

        boost::shared_ptr<IVoltageSource> Power_supply;
        if(V>0)
        {
            const Keithley237::Configuration config("keithley");
            Power_supply = boost::shared_ptr<IVoltageSource>(new Keithley237(config));
            int volt=25,step=25;
            while (volt<V-25)
            {
                IVoltageSource::Value value(((double)volt) * VOLTAGE_FACTOR, DEFAULT_COMPLIANCE );
                Power_supply->Set(value);
                sleep(1);
                volt=volt+step;
                if(volt>400)
                    step=10;
                if(volt>600)
                    step=5;
            }
            IVoltageSource::Value value(((double)V) * VOLTAGE_FACTOR, DEFAULT_COMPLIANCE );
            Power_supply->Set(value);
            sleep(4);
        }

        SysCommand sysCommand;

//        if (guiMode) runGUI(tbInterface.get(), controlNetwork.get(), configParameters.get());
        if (strcmp(testMode.c_str(), "") != 0) runTest(tbInterface.get(), controlNetwork.get(), sysCommand,
                                                       testMode.c_str());
        else if (strcmp(cmdFile.c_str(), "") != 0) runFile(tbInterface.get(), controlNetwork.get(), sysCommand,
                                                           cmdFile.c_str());
        else
        {
            char *p;
            Gl_histinit("../.hist");
            do
            {
                p = Getline("psi46expert> ");
                Gl_histadd(p);

                psi::LogDebug() << "psi46expert> " << p << psi::endl;

                if (sysCommand.Parse(p)) execute(sysCommand, tbInterface.get(), controlNetwork.get());
            }
            while ((strcmp(p,"exit\n") != 0) && (strcmp(p,"q\n") != 0));
        }

        return 0;
    }
    catch(psi_exception& e)
    {
        psi::LogError() << e.what() << psi::endl;
        return 1;
    }
}
