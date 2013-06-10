/*!
 * \file DacOverview.cc
 * \brief Implementation of DacOverview class.
 */

#include "DacOverview.h"
#include "psi46expert/TestRoc.h"
#include "psi46expert/TestModule.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/constants.h"
#include "TCanvas.h"
#include "BasePixel/TestParameters.h"

DacOverview::DacOverview(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface)
    : Test("DacOverview", testRange), tbInterface(aTBInterface)
{
    const TestParameters& testParameters = TestParameters::Singleton();
    NumberOfSteps = testParameters.PHNumberOfSteps();
    DacType = testParameters.PHDacType();
    debug = true;
}

void DacOverview::RocAction(TestRoc& roc)
{
    SaveDacParameters(roc);
    roc.EnableDoubleColumn(10);
    TestPixel& pixel = roc.GetPixel(10, 13);
    pixel.ArmPixel();
    tbInterface->Flush();
    DoDacScan(pixel);
    pixel.DisarmPixel();
    RestoreDacParameters(roc);
}

void DacOverview::DoDacScan(TestPixel& pixel)
{
    psi::LogInfo() << " ************************* DAC SCAN **************************" << std::endl;

    int offset;
    if (tbInterface->TBMPresent()) offset = 16;
    else offset = 9;

    psi::LogInfo() << "chipId = " << pixel.GetRoc().GetChipId() << ", col = " << pixel.GetColumn()
                   << ", row = " << pixel.GetRow() << std::endl;

    int position;
    unsigned Min = 0;
    unsigned Max = 0;
    std::string Type;
    std::string dacName;

    TH1D *histoTbmUb, *histoTbmLev[4], *histoRocLev[5], *histoRocUb;

    if (DacType == 0) { // ROC Dacs
        Min = 1;
        Max = 28;
        Type = "ROC";
    } else if (DacType == 1) { // TBM Dacs
        Min = 2;
        Max = 5;
        Type = "TBM";
    }

    tbInterface->DataTriggerLevel(-100);    // xxx
    tbInterface->Flush();

    for (unsigned DacRegister = Min; DacRegister < Max; DacRegister++) { // loop over all possible Dacs of a DacType
        psi::LogInfo() << "Min = " << Min << ", Max = " << Max << std::endl;
        psi::LogInfo() << "DAC set to " << DacRegister << std::endl;
        int scanMax = 256;
        int defaultValue = 0;
        bool haveDefaultValue = false;
        if (DacType == 0) defaultValue = pixel.GetRoc().GetDAC((DACParameters::Register)DacRegister);
        else if (DacType == 1)
            haveDefaultValue = pixel.GetRoc().GetModule().GetTBM((TBMParameters::Register)DacRegister, defaultValue);

        if  (DacType == 0) {
            dacName = DACParameters::GetRegisterName((DACParameters::Register)DacRegister);
        }

        else if (DacType == 1 && DacRegister == 2) dacName = "Inputbias";
        else if (DacType == 1 && DacRegister == 3) dacName = "Outputbias";
        else if (DacType == 1 && DacRegister == 4) dacName = "Dacgain";

        psi::LogInfo() << "Using " << dacName << std::endl;

        int loopcount = 0;

        psi::LogInfo() << "default value = " << defaultValue << std::endl;

        unsigned short count;
        short data[psi::FIFOSIZE];

        histoTbmUb = new TH1D(Form("%sDAC%i_TbmUb", Type.c_str(), DacRegister), "TbmUb", NumberOfSteps, 0, 255);
        histoTbmUb->GetXaxis()->SetTitle(Form("%s [DAC units]", dacName.c_str()));
        histoTbmUb->GetYaxis()->SetTitle("TBM Lev & UB [ADC units]");
        histoRocUb = new TH1D(Form("%sDAC%i_RocUb", Type.c_str(), DacRegister), "RocUb", NumberOfSteps, 0, 255);
        histoRocUb->GetXaxis()->SetTitle(Form("%s [DAC units]", dacName.c_str()));
        histoRocUb->GetYaxis()->SetTitle("ROC Lev & UB [ADC units]");
        for (int i = 0; i < 4; i++) {
            histoTbmLev[i] = new TH1D(Form("%sDAC%i_TbmLev%i", Type.c_str(), DacRegister, i), "TbmLev", NumberOfSteps, 0, 255);
            histoTbmLev[i]->GetXaxis()->SetTitle(Form("%s [DAC units]", dacName.c_str()));
            histoTbmLev[i]->GetYaxis()->SetTitle("TBM Lev & UB [ADC units]");
        }
        for (int i = 0; i < 5; i++) {
            histoRocLev[i] = new TH1D(Form("%sDAC%i_RocLev%i", Type.c_str(), DacRegister, i), "RocLev", NumberOfSteps, 0, 255);
            histoRocLev[i]->GetXaxis()->SetTitle(Form("%s [DAC units]", dacName.c_str()));
            histoRocLev[i]->GetYaxis()->SetTitle("ROC Lev & UB [ADC units]");
        }

        // set level so that Tbm Lev0 corresponds to lowest level
        tbInterface->ADCRead(data, count, 1);
        int lev1 = data[7];
        tbInterface->ADCRead(data, count, 1);
        int lev2 = data[7];
        while (lev2 > lev1) {
            lev1 = lev2;
            tbInterface->ADCRead(data, count, 1);
            lev2 = data[7];
        }
        for (int i = 0; i < 3; i++) {
            tbInterface->ADCRead(data, count, 1);
        }

        // loop over values for a Dac
        for (int scanValue = 0; scanValue < scanMax; scanValue += ((int)scanMax / NumberOfSteps)) {
            if (DacType == 0) pixel.GetRoc().SetDAC((DACParameters::Register)DacRegister, scanValue);
            else if (DacType == 1)
                pixel.GetRoc().GetModule().SetTBM(pixel.GetRoc().GetChipId(),
                                                  (TBMParameters::Register)DacRegister, scanValue);
            loopcount++;
            int sum[4] = {0, 0, 0, 0};

            // Test TBM LEV

            position = 7;
            int readouts = 1;
            for (int j = 0; j < readouts; j++) { // number of readouts per level
                for (int i = 0; i < 4; i++) { // loop over levels
                    tbInterface->ADCRead(data, count, 1);
                    if (count != 70)
                        psi::LogInfo() << "Warning! count = " << count << std::endl;
                    sum[i] = sum[i] + data[position];
                }
            }

            psi::LogInfo() << "loopcount = " << loopcount << std::endl;

            for (int i = 0; i < 4; i++) {
                sum[i] = sum[i] / readouts;
                if (count != 70) histoTbmLev[i]->SetBinContent(loopcount, 2500);
                else histoTbmLev[i]->SetBinContent(loopcount, sum[i]);
            }

            // Test TBM UB, ROC LEV, ROC UB

            tbInterface->ADCRead(data, count, 12);
            if (count != 70)
                psi::LogInfo() << "Warning! count = " << count << std::endl;

            position = 1;
            if (count != 70) histoTbmUb->SetBinContent(loopcount, 2500);
            else histoTbmUb->SetBinContent(loopcount, data[position]);

            position = 8 + pixel.GetRoc().GetAoutChipPosition() * 3;
            if (count != 70) histoRocUb->SetBinContent(loopcount, 2500);
            else histoRocUb->SetBinContent(loopcount, data[position]);

            for (int i = 0; i < 5; i++) {
                position = 11 + i + pixel.GetRoc().GetAoutChipPosition() * 3;
                if (count != 70) histoRocLev[i]->SetBinContent(loopcount, 2500);
                else histoRocLev[i]->SetBinContent(loopcount, data[position]);
            }
        }
        for (int i = 0; i < 5; i++) histograms->Add(histoRocLev[i]);
        histograms->Add(histoRocUb);
        for (int i = 0; i < 4; i++) histograms->Add(histoTbmLev[i]);
        histograms->Add(histoTbmUb);

        if (DacType == 0) pixel.GetRoc().SetDAC((DACParameters::Register)DacRegister, defaultValue);
        else if (DacType == 1 && haveDefaultValue)
            pixel.GetRoc().GetModule().SetTBM(pixel.GetRoc().GetChipId(),
                                              (TBMParameters::Register) DacRegister, defaultValue);
    }
}
