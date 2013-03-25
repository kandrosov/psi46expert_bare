/*!
 * \file Analysis.cc
 * \brief Implementation of Analysis class.
 *
 * \b Changelog
 * 09-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Corrected questionable language constructions, which was found using -Wall g++ option.
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using definitions from PsiCommon.h.
 */

#include "Analysis.h"
#include <TMath.h>
#include "BasePixel/constants.h"

// -- Computes the differences of two maps and fills the result in a 1D histogram
TH1D* Analysis::TrimBitTest(TH2D *calMap, TH2D *trimMap, char* histoName)
{
    TH1D *histo = new TH1D(histoName, histoName, 260, 0., 260.);
    for (unsigned iCol = 0; iCol < psi::ROCNUMCOLS; iCol++) {
        for (unsigned iRow = 0; iRow < psi::ROCNUMROWS; iRow++) {

            Float_t difference = calMap->GetBinContent(iCol + 1, iRow + 1) - trimMap->GetBinContent(iCol + 1, iRow + 1);
            histo->Fill(difference);

        }
    }
    return histo;
}


// -- Computes the difference map of two maps
TH2D* Analysis::DifferenceMap(TH2D *map1, TH2D *map2, char* mapName)
{
    TH2D *differenceMap = new TH2D(mapName, mapName, psi::ROCNUMCOLS, 0., psi::ROCNUMCOLS, psi::ROCNUMROWS, 0.,
                                   psi::ROCNUMROWS);
    for (unsigned iCol = 0; iCol < psi::ROCNUMCOLS; iCol++) {
        for (unsigned iRow = 0; iRow < psi::ROCNUMROWS; iRow++) {
            Float_t difference = map1->GetBinContent(iCol + 1, iRow + 1) - map2->GetBinContent(iCol + 1, iRow + 1);
            differenceMap->SetBinContent(iCol + 1, iRow + 1, difference);
        }
    }
    return differenceMap;
}


// -- Computes the sum map of three maps
TH2D* Analysis::SumVthrVcal(TH2D *map1, TH2D *map2, TH2D *map3, char* mapName)
{
    TH2D *sumMap = new TH2D(mapName, mapName, 256, 0., 255., 256, 0., 255.);
    for (int i = 0; i < 256; i++) {
        for (int k = 0; k < 256; k++) {
            Float_t sum = map1->GetBinContent(i + 1, k + 1) + map2->GetBinContent(i + 1, k + 1) + map3->GetBinContent(i + 1, k + 1);
            sumMap->SetBinContent(i + 1, k + 1, sum);
        }
    }
    return sumMap;
}


// -- Fills a 1D histogram with the data of a map
TH1D* Analysis::Distribution(TH2D *map, int nBins, double lowerEdge, double upperEdge)
{
    TH1D *histo = new TH1D(Form("%sDistribution", map->GetName()), Form("%sDistribution", map->GetName()), nBins, lowerEdge, upperEdge);
    for (unsigned iCol = 0; iCol < psi::ROCNUMCOLS; iCol++) {
        for (unsigned iRow = 0; iRow < psi::ROCNUMROWS; iRow++) {
            histo->Fill(map->GetBinContent(iCol + 1, iRow + 1));
        }
    }
    return histo;
}


// -- Fills a 1D histogram with the data of a map, the range of the 1D histogram is computed automatically
TH1D* Analysis::Distribution(TH2D *map)
{
    double lowerEdge = TMath::Floor(map->GetMinimum()) - 5;
    double upperEdge = TMath::Floor(map->GetMaximum()) + 5;
    int nBins = (int)(upperEdge - lowerEdge);
    TH1D *histo = new TH1D(Form("%sDistribution", map->GetName()), Form("%sDistribution", map->GetName()), nBins, lowerEdge, upperEdge);
    for (unsigned iCol = 0; iCol < psi::ROCNUMCOLS; iCol++) {
        for (unsigned iRow = 0; iRow < psi::ROCNUMROWS; iRow++) {
            histo->Fill(map->GetBinContent(iCol + 1, iRow + 1));
        }
    }
    return histo;
}
