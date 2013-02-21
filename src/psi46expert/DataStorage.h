/*!
 * \file DataStorage.h
 * \brief Definition of DataStorage class.
 *
 * \b Changelog
 * 20-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#pragma once

#include <boost/scoped_ptr.hpp>

#include <TFile.h>
#include <TParameter.h>

#include "BasePixel/GlobalConstants.h"
#include "BasePixel/IVoltageSource.h"

namespace DataStorageInternals
{
template<typename Value>
struct ConversionFactor {};

template<>
struct ConversionFactor<psi::ElectricCurrent>
{
    static const psi::ElectricCurrent& Factor()
    {
        static const psi::ElectricCurrent factor = 1. * psi::amperes;
        return factor;
    }
};

template<>
struct ConversionFactor<psi::ElectricPotential>
{
    static const psi::ElectricPotential& Factor()
    {
        static const psi::ElectricPotential factor = 1. * psi::volts;
        return factor;
    }
};

template<>
struct ConversionFactor<psi::Time>
{
    static const psi::Time& Factor()
    {
        static const psi::Time factor = 1. * psi::seconds;
        return factor;
    }
};

template<>
struct ConversionFactor<psi::CurrentPerTime>
{
    static const psi::CurrentPerTime& Factor()
    {
        static const psi::CurrentPerTime factor = ConversionFactor<psi::ElectricCurrent>::Factor()
                / ConversionFactor<psi::Time>::Factor();
        return factor;
    }
};


}

/*!
 * \brief Provides storage interface to save test results into the ROOT file.
 */
class DataStorage
{
public:
    template<typename Value>
    static double ToStorageUnits(const Value& v)
    {
        return v / DataStorageInternals::ConversionFactor<Value>::Factor();
    }

    template<typename Value>
    static Value FromStorageUnits(double v)
    {
        return v * DataStorageInternals::ConversionFactor<Value>::Factor();
    }

    template<typename Iterator>
    static std::vector<double> ToStorageUnits(const Iterator& begin, const Iterator& end)
    {
        std::vector<double> result;
        for(Iterator iter = begin; iter != end; ++iter)
            result.push_back(ToStorageUnits(*iter));
        return result;
    }

public:
    static DataStorage& Active();
    static bool hasActive();
    static void setActive(boost::shared_ptr<DataStorage> dataStorage);

    DataStorage(const std::string& fileName);
    ~DataStorage();

    /*!
     * Save a single measurement into the output ROOT file.
     */
    template<typename M>
    void SaveMeasurement(const std::string& name, const M& value)
    {
        const double storageValue = ToStorageUnits(value);
        boost::scoped_ptr< TParameter<double> > parameter(new TParameter<double>(name.c_str(), storageValue));
        if(!parameter->Write())
            THROW_PSI_EXCEPTION("ERROR: measurement '" << name << "' equal to '" << value
                                << "' can't be saved into the output ROOT file.");
    }

    void SaveGraph(const std::string& name, const std::vector<IVoltageSource::Measurement>& measurements);

private:
    static boost::shared_ptr<DataStorage> active;

    boost::scoped_ptr<TFile> file;
};
