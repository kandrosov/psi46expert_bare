/*!
 * \file Analysis.h
 * \brief Definition of Analysis class.
 *
 * \b Changelog
 * 09-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Corrected questionable language constructions, which was found using -Wall g++ option.
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using definitions from PsiCommon.h.
 */

#pragma once

#include <TH2D.h>
#include <TH1D.h>

/*!
 * \brief Utilities to analyse histograms
 */
class Analysis {
public:
    static TH1D* TrimBitTest(TH2D *calMap, TH2D *trimMap, char* histoName);
    static TH2D* DifferenceMap(TH2D *map1, TH2D *map2, char* mapName);
    static TH2D* SumVthrVcal(TH2D *map1, TH2D *map2, TH2D *map3, char* mapName);
    static TH1D* Distribution(TH2D *map, int nBins, double lowerEdge, double upperEdge);
    static TH1D* Distribution(TH2D *map);
private:
    Analysis() {}
};
