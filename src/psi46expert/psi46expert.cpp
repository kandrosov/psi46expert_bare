/*!
 * \file psi46expert.cpp
 * \brief Main entrence for psi46expert.
 */

#include <iostream>
#include <boost/thread.hpp>
#include <boost/program_options.hpp>
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

namespace {
const int NORMAL_EXIT_CODE = 0;
const int ERROR_EXIT_CODE = 1;
const int PRINT_ARGS_EXIT_CODE = 2;

const std::string LOG_HEAD = "psi46expert";

const std::string optHelp = "help";
const std::string optOperator = "operator";
const std::string optDetector = "detector";
const std::string optWorkingDirectory = "dir";
const std::string optRootFileName = "root";
const std::string optLogFileName = "log";

const std::string CONFIG_FILE_NAME = "configParameters.dat";
const std::string DEFAULT_ROOT_FILE_NAME = "Test.root";
const std::string DEFAULT_LOG_FILE_NAME = "Test.log";
const std::string DEFAULT_DEBUG_LOG_FILE_NAME = "Debug.log";
const std::string HISTORY_FILE_NAME = ".psi46expert_history";

static boost::program_options::options_description CreateProgramOptions()
{
    using boost::program_options::value;
    boost::program_options::options_description desc("Available command line arguments");
    desc.add_options()
            (optHelp.c_str(), "print help message")
            (optOperator.c_str(), value<std::string>(), "set operator name")
            (optDetector.c_str(), value<std::string>(), "set full detector name")
            (optWorkingDirectory.c_str(), value<std::string>(), "set working directory")
            (optRootFileName.c_str(), value<std::string>(), "set ROOT file name")
            (optLogFileName.c_str(), value<std::string>(), "set log file name");
    return desc;
}

bool ParseProgramArguments(int argc, char* argv[])
{
    using namespace boost::program_options;
    static options_description description = CreateProgramOptions();
    variables_map variables;

    try {
        store(parse_command_line(argc, argv, description), variables);
        notify(variables);
    }
    catch(error& e) {
        std::cerr << "ERROR: " << e.what() << ".\n\n" << description << std::endl;
        return false;
    }

    if(variables.count(optHelp)) {
        std::cout << description << std::endl;
        return false;
    }

    if(!variables.count(optOperator)) {
        std::cerr << "Please, specify operator name.\n\n" << description << std::endl;
        return false;
    }
    const std::string operatorName = variables[optOperator].as<std::string>();

    if(!variables.count(optDetector)) {
        std::cerr << "Please, specify detector name.\n\n" << description << std::endl;
        return false;
    }
    const std::string detectorName = variables[optDetector].as<std::string>();

    const std::string workingDirectory = variables.count(optWorkingDirectory) ?
                variables[optWorkingDirectory].as<std::string>() : ".";
    ConfigParameters& configParameters = ConfigParameters::ModifiableSingleton();
    configParameters.setDirectory(workingDirectory);

    const std::string logFile = variables.count(optLogFileName) ?
                variables[optLogFileName].as<std::string>() : DEFAULT_LOG_FILE_NAME;
    configParameters.setLogFileName(logFile);
    configParameters.setDebugFileName(DEFAULT_DEBUG_LOG_FILE_NAME);

    psi::LogInfo ().open( configParameters.FullLogFileName() );
    psi::LogDebug().open( configParameters.FullDebugFileName() );

    psi::LogInfo(LOG_HEAD) << "Starting... " << psi::LogInfo::FullTimestampString() << std::endl;

    const std::string configFile = workingDirectory + "/" + CONFIG_FILE_NAME;
    configParameters.Read(configFile);

    const std::string rootFile = variables.count(optRootFileName) ?
                variables[optRootFileName].as<std::string>() : DEFAULT_ROOT_FILE_NAME;
    configParameters.setRootFileName(rootFile);

    boost::shared_ptr<psi::DataStorage> dataStorage(new psi::DataStorage(configParameters.FullRootFileName()));
    psi::DataStorage::setActive(dataStorage);
    dataStorage->SetOperatorName(operatorName);
    dataStorage->SetDetectorName(detectorName);

    return true;
}

} // anonymous namespace

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

        tbInterface = psi::TestBoardFactory::MakeAnalog();
        if (!tbInterface->IsPresent())
            THROW_PSI_EXCEPTION("Unable to connect to the test board.");

        biasController = boost::shared_ptr<psi::BiasVoltageController>(
                             new psi::BiasVoltageController(boost::bind(&Program::OnCompliance, this, _1),
                                     boost::bind(&Program::OnError, this, _1)));
        controlNetwork = boost::shared_ptr<psi::control::TestControlNetwork>(
                             new psi::control::TestControlNetwork(tbInterface, biasController));
        shell = boost::shared_ptr<psi::control::Shell>(new psi::control::Shell(HISTORY_FILE_NAME, controlNetwork));
    }

    void Run() {
        detail::BiasThread biasControllerThread(biasController);

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
        LogError(LOG_HEAD) << "CRITICAL ERROR in the bias control thread.\n" << e.what()
                           << "\nProgram will be terminated." << std::endl;
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
        if(!ParseProgramArguments(argc, argv))
            return PRINT_ARGS_EXIT_CODE;
        psi::psi46expert::Program program;
        program.Run();
    } catch(psi::exception& e) {
        psi::LogError(e.header()) << "ERROR: " << e.message() << std::endl;
        return ERROR_EXIT_CODE;
    }
    return NORMAL_EXIT_CODE;
}
