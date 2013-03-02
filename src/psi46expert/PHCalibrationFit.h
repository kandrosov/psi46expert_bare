/*!
 * \file PHCalibrationFit.h
 * \brief Definition of PHCalibrationFit class.
 *
 * \b Changelog
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Inheritence from TObject removed due to compability issues.
 */

#pragma once

#include "TF1.h"
#include "TH1D.h"
#include "TGraph.h"

/*!
 * \brief Code to fit PHCalibrationFits
 *
 * fit modes: 1 = Peter's 5 parameter function
 *            0 = linear in the main region
 *            3 = tanh fit
 */
class PHCalibrationFit
{

public :
	PHCalibrationFit(int fitMode);
	void FitLin();
	void FitTanPol();
	void FitTanh();
	void FitAllCurves(char *dirName, int nRocs = 16);
	void FitCurve(char *dirName, int chip, int col, int row);
	void ShowFitPlots();
	void WriteOutputFile(FILE *outputFile);

	
private :

	static const double rangeConversion = 7.;
	static const int vcalSteps = 5;
	double vcal[2*vcalSteps], vcalLow[2*vcalSteps];
	double x[2*vcalSteps], y[2*vcalSteps], xErr[2*vcalSteps], yErr[2*vcalSteps];
	int n;
	int fitMode;
	
	int nFitParams;
	
	TH1D *histoFit[5], *histoChi;
	TGraph *graph;
	TF1 *phFit, *tanFit;
};
