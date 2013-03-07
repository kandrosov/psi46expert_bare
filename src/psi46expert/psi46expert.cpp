/*!
 * \file psi46expert.cpp
 * \brief Main entrence for psi46expert.
 *
 * \b Changelog
 * 06-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Component creation and execution moved from main to the class Program.
 *      - Now using TestBoardFactory to create a test board.
 * 04-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - The startup current checks moved into TestControlNetwork constructor.
 * 02-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using psi::Sleep instead interface/Delay.
 * 01-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using a new PSI Logging System.
 *      - Class SysCommand removed.
 *      - Class Keithley removed.
 * 28-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using psi::control::Shell class to provide command line user interface.
 * 26-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adoptation for the new multithread TestControlNetwork interface.
 * 25-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - IVoltageSource, VoltageSourceFactory and DataStorage moved into psi namespace.
 *      - psi_exception renamed to exception and moved into psi namespace.
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using VoltageSourceFactory.
 *      - Now using definitions from PsiCommon.h.
 * 21-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using DataStorage class to save the results.
 * 18-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Added 'help' command.
 *      - Added CommandLine class.
 * 15-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using boost::units::quantity to represent physical values.
 *      - Switching to use GNU readline library instead getline.c
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

#include <iostream>

#include <boost/thread.hpp>

#include "psi46expert/TestControlNetwork.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/ConfigParameters.h"
#include "BasePixel/constants.h"
#include "psi/log.h"
#include "BasePixel/DataStorage.h"
#include "BasePixel/VoltageSourceFactory.h"
#include "PsiShell.h"
#include "BasePixel/FakeTestBoard.h"
#include "BiasVoltageController.h"
#include "TestBoardFactory.h"

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

static const std::string LOG_HEAD = "psi46expert";

//void runTest(TBInterface* tbInterface, TestControlNetwork* controlNetwork, SysCommand& sysCommand, const char* testMode)
//{
//  if (tbInterface->IsPresent() < 1)
//  {
//    std::cout << "Error!! Testboard not present. Aborting" << std::endl;
//    return;
//  }
//  gDelay->Timestamp();
//  if (strcmp(testMode, fullTest) == 0)
//  {
//    psi::Log<psi::Info>() << "[psi46expert] SvFullTest and Calibration: start." << std::endl;

//    controlNetwork->FullTestAndCalibration();

//    psi::Log<psi::Info>() << "[psi46expert] SvFullTest and Calibration: end." << std::endl;
//  }
//  if (strcmp(testMode, shortTest) == 0)
//  {
//    psi::Log<psi::Info>() << "[psi46expert] SvShortTest: start." << std::endl;

//    controlNetwork->ShortCalibration();

//    psi::Log<psi::Info>() << "[psi46expert] SvShortTest: end." << std::endl;
//  }
//  if (strcmp(testMode, shortCalTest) == 0)
//  {
//    psi::Log<psi::Info>() << "[psi46expert] SvShortTest and Calibration: start." << std::endl;

//    controlNetwork->ShortTestAndCalibration();

//    psi::Log<psi::Info>() << "[psi46expert] SvShortTest and Calibration: end." << std::endl;
//  }
////  if (strcmp(testMode, xrayTest) == 0)
////  {
////    TestRange *testRange = new TestRange();
////    testRange->CompleteRange();
////    Test *test = new Xray(testRange, tbInterface);
////    test->ControlNetworkAction(controlNetwork);
////  }
//  if (strcmp(testMode, calTest) == 0)
//  {
//    sysCommand.Read("cal.sys");
//    execute(sysCommand, tbInterface, controlNetwork);
//  }
//  if (strcmp(testMode, phCalTest) == 0)
//  {
//    sysCommand.Read("phCal.sys");
//    execute(sysCommand, tbInterface, controlNetwork);
//  }
//  if (strcmp(testMode, dtlTest) == 0)
//  {
//    sysCommand.Read("dtlTest.sys");
//    execute(sysCommand, tbInterface, controlNetwork);
//  }
        
//        if (strcmp(testMode, guiTest) == 0)
//        {
//          sysCommand.Read("gui.sys");
//          execute(sysCommand, tbInterface, controlNetwork);
//        }
        
//        if (strcmp(testMode, ThrMaps) == 0)
//        {
//          sysCommand.Read("ThrMaps.sys");
//          execute(sysCommand, tbInterface, controlNetwork);
//        }
// 	if (strcmp(testMode,scurveTest ) == 0)
//        {
//          sysCommand.Read("scurve.sys");
//          execute(sysCommand, tbInterface, controlNetwork);
//        }

//  gDelay->Timestamp();
//}

void parameters(int argc, char* argv[], std::string& cmdFile, std::string& testMode, bool& guiMode)
{
    ConfigParameters& configParameters = ConfigParameters::ModifiableSingleton();

  int hubId;
  char rootFile[1000], logFile[1000], dacFile[1000], trimFile[1000], directory[1000], tbName[1000], maskFile[1000];
        sprintf(directory, ".");
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

  psi::Log<psi::Info> ().open( configParameters.FullLogFileName() );
  psi::Log<psi::Debug>().open( configParameters.FullDebugFileName() );

  psi::Log<psi::Info>(LOG_HEAD) << "--------- psi46expert ---------" << std::endl;
  psi::Log<psi::Info>(LOG_HEAD).PrintTimestamp();
  
  configParameters.Read(Form("%s/configParameters.dat", directory));
  if (rootFileArg) configParameters.setRootFileName(rootFile);
  if (dacArg) configParameters.setDacParametersFileName(dacFile);
  if (tbArg) configParameters.setTestboardName(tbName);
  if (trimArg) configParameters.setTrimParametersFileName(trimFile);
    if (maskArg) configParameters.setMaskFileName(maskFile);
    if (hubIdArg) configParameters.setHubId(hubId);
}

namespace psi {
namespace psi46expert {
namespace detail {
class BiasThread
{
public:
    BiasThread(boost::shared_ptr<psi::BiasVoltageController> biasController)
        : controller(biasController),
          thread(boost::bind(&psi::BiasVoltageController::operator(), biasController.get())) {}
    ~BiasThread()
    {
        controller->DisableControl();
        controller->DisableBias();
        controller->Stop();
        thread.join();
    }

private:
    boost::shared_ptr<psi::BiasVoltageController> controller;
    boost::thread thread;
};

} // detail

class Program
{
public:
    Program()
        : haveCompliance(false), haveError(false)
    {
        const ConfigParameters& configParameters = ConfigParameters::Singleton();

        dataStorage = boost::shared_ptr<psi::DataStorage>(new psi::DataStorage(configParameters.FullRootFileName()));
        psi::DataStorage::setActive(dataStorage);

        tbInterface = psi::TestBoardFactory::MakeAnalog();
        if (!tbInterface->IsPresent())
            THROW_PSI_EXCEPTION("Unable to connect to the test board.");

        biasController = boost::shared_ptr<psi::BiasVoltageController>(
                    new psi::BiasVoltageController(boost::bind(&Program::OnCompliance, this, _1),
                                                   boost::bind(&Program::OnError, this, _1)));
        controlNetwork = boost::shared_ptr<TestControlNetwork>(new TestControlNetwork(tbInterface));
        shell = boost::shared_ptr<psi::control::Shell>(new psi::control::Shell(".psi46expert_history"));
    }

    void Run()
    {
        detail::BiasThread biasControllerThread(biasController);
        biasController->EnableBias();
        biasController->EnableControl();

        bool canRun = true;
        bool printHelpLine = true;
        while(canRun)
        {
            shell->Run(printHelpLine);
            boost::lock_guard<boost::mutex> lock(mutex);
            if(!haveError && haveCompliance)
            {
                biasController->DisableBias();
                haveCompliance = false;
                printHelpLine = false;
            }
            else
                canRun = false;
        }
    }

private:
    void OnCompliance(const psi::IVoltageSource::Measurement&)
    {
        boost::lock_guard<boost::mutex> lock(mutex);
        Log<Error>() << std::endl;
        Log<Error>(LOG_HEAD) << "ERROR: compliance is reached. Any running test will be aborted."
                                " Bias voltages will be switched off." << std::endl;
        shell->InterruptExecution();
        biasController->DisableControl();
        haveCompliance = true;
    }

    void OnError(const std::exception& e)
    {
        boost::lock_guard<boost::mutex> lock(mutex);
        Log<Error>() << std::endl;
        Log<Error>(LOG_HEAD) << "CRITICAL ERROR in the bias control thread." << std::endl << e.what() << std::endl
                             << "Program will be terminated." << std::endl;
        shell->InterruptExecution();
        biasController->DisableControl();
        haveError = true;
    }

private:
    boost::mutex mutex;
    bool haveCompliance, haveError;
    boost::shared_ptr<psi::DataStorage> dataStorage;
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    boost::shared_ptr<psi::BiasVoltageController> biasController;
    boost::shared_ptr<TestControlNetwork> controlNetwork;
    boost::shared_ptr<psi::control::Shell> shell;
};
} // psi46expert
} // psi

int main(int argc, char* argv[])
{
    try
    {
        psi::ElectricPotential V = 0.0 * psi::volts;
        for (int i = 0; i < argc; i++)
        {
            if (!strcmp(argv[i],"-V"))
                V=atoi(argv[++i]) * psi::volts;
        }

        std::string testMode = "";
        std::string cmdFile = "";

        bool guiMode(false);
        parameters(argc, argv, cmdFile, testMode, guiMode);

        psi::psi46expert::Program program;
        program.Run();

        return 0;
    }
    catch(psi::exception& e)
    {
        psi::Log<psi::Error>(e.header()) << "ERROR: " << e.message() << std::endl;
        return 1;
    }
}
