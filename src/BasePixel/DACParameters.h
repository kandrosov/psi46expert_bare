/*!
 * \file DACParameters.h
 * \brief Definition of DACParameters class.
 *
 * \b Changelog
 * 01-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Class SysCommand removed.
 * 26-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Removed redundant class Roc.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - removed deprecated conversion from string constant to char*
 */

#pragma once

class TestRoc;

/*!
 * \brief The class represents the DAC settings of a readout chip (ROC)
 */
class DACParameters
{
  public:
    DACParameters();
    DACParameters(TestRoc* const roc);
    void Initialize();
    DACParameters* Copy();
    void Restore();
    
    // == accessing =============================================================
    void SetParameter(int reg, int value, bool correction = true);
    void SetParameter(const char* dacName, int value);
    int GetDAC(const char*dacName);
    int GetDAC(int reg);
    const char* GetName(int reg);

    // == file input / output =================================================== 
    bool ReadDACParameterFile ( const char *filename);
    bool WriteDACParameterFile( const char *filename);

  private:
    void _SetParameter(int reg, int value);

  protected:
    static const int NDACParameters = 256;

    int parameters[NDACParameters];
    std::string names[NDACParameters];

    TestRoc* const roc;
};
