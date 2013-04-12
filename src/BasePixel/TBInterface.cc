/*!
 * \file TBInterface.cc
 * \brief Implementation of TBInterface class.
 *
 * \b Changelog
 * 12-04-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Refactoring of TBParameters class.
 */

#include "BasePixel/TBInterface.h"
#include "psi/log.h"
#include "psi/exception.h"

TBInterface::TBInterface()
    : tbParameters(new TBParameters())
{
    ChipId = 0;
    TBMpresent = 0;
    HUBaddress = 0;
}

// == Parameters ================================================


const TBParameters& TBInterface::GetTBParameters()
{
    return *tbParameters;
}

int TBInterface::GetParameter(TBParameters::Register reg)
{
    int value;
    if(!tbParameters->Get(reg, value))
        THROW_PSI_EXCEPTION("TB register " << reg << " is not set.");
    return value;
}

// -- Saves the testboard parameters for later use
void TBInterface::SaveTBParameters()
{
    savedTBParameters = boost::shared_ptr<TBParameters>(new TBParameters(*tbParameters));
}


// -- Reads the testboard parameters from a file
void TBInterface::ReadTBParameterFile(const std::string& fileName)
{
    tbParameters->Read(fileName);
}


// -- Writes the testboard parameters to a file
void TBInterface::WriteTBParameterFile(const std::string& fileName)
{
    tbParameters->Write(fileName);
}


// -- Checks, if a TBM is present
bool TBInterface::TBMIsPresent()
{
    return TBMpresent;
}


// == Helper functions =============================================================

int TBInterface::COLCODE(int x)
{
    return ((x >> 1) & 0x7e) ^ x;
}


int TBInterface::ROWCODE(int x)
{
    return (x >> 1) ^ x;
}


int TBInterface::RangeCheck(int value, int min, int max)
{
    if(value < min) {
        psi::LogInfo() << "[TBInterface] Value too low. Register set to minimum (="
                       << min << ")." << std::endl;
        return min;
    }
    if(value > max) {
        psi::LogInfo() << "[TBInterface] Value too hight. Register set to maximum (="
                       << max << ")." << std::endl;
        return max;
    }
    return value;
}
