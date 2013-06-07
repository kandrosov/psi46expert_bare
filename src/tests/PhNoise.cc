/*!
 * \file PhNoise.cc
 * \brief Implementation of PhNoise class.
 */

#include <iomanip>

#include "psi/log.h"

#include <TMath.h>

#include "PhNoise.h"
#include "psi46expert/TestRoc.h"
#include "psi46expert/TestModule.h"
#include "BasePixel/TBAnalogInterface.h"

bool PhNoise::debug = true;

PhNoise::PhNoise(TestRange *aTestRange, TBInterface *aTBInterface)
{
    psi::LogDebug() << "[PhNoise] Initialization." << std::endl;

    testRange = aTestRange;
    tbInterface = aTBInterface;
}

void PhNoise::ModuleAction()
{
    int offset;
    if (((TBAnalogInterface*)tbInterface)->TBMPresent()) offset = 3;
    else offset = 2; //either tbm black or roc black
    TH1D *black = new TH1D("black", "black", 4000, -2000., 2000.);
    for (int i = 0; i < nReadouts; i++) {
        ((TBAnalogInterface*)tbInterface)->ADCRead(data, count, 1);
        if (count > offset) black->Fill(data[offset]);
    }

    if (debug)
        psi::LogInfo() << "Black " << std::setprecision(1) << black->GetMean() << " +- "
                       << std::setprecision(2) << black->GetRMS() << std::endl;

    Test::ModuleAction();
}


void PhNoise::RocAction()
{
    int data[psi::ROCNUMROWS * psi::ROCNUMCOLS], offset;
    if (((TBAnalogInterface*)tbInterface)->TBMPresent()) offset = 16;
    else offset = 9;
    int phPosition = offset + aoutChipPosition * 3;

    TH2D *phMean = GetMap("phMean");
    TH2D *phSquaredMean = GetMap("phSquaredMean");
    TH2D *phVariance =  GetMap("phVariance");

    for (int i = 0; i < nReadouts; i++) {
        roc->AoutLevelChip(phPosition, 1, data);
        for (unsigned k = 0; k < psi::ROCNUMROWS * psi::ROCNUMCOLS; k++) {
            if (debug && k == 2393)
                psi::LogInfo() << k << " ph " << data[k] << std::endl;
            phMean->Fill(k / psi::ROCNUMROWS, k % psi::ROCNUMROWS, data[k]);
            phSquaredMean->Fill(k / psi::ROCNUMROWS, k % psi::ROCNUMROWS, data[k]*data[k]);
        }
    }
    double mean, squaredMean, variance;
    for (unsigned k = 0; k < psi::ROCNUMROWS * psi::ROCNUMCOLS; k++) {
        mean = phMean->GetBinContent(k / psi::ROCNUMROWS + 1, k % psi::ROCNUMROWS + 1);
        squaredMean = phSquaredMean->GetBinContent(k / psi::ROCNUMROWS + 1, k % psi::ROCNUMROWS + 1);
        variance = TMath::Sqrt((squaredMean - mean * mean / nReadouts) / (nReadouts - 1));
        phVariance->Fill(k / psi::ROCNUMROWS, k % psi::ROCNUMROWS, variance);
        if (debug && k == 2393)
            psi::LogInfo() << "phMean " << (mean / nReadouts) << " phSquaredMean " << (squaredMean / nReadouts)
                           << " variance " << variance << std::endl;
    }

    histograms->Add(phMean);
    histograms->Add(phSquaredMean);
    histograms->Add(phVariance);
}
