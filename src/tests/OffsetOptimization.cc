/*!
 * \file OffsetOptimization.cc
 * \brief Implementation of OffsetOptimization class.
 */

#include "TCanvas.h"
#include <TF1.h>
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/constants.h"
#include "psi/log.h"
#include "OffsetOptimization.h"
#include "psi46expert/TestRoc.h"
#include "PhDacScan.h"
#include "BasePixel/TestParameters.h"

OffsetOptimization::OffsetOptimization(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface)
    : Test("OffsetOptimization", testRange), tbInterface(aTBInterface)
{
    const TestParameters& testParameters = TestParameters::Singleton();
    dac1Start = testParameters.PHdac1Start();
    dac1Stop  = testParameters.PHdac1Stop();
    dac1Step  = testParameters.PHdac1Step();
    dac2Start = testParameters.PHdac2Start();
    dac2Stop  = testParameters.PHdac2Stop();
    dac2Step  = testParameters.PHdac2Step();
}

void OffsetOptimization::RocAction(TestRoc& roc)
{
    SaveDacParameters(roc);
    roc.SetDAC(DACParameters::CtrlReg, 4);
    Test::RocAction(roc);
    RestoreDacParameters(roc);
}

void OffsetOptimization::PixelAction(TestPixel& pixel)
{
    pixel.ArmPixel();
    tbInterface->Flush();
    DoDacDacScan(pixel);
    pixel.DisarmPixel();
}

void OffsetOptimization::DoDacDacScan(TestPixel& pixel)
{
    psi::LogDebug() << "[OffsetOptimization] DAC DAC Scan" << std::endl;

    int dacValue1Size = (dac1Stop - dac1Start) / dac1Step; // VOffsetR0
    int dacValue2Size = (dac2Stop - dac2Start) / dac2Step; // VOffsetOp
    int linearRange;
    short result[256];
    int optimalOp, optimalR0;
    int VcalRangeMax = -99;
    int index1 = 0, index2 = 0;

    std::ostringstream histo2Name;
    histo2Name << "Linear_Range_of_Vcal_c" << pixel.GetColumn() << "r" << pixel.GetRow()
               << "_C" << pixel.GetRoc().GetChipId();
    TH2D *histo2 = new TH2D( histo2Name.str().c_str(), histo2Name.str().c_str(),
                             dacValue1Size + 1, dac1Start, dac1Stop + dac1Step,
                             dacValue2Size + 1, dac2Start, dac2Stop + dac2Step);
    histo2->GetXaxis()->SetTitle("VOffsetR0 [DAC units]");
    histo2->GetYaxis()->SetTitle("VoffsetOp [DAC units]");
    histo2->GetZaxis()->SetTitle("linear range");

    std::ostringstream minPhHistoName;
    minPhHistoName << "Min_PH_c" << pixel.GetColumn() << "r" << pixel.GetRow()
               << "_C" << pixel.GetRoc().GetChipId();
    TH2D *minPhHisto = new TH2D( minPhHistoName.str().c_str(), minPhHistoName.str().c_str(),
                                 dacValue1Size + 1, dac1Start, dac1Stop + dac1Step,
                                 dacValue2Size + 1, dac2Start, dac2Stop + dac2Step);
    minPhHisto->GetXaxis()->SetTitle("VOffsetR0 [DAC units]");
    minPhHisto->GetYaxis()->SetTitle("VoffsetOp [DAC units]");
    minPhHisto->GetZaxis()->SetTitle("starting PH");

    int r0, op;
    for (int i = 0; i <= dacValue1Size; i++) {
        r0 = dac1Start + i * dac1Step;
        pixel.GetRoc().SetDAC(DACParameters::VOffsetR0, r0);

        for (int k = 0; k <= dacValue2Size; k++) {
            op = dac2Start + k * dac2Step;
            pixel.GetRoc().SetDAC(DACParameters::VoffsetOp, op);

            std::ostringstream histoName;
            histoName << "PHVcal_VoffsetOp" << op << "_VOffsetR0" << r0
                      << "_C" << pixel.GetRoc().GetChipId();

            TH1D *histo = new TH1D(histoName.str().c_str(), histoName.str().c_str(), 256, 0, 256);

            // PHDac( dac, dacRange, Trig, position, output)
            tbInterface->PHDac( 25, 256, phDacScan.GetNTrig(), 16 + pixel.GetRoc().GetAoutChipPosition() * 3, result);

            for (int dac = 0; dac < 256; dac++) histo->SetBinContent( dac + 1, result[dac]);

            histo->SetMaximum( result[255] + 100);

            linearRange = static_cast<int>( phDacScan.FindLinearRange(histo) );

            if (linearRange > VcalRangeMax) {
                VcalRangeMax = linearRange;
                index1 = i;
                index2 = k;
            }
            psi::LogDebug() << "[OffsetOptimization] Linear Range: " << linearRange << std::endl;

            histo2->SetBinContent(i + 1, k + 1, linearRange);
            minPhHisto->SetBinContent(i + 1, k + 1, phDacScan.GetMinPh());

            histograms->Add(histo);

            psi::LogDebug() << "[OffsetOptimization] VOffsetR0: " << r0
                            << " VOffsetOp: " << op << std::endl;
        }
    }

    histograms->Add(minPhHisto);
    histograms->Add(histo2);

    optimalOp = dac2Start + index2 * dac2Step;
    optimalR0 = dac1Start + index1 * dac1Step;

    psi::LogDebug() << "[OffsetOptimization] Vcal Range Max: " << VcalRangeMax
                    << " @ VOffsetR0: " << optimalR0
                    << " @ VOffsetOp: " << optimalOp << std::endl;

    psi::LogDebug() << "[OffsetOptimization] Pixel Column: " << pixel.GetColumn()
                    << " Row: " << pixel.GetRow() << std::endl;
}
