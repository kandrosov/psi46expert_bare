/*!
 * \file psi46report.cpp
 * \brief Main entrence for psi46report program.
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 * Creates a report from ROOT file with measurements made by psi46expert.
 */

#include <iostream>
#include <boost/program_options.hpp>
#include <TFile.h>
#include "psi/exception.h"
#include "RootPrintToPdf.h"

namespace {
const int NORMAL_EXIT_CODE = 0;
const int ERROR_EXIT_CODE = 1;
const int PRINT_ARGS_EXIT_CODE = 2;

const std::string optHelp = "help";
const std::string optInputFile = "input";
const std::string optOutputFile = "output";

static boost::program_options::options_description CreateProgramOptions()
{
    using boost::program_options::value;
    boost::program_options::options_description desc("Available command line arguments");
    desc.add_options()
            (optHelp.c_str(), "print help message")
            (optInputFile.c_str(), value<std::string>(), "input ROOT file with measurements")
            (optOutputFile.c_str(), value<std::string>(), "output PDF file with a report");
    return desc;
}

bool ParseProgramArguments(int argc, char* argv[], std::string& inputFileName, std::string& outputFileName)
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

    if(!variables.count(optInputFile)) {
        std::cerr << "Please, specify input ROOT file.\n\n" << description << std::endl;
        return false;
    }
    inputFileName = variables[optInputFile].as<std::string>();

    if(!variables.count(optOutputFile)) {
        std::cerr << "Please, specify output PDF file.\n\n" << description << std::endl;
        return false;
    }
    outputFileName = variables[optOutputFile].as<std::string>();

    return true;
}
} // anonymous namespace

namespace psi {
namespace psi46report {
class Program {
public:
    Program(const std::string& inputFileName, const std::string& outputFileName)
        : inputFile(inputFileName.c_str(), "READ"), printer(outputFileName)
    {
        if(inputFile.IsZombie())
            THROW_PSI_EXCEPTION("Unable to open input ROOT file '" << inputFileName << "'.");
    }

    void Run()
    {
        std::cout << "Program::Run()" << std::endl;
    }

private:
    TFile inputFile;
    root_ext::PdfPrinter printer;
};
} // psi46report
} // psi

int main(int argc, char* argv[])
{
    try {
        std::string inputFileName, outputFileName;
        if(!ParseProgramArguments(argc, argv, inputFileName, outputFileName))
            return PRINT_ARGS_EXIT_CODE;
        psi::psi46report::Program program(inputFileName, outputFileName);
        program.Run();
    } catch(psi::exception& e) {
        std::cerr << "ERROR: " << e.message() << std::endl;
        return ERROR_EXIT_CODE;
    }
    return NORMAL_EXIT_CODE;
}
