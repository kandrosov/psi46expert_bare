/*!
 * \file DataStorage.cc
 * \brief Implementation of DataStorage class.
 *
 * \b Changelog
 * 20-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#include <TGraph.h>

#include "BasePixel/psi_exception.h"

#include "DataStorage.h"

boost::shared_ptr<DataStorage> DataStorage::active;

DataStorage& DataStorage::Active()
{
    if(!hasActive())
        THROW_PSI_EXCEPTION("[DataStorage] Active data storage is not selected.");
    return *active;
}

bool DataStorage::hasActive()
{
    return active != 0;
}

void DataStorage::setActive(boost::shared_ptr<DataStorage> dataStorage)
{
    active = dataStorage;
    if(hasActive())
        active->file->cd();
}

DataStorage::DataStorage(const std::string& fileName)
    : file(new TFile(fileName.c_str(), "RECREATE")) {}

DataStorage::~DataStorage()
{
    file->Write();
    file->Close();
}

void DataStorage::SaveGraph(const std::string& name, const std::vector<IVoltageSource::Measurement>& measurements)
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
