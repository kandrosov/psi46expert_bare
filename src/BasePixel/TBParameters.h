/*!
 * \file TBParameters.h
 * \brief Definition of TBParameters class.
 *
 * \b Changelog
 * 01-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Class SysCommand removed.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - removed deprecated conversion from string constant to char*
 */

#pragma once

#include <string>

class TBInterface;

/*!
 * \brief The class represents the settings of a testboard
 */
class TBParameters
{
  public:
    TBParameters() {}
    TBParameters(TBInterface *aTBInterface);

    virtual ~TBParameters() {}

    void Restore();
    virtual TBParameters* Copy() = 0;

    // == accessing =============================================================
    virtual void SetParameter(int reg, int value) = 0;
    void SetParameter(const char* tbName, int value);
    int GetParameter(const char* dacName);
    
    // == file input / output =================================================== 
    bool ReadTBParameterFile ( const char *filename);
    bool WriteTBParameterFile( const char *filename);
        
  protected:
    void _SetParameter(int reg, int value);
    
  protected:
    static const int NTBParameters = 256;
   
    int parameters[NTBParameters];
    std::string names[NTBParameters];
    
    TBInterface *tbInterface;
};
