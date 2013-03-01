/*!
 * \file TBParameters.cc
 * \brief Implementation of TBParameters class.
 *
 * \b Changelog
 * 01-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using a new PSI Logging System.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - removed deprecated conversion from string constant to char*
 */

#include <fstream>
#include <sstream>
#include <string>

#include <string.h>
#include <stdio.h>

#include "BasePixel/TBParameters.h"
#include "BasePixel/TBAnalogInterface.h"
#include "interface/Log.h"

TBParameters::TBParameters(TBInterface *aTBInterface) 
{
	tbInterface = aTBInterface;

	for (int i = 0; i < NTBParameters; i++) {
		parameters[i] = -1;
		names[i] = "";
	}
}


bool TBParameters::Execute(SysCommand command)
{
	for (int iDAC = 0; iDAC < NTBParameters; iDAC++)
	{

        if ( (strcmp(names[iDAC].c_str(),"") != 0) && (strcmp(command.carg[0],names[iDAC].c_str()) == 0))
		{
			SetParameter(iDAC, *command.iarg[1]);
			return true;
		}
	}
	return false;
}


// -- sets all current parameters
void TBParameters::Restore() {
	for (int i = 0; i < NTBParameters; i++) {
		if (parameters[i] != -1) {
			SetParameter(i,parameters[i]);
		}
	}
	tbInterface->Flush();	
}


// == accessing =============================================================


// -- sets a testboard parameter
void TBParameters::SetParameter(const char* dacName, int value)
{
	for (int i = 0; i < NTBParameters; i++) {
        if (strcmp(names[i].c_str(),dacName) == 0) {
			SetParameter(i,value);
		}
	}
}


int TBParameters::GetParameter(const char* dacName)
{
	for (int i = 0; i < NTBParameters; i++)
	{
        if (strcmp(names[i].c_str(),dacName) == 0)
		{
			return parameters[i];
		}
	}
  psi::Log<psi::Info>() << "[TBParameters] Error: DAC Parameter '"
                 << dacName << "' is not found." << std::endl;

	return 0;
}


// == file input / output ===================================================	


// -- reads the parameters from a file
bool TBParameters::ReadTBParameterFile( const char *_file)
{
  std::ifstream _input( _file);
  if( !_input.is_open())
  {
    psi::Log<psi::Info>() << "[TBParameters] Error: Can not open file '" << _file
                   << "' to read TB parameters." << std::endl;

    return false;
  }

  psi::Log<psi::Info>() << "[TBParameters] Reading TB-Parameters from '" << _file
                 << "'." << std::endl;

  // Read file by lines
  for( std::string _line; _input.good(); )
  {
    getline( _input, _line);

    // Skip Empty Lines and Comments (starting from # or - )
    if( !_line.length()
        || '#' == _line[0]
        || '-' == _line[0] ) continue;

    std::istringstream _istring( _line);
    std::string _tmp;
    int _register;
    int _value;

    _istring >> _register >> _tmp >> _value;

    // Skip line in case any errors occured while reading parameters
    if( _istring.fail() || !_tmp.length() ) continue;

    SetParameter( _register, _value);
  }

  _input.close();

	tbInterface->Flush();

  return true;
}


// -- write the parameters to a file
bool TBParameters::WriteTBParameterFile(const char *_file)
{
    std::ofstream file(_file);
    if (!file.is_open())
  {
    psi::Log<psi::Info>() << "[TBParameters] Error: Can not open file '" << _file
                   << "' to write TB parameters." << std::endl;

    return false;
  }

  psi::Log<psi::Info>() << "[TBParameters] Writing TB-Parameters to '" << _file
                 << "'." << std::endl;

	for (int i = 0; i < NTBParameters; i++)
	{
		if (parameters[i] != -1)
        {
            file << std::setw(3) << i << std::setw(1) << " " << std::setw(10) << names[i] << std::setw(1) << " "
                 << std::setw(3) << parameters[i] << std::endl;
		}
	}
	return true;
}


// == Private =======================================================

// -- loads a parameter without setting it
void TBParameters::_SetParameter(int reg, int value)
{
	parameters[reg] = value;
}
