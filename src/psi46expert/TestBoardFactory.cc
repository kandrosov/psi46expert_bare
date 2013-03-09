/*!
 * \file TestBoardFactory.cc
 * \brief Implementation of TestBoardFactory class.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 09-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Corrected questionable language constructions, which was found using -Wall g++ option.
 * 06-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#include <map>

#include "psi/exception.h"

#include "BasePixel/ConfigParameters.h"
#include "BasePixel/AnalogTestBoard.h"
#include "BasePixel/FakeTestBoard.h"

#include "TestBoardFactory.h"

typedef TBAnalogInterface* (*AnalogMaker)();
typedef std::map<std::string, AnalogMaker> AnalogMakerMap;

static TBAnalogInterface* AnalogTestBoardMaker()
{
    return new AnalogTestBoard();
}

static TBAnalogInterface* FakeTestBoardMaker()
{
    return new FakeTestBoard();
}

static AnalogMakerMap CreateAnalogMakerMap()
{
    AnalogMakerMap map;
    map["Analog"] = &AnalogTestBoardMaker;
    map["Fake"] = &FakeTestBoardMaker;
    return map;
}

static const AnalogMakerMap analogMakerMap = CreateAnalogMakerMap();

psi::TestBoardFactory::AnalogTestBoardPtr psi::TestBoardFactory::MakeAnalog()
{
    const ConfigParameters& configParameters = ConfigParameters::Singleton();
    AnalogMakerMap::const_iterator iter = analogMakerMap.find(configParameters.TestboardType());
    if(iter == analogMakerMap.end())
        THROW_PSI_EXCEPTION("Test board type '" << configParameters.TestboardType() << "' is not supported.");

    return AnalogTestBoardPtr(iter->second());
}
