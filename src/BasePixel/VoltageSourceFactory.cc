/*!
 * \file VoltageSourceFactory.cc
 * \brief Implementation of VoltageSourceFactory class.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using ThreadSafeVoltageSource.
 *      - IVoltageSource, ThreadSafeVoltageSource and VoltageSourceFactory moved into psi namespace.
 * 11-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#include <map>

#include "ConfigParameters.h"
#include "Keithley237.h"
#include "VoltageSourceFactory.h"

typedef psi::IVoltageSource* (*Maker)(const ConfigParameters&);
typedef std::map<std::string, Maker> MakerMap;

static psi::IVoltageSource* Keithley237Maker(const ConfigParameters& configParameters)
{
    const psi::Keithley237::Configuration keithleyConfig(configParameters.VoltageSourceDevice(),
                                                         configParameters.SetVoltageSourceToLocalModeOnExit(),
                                                         configParameters.NumberOfVoltageSourceReadingsToAverage(),
                                                         configParameters.VoltageSourceIntegrationTime());
    return new psi::Keithley237(keithleyConfig);
}

static MakerMap CreateMakerMap()
{
    MakerMap map;
    map["Keithley237"] = &Keithley237Maker;
    return map;
}

static const MakerMap makerMap = CreateMakerMap();

static psi::IVoltageSource* CreateVoltageSource()
{
    const ConfigParameters& configParameters = ConfigParameters::Singleton();
    MakerMap::const_iterator iter = makerMap.find(configParameters.VoltageSource());
    if(iter == makerMap.end())
        THROW_PSI_EXCEPTION("[VoltageSourceFactory] Voltage source '" << configParameters.VoltageSource()
                            << "' not found.");
    return iter->second(configParameters);
}

psi::VoltageSourceFactory::VoltageSourcePtr psi::VoltageSourceFactory::Get()
{
    static VoltageSourcePtr voltageSource(new ThreadSafeVoltageSource(CreateVoltageSource()));
    return voltageSource;
}
