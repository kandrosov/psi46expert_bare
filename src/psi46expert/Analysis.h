/*!
 * \file Analysis.h
 * \brief Definition of Analysis class.
 *
 * \b Changelog
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using definitions from PsiCommon.h.
 */

#pragma once

#include <TH2D.h>
#include <TH1D.h>
#include "BasePixel/constants.h"

/*!
 * \brief Utilities to analyse histograms
 */
class Analysis
{

public:
	Analysis();

	TH1D* TrimBitTest(TH2D *calMap, TH2D *trimMap, char* histoName);
	TH2D* DifferenceMap(TH2D *map1, TH2D *map2, char* mapName);
	TH2D* SumVthrVcal(TH2D *map1, TH2D *map2, TH2D *map3, char* mapName);
	TH1D* Distribution(TH2D *map, int nBins, double lowerEdge, double upperEdge);
	TH1D* Distribution(TH2D *map);
	
protected:


};

static Analysis *gAnalysis = new Analysis();
