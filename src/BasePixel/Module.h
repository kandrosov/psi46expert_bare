/*!
 * \file Module.h
 * \brief Definition of Module class.
 *
 * \b Changelog
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using definitions from PsiCommon.h.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new ConfigParameters class definition.
 */

#pragma once

#include "Roc.h"
#include "TBM.h"
#include "TBInterface.h"
#include "BasePixel/ConfigParameters.h"

/*!
 * \brief Class provides basic module functionality, contains a TBM and several ROCs
 */
class Module
{

public:
    inline Module() {}
    Module(int aCNId, TBInterface *aTBInterface);
	virtual ~Module();

	void Initialize();
	void Execute(SysCommand &command);
	void SetDAC(int chipId, int reg, int value);
	void SetTBM(int chipId, int reg, int value);
	void WriteTrimConfiguration(char* filename);
	void ReadTrimConfiguration(char* filename);
	void WriteDACParameterFile( const char* filename);
	void ReadDACParameterFile(char* filename);
	void SetTrim(int iRoc, int iCol, int iRow, int trimBit);
        int GetTemperature();
	void AdjustDTL();
		
	int GetHubId();
	int GetTBM(int reg);
	int NRocs();
	Roc* GetRoc(int iRoc);
	TBM* GetTBM();
	void SetTBMSingle(int tbmChannel);
	
protected:

	int nRocs;
    Roc* roc[psi::MODULENUMROCS];
	
	TBM *tbm;
	TBInterface *tbInterface;
	
	int hubId;
	int controlNetworkId;

};
