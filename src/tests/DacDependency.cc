/*!
 * \file DacDependency.cc
 * \brief Implementation of DacDependency class.
 */

#include "DacDependency.h"
#include "BasePixel/ThresholdMap.h"
#include <TH2D.h>
#include "BasePixel/TestParameters.h"

DacDependency::DacDependency(TestRange *aTestRange, TBInterface *aTBInterface)
{
    testRange = aTestRange;
    tbInterface = aTBInterface;
    ReadTestParameters();
}

void DacDependency::ReadTestParameters()
{
    const TestParameters& testParameters = TestParameters::Singleton();
    dac1 = testParameters.Dac1();
    dac2 = testParameters.Dac2();
    nTrig = testParameters.DacNTrig();
    dacRange1 = testParameters.DacRange1();
    dacRange2 = testParameters.DacRange2();
}

void DacDependency::PixelAction()
{
    const std::string& dacName1 = DACParameters::GetRegisterName(dac1);
    const std::string& dacName2 = DACParameters::GetRegisterName(dac2);
    std::ostringstream histo_name;
    histo_name << dacName2 << dacName1 << "_c" << column << "r" << row << "_C" << chipId;
    TH2D *histo = new TH2D(histo_name.str().c_str(), histo_name.str().c_str(),
                           dacRange1, 0, dacRange1, dacRange2, 0, dacRange2);
    histo->GetXaxis()->SetTitle(Form("%s [DAC units]", dacName1.c_str()));
    histo->GetYaxis()->SetTitle(Form("%s [DAC units]", dacName2.c_str()));
    histo->GetZaxis()->SetTitle("# readouts");


    SaveDacParameters();
    Flush();

    ArmPixel();
    Flush();

// 	for (int i = 0; i < dacRange1; i++)
// 	{
// 		roc->SetDAC(dac1, i);
// 		roc->SendSignals(0, dacRange2, 1, nTrig, dacName2);
// 		Flush();
// 		roc->ReadSignals(0, dacRange2, 1, nTrig, data);
// 		for (int k = 0; k < dacRange2; k++)
// 		{
// 			histo->SetBinContent(i+1, k+1, data[k]);
// 		}
// 	}
// 	DisarmPixel();

    int result[dacRange1 * dacRange2];
    roc->DacDac(dac1, dacRange1, dac2, dacRange2, nTrig, result);

    DisarmPixel();

    int n = 0;
    for (int i = 0; i < dacRange1; i++) {
        for (int k = 0; k < dacRange2; k++) {
            histo->SetBinContent(i + 1, k + 1, result[n]);
            n++;
        }
    }

    RestoreDacParameters();

    histograms->Add(histo);
}


void DacDependency::SetDacs(DACParameters::Register d1, DACParameters::Register d2, int range1, int range2)
{
    dac1 = d1;
    dac2 = d2;
    dacRange1 = range1;
    dacRange2 = range2;
}


void DacDependency::SetNTrig(int aNumber)
{
    nTrig = aNumber;
}
