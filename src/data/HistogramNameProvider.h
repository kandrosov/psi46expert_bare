/*!
 * \file HistogramNameProvider.h
 * \brief Definition of standard histogram names.
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 */

#pragma once

#include <string>
#include <sstream>

namespace psi {
namespace data {

struct HistogramNameProvider {
    static std::string FullMapName(const std::string& mapName, unsigned chipId = 0, unsigned mapId = 0)
    {
        std::ostringstream fullMapName;
        fullMapName << mapName << "_C" << chipId;
        if(mapId)
            fullMapName << "_nb" << mapId;
        return fullMapName.str();
    }

    static std::string DistributionName(const std::string& fullMapName, unsigned distributionId = 0)
    {
        std::ostringstream distributionName;
        distributionName << fullMapName << "Distribution";
        if(distributionId)
            distributionName << "_nb" << distributionId;
        return distributionName.str();
    }

    static const std::string& VcalThresholdMapName() { static std::string name = "VcalThresholdMap"; return name; }
    static const std::string& VcalsThresholdMapName() { static std::string name = "VcalsThresholdMap"; return name; }
    static const std::string& XTalkMapName() { static std::string name = "XTalkMap"; return name; }
    static const std::string& NoiseMapName() { static std::string name = "NoiseMap"; return name; }
    static const std::string& CalXTalkMapName() { static std::string name = "CalXTalkMap"; return name; }
    static const std::string& CalThresholdMapName() { static std::string name = "CalThresholdMap"; return name; }

private:
    HistogramNameProvider(){}
};
} // data
} // psi
