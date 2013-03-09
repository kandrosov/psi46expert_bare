/*!
 * \file PixelAlive.cc
 * \brief Implementation of PixelAlive class.
 *
 * \b Changelog
 * 09-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Corrected questionable language constructions, which was found using -Wall g++ option.
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using definitions from PsiCommon.h.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#include "psi/log.h"

#include "PixelAlive.h"
#include "psi46expert/TestRoc.h"
#include "BasePixel/constants.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/TestParameters.h"

PixelAlive::PixelAlive(TestRange *aTestRange, TBInterface *aTBInterface)
{
    psi::LogDebug() << "[PixelAlive] Initialization." << std::endl;

    testRange = aTestRange;
    tbInterface = aTBInterface;
    ReadTestParameters();
}


void PixelAlive::ReadTestParameters()
{
    const TestParameters& testParameters = TestParameters::Singleton();
    nTrig = testParameters.PixelMapReadouts();
    efficiency = testParameters.PixelMapEfficiency() / 100.0 ;
}


void PixelAlive::RocAction()
{
    psi::LogDebug() << "[PixelAlive] Chip #" << chipId << '.' << std::endl;

    TH2D *histo = GetMap("PixelMap");
    histo->SetMaximum(nTrig);
    histo->SetMinimum(0);

    short mask[psi::ROCNUMROWS * psi::ROCNUMCOLS];
    roc->MaskTest(1, mask);

    for (unsigned i = 0; i < psi::ROCNUMCOLS; i++)
    {
        for (unsigned k = 0; k < psi::ROCNUMROWS; k++)
        {
            int n = mask[i * psi::ROCNUMROWS + k];
            if (n != 0)
            {
                GetPixel(i, k)->SetAlive(false);

                psi::LogInfo() << "[PixelAlive] Error: Mask Defect. n = " << n
                               << " for Pixel( " << i << ", " << k << ")." << std::endl;

                histo->SetBinContent(i + 1, k + 1, -1);
            }
            else histo->SetBinContent(i + 1, k + 1, 0);
        }
    }

    double data[psi::ROCNUMROWS * psi::ROCNUMCOLS];
    roc->ChipEfficiency(nTrig, data);

    for (unsigned i = 0; i < psi::ROCNUMROWS * psi::ROCNUMCOLS; i++)
    {
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
