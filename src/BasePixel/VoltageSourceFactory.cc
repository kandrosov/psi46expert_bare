/*!
 * \file VoltageSourceFactory.cc
 * \brief Implementation of VoltageSourceFactory class.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 11-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#include <map>

#include "ConfigParameters.h"
#include "Keithley237.h"
#include "VoltageSourceFactory.h"

typedef boost::shared_ptr<IVoltageSource> (*Maker)(const ConfigParameters&);
typedef std::map<std::string, Maker> MakerMap;


boost::shared_ptr<IVoltageSource> Keithley237Maker(const ConfigParameters& configParameters)
{
    const Keithley237::Configuration keithleyConfig(configParameters.Volta);
    return boost::shared_ptr<IVoltageSource>(new Keithley237(keithleyConfig))
}

static MakerMap CreateMakerMap()
{
    MakerMap map;
    map["Keithley237"]
    return map;
}

static const MakerMap makerMap = CreateMakerMap();

static boost::shared_ptr<IVoltageSource> CreateVoltageSource()
{
    ConfigParameters* config = ConfigParameters::Singleton();
    static const std::string name = "keithley";
}

static boost::shared_ptr<IVoltageSource> VoltageSourceFactory::Get()
{
    static boost::shared_ptr<IVoltageSource> voltageSource = CreateVoltageSource();
    return voltageSource;
}
