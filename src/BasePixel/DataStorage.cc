/*!
 * \file DataStorage.cc
 * \brief Implementation of DataStorage class.
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 */

#include <boost/scoped_ptr.hpp>

#include <TGraph.h>
#include <TFile.h>
#include <TParameter.h>

#include "psi/exception.h"
#include "psi/log.h"

#include "DataStorage.h"

static const std::string DETECTOR_NAME_BRANCH = "detector_name";
static const std::string DATE_BRANCH = "date";
static const std::string OPERATOR_NAME_BRANCH = "operator_name";

namespace psi {
namespace DataStorageInternals {

class File {
public:
    File(const std::string& name)
        : tFile(new TFile(name.c_str(), "UPDATE", "", 9))
    {
        if(tFile->IsZombie())
            THROW_PSI_EXCEPTION("Unable to open the output ROOT file.");
    }
    TFile& operator*() {
        return *tFile;
    }
    TFile* operator->() {
        return tFile.get();
    }
private:
    boost::scoped_ptr<TFile> tFile;
};

} // DataStorageInternals
} // psi

boost::shared_ptr<psi::DataStorage> psi::DataStorage::active;

psi::DataStorage& psi::DataStorage::Active()
{
    if(!hasActive())
        THROW_PSI_EXCEPTION("Active data storage is not selected.");
    return *active;
}

psi::DataStorage::DataStorage(const std::string& _fileName, const std::string& _detectorName,
                              const std::string& _operatorName)
    : fileName(_fileName)
{
    static const std::string detectorSummaryTreeName = "detector_test_summary";

    std::string detectorName = _detectorName;
    std::string operatorName = _operatorName;
    std::string date = psi::DateTimeProvider::StartTime();

    Enable();
    boost::shared_ptr<TTree> detectorSummary(new TTree(detectorSummaryTreeName.c_str(),
                      detectorSummaryTreeName.c_str()));
    detectorSummary->SetDirectory(0);
    detectorSummary->Branch(DETECTOR_NAME_BRANCH.c_str(), &detectorName);
    detectorSummary->Branch(OPERATOR_NAME_BRANCH.c_str(), &operatorName);
    detectorSummary->Branch(DATE_BRANCH.c_str(), &date);
    detectorSummary->Fill();
    detectorSummary->Write();
    Disable();
}

void psi::DataStorage::Enable()
{
    if(!file)
        file = boost::shared_ptr<DataStorageInternals::File>(new DataStorageInternals::File(fileName));
    (*file)->cd();
}

void psi::DataStorage::Disable()
{
    file = boost::shared_ptr<DataStorageInternals::File>();
}

bool psi::DataStorage::_SaveMeasurement(const std::string& name, double value)
{
    if(!Enabled())
        THROW_PSI_EXCEPTION("Data storage is not enabled.");
    boost::scoped_ptr< TParameter<double> > parameter(new TParameter<double>(name.c_str(), value));
    return parameter->Write() != 0;
}

void psi::DataStorage::EnterDirectory(const std::string& dirName)
{
    if(!Enabled())
        THROW_PSI_EXCEPTION("Data storage is not enabled.");

    directoryHistory.push(dirName);
    (*file)->mkdir(dirName.c_str());
    (*file)->cd(dirName.c_str());
}

void psi::DataStorage::GoToPreviousDirectory()
{
    if(!Enabled())
        THROW_PSI_EXCEPTION("Data storage is not enabled.");

    std::string dirName = "/";
    directoryHistory.pop();
    if(directoryHistory.size()) {
        dirName = directoryHistory.top();
    }
    (*file)->cd(dirName.c_str());
}
