/*!
 * \file Analysis.cc
 * \brief Implementation of Analysis class.
 */

#include <sstream>

#include "Analysis.h"
#include <TMath.h>
#include "data/HistogramNameProvider.h"
#include "BasePixel/constants.h"

// -- Computes the differences of two maps and fills the result in a 1D histogram
TH1D* Analysis::TrimBitTest(TH2D *calMap, TH2D *trimMap, const std::string& histoName)
{
    TH1D *histo = new TH1D(histoName.c_str(), histoName.c_str(), 260, 0., 260.);
    for (unsigned iCol = 0; iCol < psi::ROCNUMCOLS; iCol++) {
        for (unsigned iRow = 0; iRow < psi::ROCNUMROWS; iRow++) {
            Float_t difference = calMap->GetBinContent(iCol + 1, iRow + 1) - trimMap->GetBinContent(iCol + 1, iRow + 1);
            histo->Fill(difference);
        }
    }
    return histo;
}

// -- Computes the difference map of two maps
TH2D* Analysis::DifferenceMap(TH2D *map1, TH2D *map2, const std::string &mapName)
{
    TH2D *differenceMap = new TH2D(mapName.c_str(), mapName.c_str(), psi::ROCNUMCOLS, 0., psi::ROCNUMCOLS,
                                   psi::ROCNUMROWS, 0., psi::ROCNUMROWS);
    for (unsigned iCol = 0; iCol < psi::ROCNUMCOLS; iCol++) {
        for (unsigned iRow = 0; iRow < psi::ROCNUMROWS; iRow++) {
            Float_t difference = map1->GetBinContent(iCol + 1, iRow + 1) - map2->GetBinContent(iCol + 1, iRow + 1);
            differenceMap->SetBinContent(iCol + 1, iRow + 1, difference);
        }
    }
    return differenceMap;
}

// -- Computes the sum map of three maps
TH2D* Analysis::SumVthrVcal(TH2D *map1, TH2D *map2, TH2D *map3, const std::string &mapName)
{
    TH2D *sumMap = new TH2D(mapName.c_str(), mapName.c_str(), 256, 0., 255., 256, 0., 255.);
    for (int i = 0; i < 256; i++) {
        for (int k = 0; k < 256; k++) {
            Float_t sum = map1->GetBinContent(i + 1, k + 1) + map2->GetBinContent(i + 1, k + 1)
                    + map3->GetBinContent(i + 1, k + 1);
            sumMap->SetBinContent(i + 1, k + 1, sum);
        }
    }
    return sumMap;
}

// -- Fills a 1D histogram with the data of a map
TH1D* Analysis::Distribution(TH2D *map, int nBins, double lowerEdge, double upperEdge, unsigned id)
{
    const std::string name = psi::data::HistogramNameProvider::DistributionName(map->GetName(), id);
    TH1D *histo = new TH1D(name.c_str(), name.c_str(), nBins, lowerEdge, upperEdge);
    for (unsigned iCol = 0; iCol < psi::ROCNUMCOLS; iCol++) {
        for (unsigned iRow = 0; iRow < psi::ROCNUMROWS; iRow++) {
            histo->Fill(map->GetBinContent(iCol + 1, iRow + 1));
        }
    }
    return histo;
}

// -- Fills a 1D histogram with the data of a map, the range of the 1D histogram is computed automatically
TH1D* Analysis::Distribution(TH2D *map, unsigned id)
{
    double lowerEdge = TMath::Floor(map->GetMinimum()) - 5;
    double upperEdge = TMath::Floor(map->GetMaximum()) + 5;
    int nBins = (int)(upperEdge - lowerEdge);
    const std::string name = psi::data::HistogramNameProvider::DistributionName(map->GetName(), id);
    TH1D *histo = new TH1D(name.c_str(), name.c_str(), nBins, lowerEdge, upperEdge);
    for (unsigned iCol = 0; iCol < psi::ROCNUMCOLS; iCol++) {
        for (unsigned iRow = 0; iRow < psi::ROCNUMROWS; iRow++) {
            histo->Fill(map->GetBinContent(iCol + 1, iRow + 1));
        }
    }
    return histo;
}
