/*!
 * \file PixelAlive.cc
 * \brief Implementation of PixelAlive class.
 */

#include "psi/log.h"

#include "PixelAlive.h"
#include "psi46expert/TestRoc.h"
#include "BasePixel/constants.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/TestParameters.h"

PixelAlive::PixelAlive(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface)
    : Test("PixelAlive", testRange), tbInterface(aTBInterface)
{
    const TestParameters& testParameters = TestParameters::Singleton();
    nTrig = testParameters.PixelMapReadouts();
    efficiency = testParameters.PixelMapEfficiency() / 100.0 ;
}

void PixelAlive::RocAction(TestRoc& roc)
{
    psi::LogDebug() << "[PixelAlive] Chip #" << roc.GetChipId() << '.' << std::endl;

    TH2D *histo = CreateMap("PixelMap", roc.GetChipId());
    histo->SetMaximum(nTrig);
    histo->SetMinimum(0);

    short mask[psi::ROCNUMROWS * psi::ROCNUMCOLS];
    roc.MaskTest(1, mask);

    for (unsigned i = 0; i < psi::ROCNUMCOLS; i++) {
        for (unsigned k = 0; k < psi::ROCNUMROWS; k++) {
            int n = mask[i * psi::ROCNUMROWS + k];
            if (n != 0) {
                roc.GetPixel(i, k).SetAlive(false);

                psi::LogInfo() << "[PixelAlive] Error: Mask Defect. n = " << n
                               << " for Pixel( " << i << ", " << k << ")." << std::endl;

                histo->SetBinContent(i + 1, k + 1, -1);
            } else histo->SetBinContent(i + 1, k + 1, 0);
        }
    }

    double data[psi::ROCNUMROWS * psi::ROCNUMCOLS];
    roc.ChipEfficiency(nTrig, data);

    for (unsigned i = 0; i < psi::ROCNUMROWS * psi::ROCNUMCOLS; i++) {
        double value = data[i] * nTrig;
        if (value == 0)
            psi::LogInfo() << "[PixelAlive] Error: Dead Pixel( "
                           << ( i / psi::ROCNUMROWS) << ", " << ( i % psi::ROCNUMROWS)
                           << ") with n = " << static_cast<int>( value) << std::endl;
        if (value < 0) value = -2;  // to distinguish this problem from mask defects
        if (histo->GetBinContent(i / psi::ROCNUMROWS + 1, i % psi::ROCNUMROWS + 1) == 0)
            histo->SetBinContent(i / psi::ROCNUMROWS + 1, i % psi::ROCNUMROWS + 1, value);
    }

    histograms->Add(histo);
}
