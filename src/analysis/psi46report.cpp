/*!
 * \file psi46report.cpp
 * \brief Main entrence for psi46report program.
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 * Creates a report from ROOT file with measurements made by psi46expert.
 */

#include <iostream>
#include <boost/program_options.hpp>
#include <TFile.h>
#include <TKey.h>
#include <TError.h>
#include "psi/exception.h"
#include "RootPrintToPdf.h"
#include "data/TestNameProvider.h"
#include "data/HistogramNameProvider.h"

namespace psi {
namespace psi46report {
namespace detail {
class Source2D : public root_ext::SimpleHistogramSource<TH2D> {
public:
    double minValue;
    Source2D() : minValue(std::numeric_limits<double>::min()) {}
protected:
    virtual void Prepare(TH2D* histogram, const std::string& display_name,
                         const PlotOptions& plot_options) const
    {
        root_ext::SimpleHistogramSource<TH2D>::Prepare(histogram, display_name, plot_options);
        if(minValue != std::numeric_limits<double>::min())
            histogram->SetMinimum(minValue);
    }
};
} //detail

class Program {
private:
    typedef std::vector<std::string> str_vector;
public:
    struct Config {
        std::string inputFileName;
        std::string outputFileName;
        bool compatibilityMode;
        Config() : compatibilityMode(false) {}
    };

public:
    Program(const Config& _config)
        : config(_config), inputFile(config.inputFileName.c_str(), "READ")
    {
        if(inputFile.IsZombie())
            THROW_PSI_EXCEPTION("Unable to open input ROOT file '" << config.inputFileName << "'.");
        const size_t m_pos = config.inputFileName.find_first_of('/');
        const size_t c_pos = config.inputFileName.find_first_of('/', m_pos + 1);
        _moduleName.append(config.inputFileName.begin() + m_pos + 1, config.inputFileName.begin() + c_pos);
        _chipName.append(config.inputFileName.begin() + c_pos + 1, config.inputFileName.begin() + c_pos + 4);
    }

    void Run()
    {
        if(config.compatibilityMode) {
            MakeBumpBondingTestReport("", _moduleName, _chipName);
            return;
        }


        const str_vector bumpBondingTestNames = FindTest(psi::data::TestNameProvider::BumpBondingTestName());
        if(!bumpBondingTestNames.size())
            THROW_PSI_EXCEPTION("Test '" << psi::data::TestNameProvider::BumpBondingTestName() << "' not found." );
        for(str_vector::const_iterator iter = bumpBondingTestNames.begin(); iter != bumpBondingTestNames.end(); ++iter){
            MakeBumpBondingTestReport((*iter) + "/", _moduleName, _chipName);
        }
    }

private:
    str_vector FindTest(const std::string& shortName) const
    {
        str_vector result;
        TIter nextkey(inputFile.GetListOfKeys());
        for(TKey* key; (key = (TKey*)nextkey()); ) {
            const std::string fullName(key->GetName());
            if(fullName.find(shortName) != std::string::npos)
                result.push_back(fullName);
        }
        return result;
    }

    void MakeBumpBondingTestReport(const std::string& dirName, const std::string& moduleName,
                                   const std::string& rocName)
    {
        root_ext::PdfPrinter printer(config.outputFileName);
        root_ext::SingleSidedPage page(true, true, false);
        const std::string titlePrefix = "Module " + moduleName + ". ROC " + rocName + ". ";

        typedef root_ext::SimpleHistogramSource<TH1D> Source1D;
        typedef detail::Source2D Source2D;

        {
            Source1D vcals_xtalk_source;
            vcals_xtalk_source.Add("vcals_xtalk_C0Distribution", &inputFile);
            page.side.histogram_name = dirName + "vcals_xtalk_C0Distribution";
            page.side.histogram_title = "Vcals - XTalk distribution";
            page.side.draw_options = "";
            page.title = titlePrefix + "Vcals - XTalk distribution";
            printer.Print(page, vcals_xtalk_source);

            Source1D cal_xtalk_source;
            cal_xtalk_source.Add("CalXTalkMap_C0Distribution", &inputFile);
            page.side.histogram_name = dirName + "CalXTalkMap_C0Distribution";
            page.side.histogram_title = "CalXTalk distribution";
            page.title = titlePrefix + "CalXTalk distribution";
            printer.Print(page, cal_xtalk_source);

            Source1D vcals_source;
            vcals_source.Add("VcalsThresholdMap_C0Distribution", &inputFile);
            page.side.histogram_name = dirName + "VcalsThresholdMap_C0Distribution";
            page.side.histogram_title = "Vcals threshold distribution";
            page.title = titlePrefix + "Vcals threshold distribution";
            printer.Print(page, vcals_source);

            Source1D xtalk_source;
            xtalk_source.Add("XTalkMap_C0Distribution", &inputFile);
            page.side.histogram_name = dirName + "XTalkMap_C0Distribution";
            page.side.histogram_title = "XTalk distribution";
            page.title = titlePrefix + "XTalk distribution";
            printer.Print(page, xtalk_source);
        }

        {
            Source2D vcals_xtalk_source;
            vcals_xtalk_source.Add("vcals_xtalk_C0", &inputFile);
            page.side.histogram_name = dirName + "vcals_xtalk_C0";
            page.side.histogram_title = "Vcals - XTalk map";
            page.side.draw_options = "colz";
            page.side.fit_range = false;
            page.title = titlePrefix + "Vcals - XTalk map";
            printer.Print(page, vcals_xtalk_source);
            vcals_xtalk_source.minValue = -5;
            printer.Print(page, vcals_xtalk_source);
            vcals_xtalk_source.minValue = -10;
            printer.Print(page, vcals_xtalk_source);

            Source2D cal_xtalk_source;
            cal_xtalk_source.Add("CalXTalkMap_C0", &inputFile);
            page.side.histogram_name = dirName + "CalXTalkMap_C0";
            page.side.histogram_title = "CalXTalk map";
            page.title = titlePrefix + "CalXTalk map";
            printer.Print(page, cal_xtalk_source);

            Source2D vcals_source;
            vcals_source.Add("VcalsThresholdMap_C0", &inputFile);
            page.side.histogram_name = dirName + "VcalsThresholdMap_C0";
            page.side.histogram_title = "Vcals threshold map";
            page.title = titlePrefix + "Vcals threshold map";
            printer.Print(page, vcals_source);

            Source2D xtalk_source;
            xtalk_source.Add("XTalkMap_C0", &inputFile);
            page.side.histogram_name = dirName + "XTalkMap_C0";
            page.side.histogram_title = "XTalk map";
            page.title = titlePrefix + "XTalk map";
            printer.Print(page, xtalk_source);
        }
    }

private:
    Config config;
    TFile inputFile;
    std::string _chipName;
    std::string _moduleName;
};
} // psi46report
} // psi

namespace {
const int NORMAL_EXIT_CODE = 0;
const int ERROR_EXIT_CODE = 1;
const int PRINT_ARGS_EXIT_CODE = 2;

const std::string optHelp = "help";
const std::string optInputFile = "input";
const std::string optOutputFile = "output";
const std::string optCompatibility = "compatibility";

static boost::program_options::options_description CreateProgramOptions()
{
    using boost::program_options::value;
    boost::program_options::options_description desc("Available command line arguments");
    desc.add_options()
            (optHelp.c_str(), "print help message")
            (optInputFile.c_str(), value<std::string>(), "input ROOT file with measurements")
            (optOutputFile.c_str(), value<std::string>(), "output PDF file with a report")
            (optCompatibility.c_str(), "run program in compatibility mode to read old psi46expert files");
    return desc;
}

bool ParseProgramArguments(int argc, char* argv[], psi::psi46report::Program::Config& config)
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
    config.inputFileName = variables[optInputFile].as<std::string>();

    if(!variables.count(optOutputFile)) {
        std::cerr << "Please, specify output PDF file.\n\n" << description << std::endl;
        return false;
    }
    config.outputFileName = variables[optOutputFile].as<std::string>();

    config.compatibilityMode = variables.count(optCompatibility);

    return true;
}
} // anonymous namespace


int main(int argc, char* argv[])
{
    try {
        gErrorIgnoreLevel = kWarning;
        gPrintViaErrorHandler = false;

        psi::psi46report::Program::Config config;
        if(!ParseProgramArguments(argc, argv, config))
            return PRINT_ARGS_EXIT_CODE;
        psi::psi46report::Program program(config);
        program.Run();
    } catch(psi::exception& e) {
        std::cerr << "ERROR: " << e.message() << std::endl;
        return ERROR_EXIT_CODE;
    } catch(std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return ERROR_EXIT_CODE;
    }

    return NORMAL_EXIT_CODE;
}
