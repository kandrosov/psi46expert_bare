/*!
 * \file DataStorage.cc
 * \brief Implementation of DataStorage class.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 25-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - DataStorage moved into psi namespace.
 *      - ROOT-related headers moved in DataStorage.cc from DataStorage.h.
 * 20-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#include <boost/scoped_ptr.hpp>

#include <TGraph.h>
#include <TFile.h>
#include <TParameter.h>

#include "psi/exception.h"

#include "DataStorage.h"

namespace psi {
namespace DataStorageInternals {
class File
{
public:
    File(const std::string& name) { tFile = new TFile(name.c_str(), "RECREATE"); }
    ~File() { tFile->Write(); tFile->Close(); delete tFile; }
    TFile& operator*() { return *tFile; }
    TFile* operator->() { return tFile; }
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
    : file(new DataStorageInternals::File(fileName)) {}

psi::DataStorage::~DataStorage()
{
}

void psi::DataStorage::SaveGraph(const std::string& name, const std::vector<IVoltageSource::Measurement>& measurements)
{
    std::vector<double> voltages(measurements.size()), currents(measurements.size());
    for(size_t n = 0; n < measurements.size(); ++n)
    {
        voltages[n] = ToStorageUnits(measurements[n].Voltage);
        currents[n] = ToStorageUnits(measurements[n].Current);
    }

    TGraph *graph = new TGraph(measurements.size(), &voltages[0], &currents[0]);
    graph->SetTitle(name.c_str());
    graph->SetName(name.c_str());
    graph->Write();
}

bool psi::DataStorage::_SaveMeasurement(const std::string& name, double value)
{
    boost::scoped_ptr< TParameter<double> > parameter(new TParameter<double>(name.c_str(), value));
    return parameter->Write() != 0;
}

