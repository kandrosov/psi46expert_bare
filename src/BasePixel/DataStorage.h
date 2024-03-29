/*!
 * \file DataStorage.h
 * \brief Definition of DataStorage class.
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 */

#pragma once

#include <stack>
#include <boost/shared_ptr.hpp>

#include "psi/exception.h"
#include "interface/IVoltageSource.h"
#include "BasePixel/constants.h"

#include <TTree.h>

namespace psi {
namespace DataStorageInternals {

class File;

template<typename Value>
struct ConversionFactor {};

template<>
struct ConversionFactor<psi::ElectricCurrent> {
    static const psi::ElectricCurrent& Factor() {
        static const psi::ElectricCurrent factor = 1. * psi::amperes;
        return factor;
    }
};

template<>
struct ConversionFactor<psi::ElectricPotential> {
    static const psi::ElectricPotential& Factor() {
        static const psi::ElectricPotential factor = 1. * psi::volts;
        return factor;
    }
};

template<>
struct ConversionFactor<psi::Time> {
    static const psi::Time& Factor() {
        static const psi::Time factor = 1. * psi::seconds;
        return factor;
    }
};

template<>
struct ConversionFactor<psi::CurrentPerTime> {
    static const psi::CurrentPerTime& Factor() {
        static const psi::CurrentPerTime factor = ConversionFactor<psi::ElectricCurrent>::Factor()
                / ConversionFactor<psi::Time>::Factor();
        return factor;
    }
};

} // DataStorageInternals

/*!
 * \brief Provides storage interface to save test results into the ROOT file.
 */
class DataStorage {
public:
    template<typename Value>
    static double ToStorageUnits(const Value& v) {
        return v / DataStorageInternals::ConversionFactor<Value>::Factor();
    }

    template<typename Value>
    static Value FromStorageUnits(double v) {
        return v * DataStorageInternals::ConversionFactor<Value>::Factor();
    }

    template<typename Iterator>
    static std::vector<double> ToStorageUnits(const Iterator& begin, const Iterator& end) {
        std::vector<double> result;
        for(Iterator iter = begin; iter != end; ++iter)
            result.push_back(ToStorageUnits(*iter));
        return result;
    }

public:
    static DataStorage& Active();
    static bool hasActive() { return active != nullptr; }
    static void setActive(boost::shared_ptr<DataStorage> dataStorage) { active = dataStorage; }

    DataStorage(const std::string& fileName, const std::string& detectorName, const std::string& operatorName);

    void EnterDirectory(const std::string &dirName);
    void GoToPreviousDirectory();
    bool Enabled() const { return file != nullptr; }
    void Enable();
    void Disable();

    /*!
     * Save a single measurement into the output ROOT file.
     */
    template<typename M>
    void SaveMeasurement(const std::string& name, const M& value) {
        const double storageValue = ToStorageUnits(value);
        if(!_SaveMeasurement(name, storageValue))
            THROW_PSI_EXCEPTION("Measurement '" << name << "' equal to '" << value << "' can't be saved into the output"
                                " ROOT file.");
    }

private:
    bool _SaveMeasurement(const std::string& name, double value);

private:
    static boost::shared_ptr<DataStorage> active;
    std::string fileName;
    boost::shared_ptr<DataStorageInternals::File> file;
    bool detectorValid;
    std::stack<std::string> directoryHistory;
};

} // psi
