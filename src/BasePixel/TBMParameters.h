/*!
 * \file TBMParameters.h
 * \brief Definition of TBMParameters class.
 *
 * \b Changelog
 * 01-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Class SysCommand removed.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - removed deprecated conversion from string constant to char*
 */

#pragma once

class TBM;

/*!
 * \brief The class represents the settings of a token bit manager (TBM)
 */
class TBMParameters
{
  public:
    TBMParameters();
    TBMParameters(TBM* const aTBM);
    void Initialize();
    TBMParameters* Copy();
//    bool Execute(SysCommand command);
    void Restore();
    
    // == accessing =============================================================
    void SetParameter(int reg, int value);
    void SetParameter(const char* dacName, int value);
    int GetDAC(const char*dacName);
    int GetDAC(int reg);
    const char* GetName(int reg);

    // == file input / output =================================================== 
    bool ReadTBMParameterFile ( const char *filename);
    bool WriteTBMParameterFile( const char *filename);

  private:
    void _SetParameter(int reg, int value);

  protected:
    static const int NTBMParameters = 7;

    int parameters[NTBMParameters];
    std::string names[NTBMParameters];

    TBM* const tbm;
};
