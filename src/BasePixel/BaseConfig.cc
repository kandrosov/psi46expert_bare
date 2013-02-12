/*!
 * \file BaseConfig.cc
 * \brief Implementation of BaseConfig class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#include <fstream>
#include "psi_exception.h"
#include "BaseConfig.h"

using namespace psi;

void BaseConfig::Read(const std::string& fileName)
{
    std::ifstream f(fileName.c_str());
    if(!f.is_open())
        THROW_PSI_EXCEPTION("[BaseConfig::Read] Unable to read the configuration file '" << fileName << "'.");
    while(f.good())
    {
        std::string line;
        std::getline(f, line);
        if(!line.length() || line[0] == '#' || line[0] == '-')
            continue;

        std::istringstream istring( line);
        std::string name;
        std::string value;

        istring >> name >> value;

        if(istring.fail() || !name.length())
            continue;
        parameters[name] = value;
    }
}

void BaseConfig::Write(const std::string& fileName) const
{
    std::ofstream f(fileName.c_str());
    if(!f.is_open())
        THROW_PSI_EXCEPTION("[BaseConfig::Write] Unable to write the configuration into the file '" << fileName
                            << "'.");
    for(Map::const_iterator iter = parameters.begin(); iter != parameters.end(); ++iter)
    {
        f << iter->first << " " << iter->second << std::endl;
    }
}

