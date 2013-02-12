/*!
 * \file PHCalibration.cc
 * \brief Implementation of PHCalibration class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new ConfigParameters class definition.
 */

#include <iostream>

#include <TSystem.h>
#include <TRandom.h>
#include <TMath.h>

#include "interface/Delay.h"
#include "BasePixel/Roc.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/ConfigParameters.h"
#include "interface/Log.h"
#include "TestRoc.h"
#include "DacDependency.h"
#include "PHCalibration.h"


PHCalibration::PHCalibration()
{
  psi::LogDebug() << "[PHCalibration] Initialization." << psi::endl;

	Initialize();
}


PHCalibration::PHCalibration(TestRange *aTestRange, TestParameters *aTestParameters, TBInterface *aTBInterface)
{
	testParameters = aTestParameters;
	testRange = aTestRange;
	tbInterface = aTBInterface;
	ReadTestParameters(testParameters);
	Initialize();
}


void PHCalibration::Initialize()
{
// 	printf("mode %i\n", mode);s
	if (mode == 0)
	{
		vcalSteps = 10;
		vcal[0] = 50;
		vcal[1] = 100;
		vcal[2] = 150;
		vcal[3] = 200;
		vcal[4] = 250;
		vcal[5] = 30;
		vcal[6] = 50;
		vcal[7] = 70;
		vcal[8] = 90;
		vcal[9] = 200;
		for (int i = 0; i < 5; i++)  ctrlReg[i] = 0;
		for (int i = 5; i < 10; i++)  ctrlReg[i] = 4;
	}
	else if (mode == 1)
	{	
		vcalSteps = 100;
		for (int i = 0; i < vcalSteps; i++) 
		{
			vcal[i] = 5 + i*5;
			vcal[i + vcalSteps] = 5 + i*5;
			if (i < 50) ctrlReg[i] = 0; else ctrlReg[i] = 4;
		}
	}	
	else if (mode == 2)
	{	
		vcalSteps = 102;
		for (int i = 0; i < 51; i++) 
		{
			vcal[i] = 5 + i*5;
			ctrlReg[i] = 0;
		}
		for (int i = 51; i < 102; i++) 
		{
			vcal[i] = 5 + (i-51)*5;
			ctrlReg[i] = 4;
		}
	}
	if (mode == 3)
	{
		vcalSteps = 9;
		vcal[0] = 50;
		vcal[1] = 100;
		vcal[2] = 150;
		vcal[3] = 200;
		vcal[4] = 250;
		vcal[5] = 50;
		vcal[6] = 70;
		vcal[7] = 90;
		vcal[8] = 200;
		for (int i = 0; i < 5; i++)  ctrlReg[i] = 0;
		for (int i = 5; i < 9; i++)  ctrlReg[i] = 4;
	}
}


void PHCalibration::ReadTestParameters(TestParameters *testParameters)
{
	nTrig = (*testParameters).PHCalibrationNTrig;
// 	memoryCorrection = (*testParameters).PHMemoryCorrection / 100;
	mode = (*testParameters).PHCalibrationMode;
	numPixels = (*testParameters).PHCalibrationNPixels;
	calDelVthrComp = (*testParameters).PHCalibrationCalDelVthrComp;
}


void PHCalibration::RocAction()
{
  psi::LogInfo() << "[PHCalibration] Chip #" << chipId << " Calibration: start." << psi::endl;

	gDelay->Timestamp();
	SaveDacParameters();

	// == Open file
	
    const ConfigParameters& configParameters = ConfigParameters::Singleton();
	char fname[1000];
    sprintf(fname, "%s/phCalibration_C%i.dat", configParameters.Directory().c_str(), chipId);
	FILE *file = fopen(fname, "w");
	if (!file)
	{
    psi::LogInfo() << "[PHCalibration] Error: Can not open file '" << fname
                   << "' to write PH Calibration." << psi::endl;

		return;
	}

  psi::LogInfo() << "[PHCalibration] Writing PH Calibration to '" << fname
                 << "'." << psi::endl;
 
	fprintf(file, "Pulse heights for the following Vcal values:\n");
	fprintf(file, "Low range: ");
	for (int i = 0; i < vcalSteps; i++) 
	{
		if (ctrlReg[i] == 0) fprintf(file, "%3i ", vcal[i]);
	}
	fprintf(file, "\n");
	fprintf(file, "High range: ");
	for (int i = 0; i < vcalSteps; i++) 
	{
		if (ctrlReg[i] == 4) fprintf(file, "%3i ", vcal[i]);
	}
	fprintf(file, "\n");
	fprintf(file, "\n");

	// == Determine appropriate CalDel and VthrComp
	
	bool debug = false;

	int numFlagsRemaining = numPixels;
        TRandom u;
	bool pxlFlags[ROCNUMROWS*ROCNUMCOLS];
	if ( numPixels < 4160 )
	{
	  while ( numFlagsRemaining > 0 ){
	    int column = TMath::FloorNint(ROCNUMCOLS*u.Rndm());
	    int row    = TMath::FloorNint(ROCNUMROWS*u.Rndm());

	    if ( pxlFlags[column*ROCNUMROWS + row] == false ){ // pixel not yet included in test
	      cout << "flagging pixel in column = " << column << ", row = " << row << " for testing" << endl;
	      pxlFlags[column*ROCNUMROWS + row] = true;
	      numFlagsRemaining--;
	    }
	  }
	}
	
	if (debug) {calDel50 = 44; calDel100 = 63; calDel200 = 66; vthrComp50 = 114; vthrComp100 = 99; vthrComp200 = 85;}
	else if (calDelVthrComp)
	{
		SetDAC("CtrlReg", 0);
		calDel200 = GetDAC("CalDel"); vthrComp200 = GetDAC("VthrComp"); // from Pretest
		roc->AdjustCalDelVthrComp(15, 15, 50, -0); calDel50 = GetDAC("CalDel"); vthrComp50 = GetDAC("VthrComp");
		roc->AdjustCalDelVthrComp(15, 15, 100, -0); calDel100 = GetDAC("CalDel"); vthrComp100 = GetDAC("VthrComp");
//		roc->AdjustCalDelVthrComp(15, 15, 200, -1); calDel200 = GetDAC("CalDel"); vthrComp200 = GetDAC("VthrComp");
	}
	else
	{
		calDel200 = GetDAC("CalDel"); vthrComp200 = GetDAC("VthrComp"); // from Pretest
		calDel100 = GetDAC("CalDel"); vthrComp100 = GetDAC("VthrComp"); // from Pretest
		calDel50 = GetDAC("CalDel"); vthrComp50 = GetDAC("VthrComp"); // from Pretest
	}

	// == Loop over all pixels

	int ph[vcalSteps][ROCNUMROWS*ROCNUMCOLS];
	int data[ROCNUMROWS*ROCNUMCOLS];
	int phPosition = 16 + aoutChipPosition*3;
	
	for (int i = 0; i < vcalSteps; i++)
	{
		SetDAC("CtrlReg", ctrlReg[i]);
		SetDAC("CalDel", GetCalDel(i));
		SetDAC("VthrComp", GetVthrComp(i));
		SetDAC("Vcal", vcal[i]);
		Flush();
		
		if ( numPixels >= 4160 )
		  roc->AoutLevelChip(phPosition, nTrig, data);
		else
		  roc->AoutLevelPartOfChip(phPosition, nTrig, data, pxlFlags);

		for (int k = 0; k < ROCNUMROWS*ROCNUMCOLS; k++) ph[i][k] = data[k];
	}	

	for (int col = 0; col < 52; col++)
	{	
	        for (int row = 0; row < 80; row++)
		{
			SetPixel(GetPixel(col, row));
			if (testRange->IncludesPixel(chipId, column, row))
			{
			
				for (int i = 0; i < vcalSteps; i++)
				{
					if (ph[i][col*ROCNUMROWS + row] != 7777) fprintf(file, "%5i ", ph[i][col*ROCNUMROWS + row]);
					else fprintf(file, "  N/A ");
				}
				fprintf(file, "   Pix %2i %2i\n", col, row);
			}
		}
	}
			
	fclose(file);
	RestoreDacParameters();
	gDelay->Timestamp();
}


int PHCalibration::GetCalDel(int vcalStep)
{
	int conversion = 1;
	if (ctrlReg[vcalStep] == 4) conversion = 7;
	if (vcal[vcalStep]*conversion < 75.) return calDel50;
	else if (vcal[vcalStep]*conversion < 125.) return calDel100;
	else return calDel200;
}


int PHCalibration::GetVthrComp(int vcalStep)
{
	int conversion = 1;
	if (ctrlReg[vcalStep] == 4) conversion = 7;
	if (vcal[vcalStep]*conversion < 75.) return vthrComp50;
	else if (vcal[vcalStep]*conversion < 125.) return vthrComp100;
	else return vthrComp200;
}
