/*!
 * \file TBMParameters.cc
 * \brief Implementation of TBMParameters class.
 *
 * \b Changelog
 * 01-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using a new PSI Logging System.
 *      - Class SysCommand removed.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - removed deprecated conversion from string constant to char*
 */

#include <fstream>
#include <sstream>
#include <string>

#include "BasePixel/TBMParameters.h"
#include "BasePixel/TBM.h"
#include "psi/log.h"

TBMParameters::TBMParameters() : tbm(NULL)
{
  Initialize();
}


TBMParameters::TBMParameters(TBM* const aTBM) : tbm(aTBM)
{
  Initialize();
}


void TBMParameters::Initialize()
{
  for (int i = 0; i < NTBMParameters; i++)
  {
    parameters[i] = -1;
    names[i] = "";
  }

  names[0]="Single";
  names[1]="Speed";
  names[2]="Inputbias";
  names[3]="Outputbias";
  names[4]="Dacgain";
  names[5]="Triggers";
  names[6]="Mode";
}


//bool TBMParameters::Execute(SysCommand command)
//{
//  for (int iDAC = 0; iDAC < NTBMParameters; iDAC++)
//  {

//    if ( (strcmp(names[iDAC].c_str(),"") != 0) && (strcmp(command.carg[0],names[iDAC].c_str()) == 0))
//    {
//      SetParameter(iDAC, *command.iarg[1]);
//      return true;
//    }
//  }
//  return false;
//}

// -- sets all the current DAC parameters
void TBMParameters::Restore()
{
  for (int i = 0; i < NTBMParameters; i++)
  {
    if (parameters[i] != -1)
    {
      SetParameter(names[i].c_str(), parameters[i]);
    } 
  }
}


TBMParameters* TBMParameters::Copy()
{
  TBMParameters* newParameters;
  newParameters = new TBMParameters(tbm);
  for (int i = 0; i < NTBMParameters; i++)
  {
    newParameters->_SetParameter(i,parameters[i]);
  }
  return newParameters;
}


// == accessing =======================================================================


void TBMParameters::SetParameter(int reg, int value)
{
  if (reg > -1 && reg < NTBMParameters)
	{
      //  psi::Log<psi::Info>( "TBMParameters") << "Setting parameter "
	  //                        << names[reg] << " to value: "
          //                         << value << std::endl;
    SetParameter(names[reg].c_str(), value);
	}
}


void TBMParameters::SetParameter(const char* dacName, int value)
{
  bool parameterSet = false;
  
  for (int i = 0; i < NTBMParameters; i++)
  {
    if (strcmp(names[i].c_str(), dacName) == 0)
    {
      //    psi::Log<psi::Info>( "TBMParameters") << "Setting parameter "
      //                               << dacName << " to value: "
      //                              << value << std::endl;
      parameters[i] = value;
      if (i == 0) //single, dual
      {
        if (value == 0) tbm->setSingleMode();
        else if (value == 1) tbm->setDualMode();
        else if (value == 2) tbm->setSingleMode2();
      }
      else if (i == 1) //speed
      {
        tbm->set2040Readout(value);
      }
      else if (i == 2) //inputbias
      {
        tbm->setTBMDAC(0, value);
      }
      else if (i == 3) //outputbias
      {
        tbm->setTBMDAC(1, value);
      }
      else if (i == 4) //dacgain
      {
        tbm->setTBMDAC(2, value);
      }
      else if (i == 5) //triggers
      {
        if (value == 0) tbm->setIgnoreTriggers(0x00);
              else if (value == 1) tbm->setIgnoreTriggers(0x10);
              else if (value == 2) tbm->setDisableTriggers(0x40);
      }
      else if (i == 6) //mode
      {
        if (value == 0) tbm->setMode(0x00);  //sync
              else if (value == 1) tbm->setMode(0xC0); //cal
              else if (value == 2) tbm->setMode(0x80); //clear
      }
      parameterSet=true;
      i = NTBMParameters;
    }
  }
  if (!parameterSet)
    psi::Log<psi::Info>() << "[TBMParameters] Error: TBM Parameter '" << dacName
                   << "' is not found." << std::endl;
}


// -- returns the DAC value of dacName
int TBMParameters::GetDAC(const char* dacName)
{
  for (int i = 0; i < NTBMParameters; i++)
  {
    if (strcmp(names[i].c_str(), dacName) == 0)
    {
      return parameters[i];
    }
  }
  psi::Log<psi::Info>() << "[TBMParameters] Error: TBM Parameter '" << dacName
                 << "' is not found." << std::endl;

  return 0;
}


int TBMParameters::GetDAC(int reg)
{
  return parameters[reg];
}


//  -- gives the name of a DAC
const char* TBMParameters::GetName(int reg)
{
  return names[reg].c_str();
}


// == file input / output =================================================== 


// -- reads TBM parameters from a file and sets them
bool TBMParameters::ReadTBMParameterFile( const char *_file)
{
  std::ifstream _input( _file);
  if( !_input.is_open())
  {
    psi::Log<psi::Info>() << "[TBMParameters] Error: Can not open file '" << _file
                   << "' to read TBM parameters." << std::endl;

    return false;
  }

  psi::Log<psi::Info>() << "[TBMParameters] Reading TBM-Parameters from '" << _file
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

  return true;
}


// -- writes the TBM parameters to a file
bool TBMParameters::WriteTBMParameterFile(const char *_file)
{
    std::ofstream file(_file);
  if (!file.is_open())
  {
    psi::Log<psi::Info>() << "[TBMParameters] Error: Can not open file '" << _file
                   << "' to write TBM parameters." << std::endl;
    return false;
  }

  psi::Log<psi::Info>() << "[TBMParameters] Writing TBM-Parameters to '" << _file
                 << "'." << std::endl;

  for (int i = 0; i < NTBMParameters; i++)
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
// -- saves the value of an parameter but doesn't set the DAC
void TBMParameters::_SetParameter(int reg, int value)
{
  parameters[reg] = value;
}
