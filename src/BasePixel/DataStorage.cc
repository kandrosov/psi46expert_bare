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
static const std::string DETECTOR_VALID_BRANCH = "valid";

namespace psi {
namespace DataStorageInternals {

class File {
public:
    File(const std::string& name) {
        tFile = new TFile(name.c_str(), "RECREATE");
    }
    ~File() {
        tFile->Close();
        delete tFile;
    }
    TFile& operator*() {
        return *tFile;
    }
    TFile* operator->() {
        return tFile;
    }
private:
    TFile* tFile;
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

bool psi::DataStorage::hasActive()
{
    return active != 0;
}

void psi::DataStorage::setActive(boost::shared_ptr<DataStorage> dataStorage)
{
    active = dataStorage;
    if(hasActive())
        (*active->file)->cd();
}

psi::DataStorage::DataStorage(const std::string& fileName)
    : file(new DataStorageInternals::File(fileName))
{
    static const std::string detectorSummaryTreeName = "detector_test_summary";
    detectorSummary = boost::shared_ptr<TTree>(new TTree(detectorSummaryTreeName.c_str(),
                      detectorSummaryTreeName.c_str()));
    detectorSummary->Branch(DETECTOR_NAME_BRANCH.c_str(), &detectorName);
    detectorSummary->Branch(OPERATOR_NAME_BRANCH.c_str(), &operatorName);
    detectorSummary->Branch(DATE_BRANCH.c_str(), &date);
    detectorSummary->Branch(DETECTOR_VALID_BRANCH.c_str(), &detectorValid);
    date = psi::log::detail::DateTimeProvider::Now();
    detectorValid = false;
}

psi::DataStorage::~DataStorage()
{
    detectorSummary->Fill();
    detectorSummary->Write();
}

bool psi::DataStorage::_SaveMeasurement(const std::string& name, double value)
{
    boost::scoped_ptr< TParameter<double> > parameter(new TParameter<double>(name.c_str(), value));
    return parameter->Write() != 0;
}

void psi::DataStorage::SetDetectorName(const std::string& _detectorName)
{
    detectorName = _detectorName;
}

void psi::DataStorage::SetOperatorName(const std::string& _operatorName)
{
    operatorName = _operatorName;
}

void psi::DataStorage::SetDetectorValidity(bool valid)
{
    detectorValid = valid;
}

void psi::DataStorage::EnterDirectory(const std::string& dirName)
{
    directoryHistory.push(dirName);
    (*file)->mkdir(dirName.c_str());
    (*file)->cd(dirName.c_str());
}

void psi::DataStorage::GoToPreviousDirectory()
{
    std::string dirName = "/";
    directoryHistory.pop();
    if(directoryHistory.size()) {
        dirName = directoryHistory.top();
    }
    (*file)->cd(dirName.c_str());
}
