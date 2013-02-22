/*!
 * \file ControlNetwork.h
 * \brief Definition of ControlNetwork class.
 *
 * \b Changelog
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using definitions from PsiCommon.h.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new ConfigParameters class definition.
 */

#pragma once

#include "Module.h"
#include "TBInterface.h"
#include "SysCommand.h"

/*!
 * \brief Provides ControlNetwork functionalities, mainly a module container
 */
class ControlNetwork
{

public:
    inline ControlNetwork() {}
    ControlNetwork(TBInterface* aTbInterface);
	virtual ~ControlNetwork();
	void Execute(SysCommand &command);
	
	void Initialize();
	
	void WriteTrimConfiguration(char* filename);
	void ReadTrimConfiguration(char* filename);
	void WriteDACParameterFile(const char* filename);
	void ReadDACParameterFile(char* filename);

	int NModules();	
	Module* GetModule(int iModule);
	void SetDAC(int iModule, int iRoc, int reg, int value);
	void SetTBM(int iModule, int iRoc, int reg, int value);
	void SetTrim(int iModule, int iRoc, int iCol, int iRow, int trimBit);
	
protected:
	int nModules;
    Module* module[psi::CONTROLNETWORKNUMMODULES];
	
	TBInterface *tbInterface;
};
