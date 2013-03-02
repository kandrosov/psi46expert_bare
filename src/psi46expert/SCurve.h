/*!
 * \file SCurve.h
 * \brief Definition of SCurve class.
 *
 * \b Changelog
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Inheritence from TObject removed due to compability issues.
 */

#pragma once

#include "TF1.h"
#include "TGraph.h"

/*!
 * \brief Code to fit SCurves
 */
class SCurve
{

public :
	SCurve();

	void Fit();
	void FitSCurves(char *dirName, int nRocs = 16);
	bool OpenInputFile(char *dirName, int roc);
	void FitSCurve(char *dirName, int roc, int col, int row);
	bool ReadData(int &n, double x[], double y[], double xErr[], double yErr[]);

	
private :

	FILE *outputFile, *inputFile;
	TF1 *fit;
	TGraph *graph;
	int mode;
	bool debug;
	double thr, sig, ePerVcal, slope;
};
