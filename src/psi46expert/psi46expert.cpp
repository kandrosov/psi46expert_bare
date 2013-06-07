/*!
 * \file psi46expert.cpp
 * \brief Main entrence for psi46expert.
 */

#include <boost/thread.hpp>
#include <signal.h>

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
static const char *dtlTest = "dtlScan";

static const std::string LOG_HEAD = "psi46expert";

//void runTest(TBInterface* tbInterface, TestControlNetwork* controlNetwork, SysCommand& sysCommand, const char* testMode)
//{
//  if (tbInterface->IsPresent() < 1)
//  {
//    psi::LogInfo() << "Error!! Testboard not present. Aborting" << std::endl;
//    return;
//  }
//  gDelay->Timestamp();
//  if (strcmp(testMode, fullTest) == 0)
//  {
//    psi::LogInfo() << "[psi46expert] SvFullTest and Calibration: start." << std::endl;

//    controlNetwork->FullTestAndCalibration();

//    psi::LogInfo() << "[psi46expert] SvFullTest and Calibration: end." << std::endl;
//  }
//  if (strcmp(testMode, shortTest) == 0)
//  {
//    psi::LogInfo() << "[psi46expert] SvShortTest: start." << std::endl;

//    controlNetwork->ShortCalibration();

//    psi::LogInfo() << "[psi46expert] SvShortTest: end." << std::endl;
//  }
//  if (strcmp(testMode, shortCalTest) == 0)
//  {
//    psi::LogInfo() << "[psi46expert] SvShortTest and Calibration: start." << std::endl;

//    controlNetwork->ShortTestAndCalibration();

//    psi::LogInfo() << "[psi46expert] SvShortTest and Calibration: end." << std::endl;
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
    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-dir")) {
            sprintf(directory, argv[++i]);
        }
        if (!strcmp(argv[i], "-c")) {
            rootFileArg = true;
            sprintf(rootFile, Form("test-%s.root", argv[++i]));
        }
        if (!strcmp(argv[i], "-d")) {
            dacArg = true;
            sprintf(dacFile, "%s", argv[++i]);
        }
        if (!strcmp(argv[i], "-r")) {
            rootFileArg = true;
            sprintf(rootFile, "%s", argv[++i]);
        }
        if (!strcmp(argv[i], "-f")) {
            cmdFileArg = true;
            std::stringstream ss;
            ss << argv[++i];
            cmdFile = ss.str();
        }
        if (!strcmp(argv[i], "-log")) {
            logFileArg = true;
            sprintf(logFile, "%s", argv[++i]);
        }
        if (!strcmp(argv[i], "-trim")) {
            trimArg = true;
            sprintf(trimFile, "%s", argv[++i]);
        }
        if (!strcmp(argv[i], "-trimVcal")) {
            trimArg = true;
            dacArg = true;
            int vcal = atoi(argv[++i]);
            sprintf(trimFile, "%s%i", "trimParameters", vcal);
            sprintf(dacFile, "%s%i", "dacParameters", vcal);
        }
        if (!strcmp(argv[i], "-mask")) {
            maskArg = true;
            sprintf(maskFile, "%s", "pixelMask.dat" ); //argv[++i]);
        }
        if (!strcmp(argv[i], "-tb")) {
            tbArg = true;
            sprintf(tbName, "%s", argv[++i]);
        }
        if (!strcmp(argv[i], "-t")) {
            testMode = argv[++i];
            if (strcmp(testMode.c_str(), dtlTest) == 0) {
                hubIdArg = true;
                hubId = -1;
            }
        }
        if (!strcmp(argv[i], "-g")) guiMode = true;

    }
    configParameters.setDirectory(directory);

    if (strcmp(testMode.c_str(), fullTest) == 0) {
        logFileArg = true;
        sprintf(logFile, "FullTest.log");
        rootFileArg = true;
        sprintf(rootFile, "FullTest.root");
    }
    if (strcmp(testMode.c_str(), shortTest) == 0 || strcmp(testMode.c_str(), shortCalTest) == 0) {
        logFileArg = true;
        sprintf(logFile, "ShortTest.log");
        rootFileArg = true;
        sprintf(rootFile, "ShortTest.root");
    } else if (strcmp(testMode.c_str(), calTest) == 0) {
        logFileArg = true;
        sprintf(logFile, "Calibration.log");
        rootFileArg = true;
        sprintf(rootFile, "Calibration.root");
    }

    if (logFileArg) configParameters.setLogFileName(logFile);
    else configParameters.setLogFileName( "log.txt");

    configParameters.setDebugFileName( "debug.log");

    psi::LogInfo ().open( configParameters.FullLogFileName() );
    psi::LogDebug().open( configParameters.FullDebugFileName() );

    psi::LogInfo(LOG_HEAD) << "--------- psi46expert ---------" << std::endl;
    psi::LogInfo(LOG_HEAD).PrintTimestamp();

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
class BiasThread {
public:
    BiasThread(boost::shared_ptr<psi::BiasVoltageController> biasController)
        : controller(biasController),
          thread(boost::bind(&psi::BiasVoltageController::operator(), biasController.get())) {}
    ~BiasThread() {
        controller->DisableControl();
        controller->DisableBias();
        controller->Stop();
        thread.join();
    }

private:
    boost::shared_ptr<psi::BiasVoltageController> controller;
    boost::thread thread;
};
struct SignalHandler {
    typedef boost::function< void () > Handler;
    static Handler& OnInterrupt() {
        static Handler h;
        return h;
    }
    static void interrupt_handler(int) {
        if(OnInterrupt())
            OnInterrupt()();
    }
};
} // detail

class Program {
public:
    Program()
        : haveCompliance(false), haveError(false), interruptRequested(false) {
        detail::SignalHandler::OnInterrupt() = boost::bind(&Program::OnInterrupt, this);
        signal(SIGINT, &detail::SignalHandler::interrupt_handler);
        const ConfigParameters& configParameters = ConfigParameters::Singleton();

        dataStorage = boost::shared_ptr<psi::DataStorage>(new psi::DataStorage(configParameters.FullRootFileName()));
        psi::DataStorage::setActive(dataStorage);

        tbInterface = psi::TestBoardFactory::MakeAnalog();
        if (!tbInterface->IsPresent())
            THROW_PSI_EXCEPTION("Unable to connect to the test board.");

        biasController = boost::shared_ptr<psi::BiasVoltageController>(
                             new psi::BiasVoltageController(boost::bind(&Program::OnCompliance, this, _1),
                                     boost::bind(&Program::OnError, this, _1)));
        controlNetwork = boost::shared_ptr<psi::control::TestControlNetwork>(
                             new psi::control::TestControlNetwork(tbInterface, biasController));
        shell = boost::shared_ptr<psi::control::Shell>(new psi::control::Shell(".psi46expert_history", controlNetwork));
    }

    void Run() {
        detail::BiasThread biasControllerThread(biasController);
//        biasController->EnableBias();
//        biasController->EnableControl();

        bool canRun = true;
        bool printHelpLine = true;
        while(canRun) {
            shell->Run(printHelpLine);
            boost::lock_guard<boost::mutex> lock(mutex);
            if(!haveError && haveCompliance) {
                biasController->DisableBias();
                haveCompliance = false;
                printHelpLine = false;
            } else if(!haveError && interruptRequested) {
                interruptRequested = false;
                printHelpLine = false;
            } else
                canRun = false;
        }
    }

private:
    void OnCompliance(const psi::IVoltageSource::Measurement&) {
        boost::lock_guard<boost::mutex> lock(mutex);
        LogError() << std::endl;
        LogError(LOG_HEAD) << "ERROR: compliance is reached. Any running test will be aborted."
                           " Bias voltages will be switched off." << std::endl;
        shell->InterruptExecution();
        biasController->DisableControl();
        haveCompliance = true;
    }

    void OnError(const std::exception& e) {
        boost::lock_guard<boost::mutex> lock(mutex);
        LogError() << std::endl;
        LogError(LOG_HEAD) << "CRITICAL ERROR in the bias control thread." << std::endl << e.what() << std::endl
                           << "Program will be terminated." << std::endl;
        shell->InterruptExecution();
        biasController->DisableControl();
        haveError = true;
    }

    void OnInterrupt() {
        boost::lock_guard<boost::mutex> lock(mutex);
        LogError() << std::endl;
        LogError(LOG_HEAD) << "Test interruption request by user. Any running test will be aborted." << std::endl;
        shell->InterruptExecution();
        interruptRequested = true;
    }

private:
    boost::mutex mutex;
    bool haveCompliance, haveError, interruptRequested;
    boost::shared_ptr<psi::DataStorage> dataStorage;
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    boost::shared_ptr<psi::BiasVoltageController> biasController;
    boost::shared_ptr<psi::control::TestControlNetwork> controlNetwork;
    boost::shared_ptr<psi::control::Shell> shell;
};
} // psi46expert
} // psi

int main(int argc, char* argv[])
{
    try {
        std::string testMode = "";
        std::string cmdFile = "";

        bool guiMode(false);
        parameters(argc, argv, cmdFile, testMode, guiMode);

        psi::psi46expert::Program program;
        program.Run();

        return 0;
    } catch(psi::exception& e) {
        psi::LogError(e.header()) << "ERROR: " << e.message() << std::endl;
        return 1;
    }
}
