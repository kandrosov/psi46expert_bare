/*!
 * \file DacDependency.cc
 * \brief Implementation of DacDependency class.
 */

#include "DacDependency.h"
#include "BasePixel/ThresholdMap.h"
#include <TH2D.h>
#include "BasePixel/TestParameters.h"

DacDependency::DacDependency(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface)
    : Test("DacDependency", testRange), tbInterface(aTBInterface)
{
    const TestParameters& testParameters = TestParameters::Singleton();
    dac1 = testParameters.Dac1();
    dac2 = testParameters.Dac2();
    nTrig = testParameters.DacNTrig();
    dacRange1 = testParameters.DacRange1();
    dacRange2 = testParameters.DacRange2();
}

void DacDependency::PixelAction(TestPixel& pixel)
{
    const std::string& dacName1 = DACParameters::GetRegisterName(dac1);
    const std::string& dacName2 = DACParameters::GetRegisterName(dac2);
    std::ostringstream histo_name;
    histo_name << dacName2 << dacName1 << "_c" << pixel.GetColumn() << "r" << pixel.GetRow() << "_C"
               << pixel.GetRoc().GetChipId();
    TH2D *histo = new TH2D(histo_name.str().c_str(), histo_name.str().c_str(),
                           dacRange1, 0, dacRange1, dacRange2, 0, dacRange2);
    histo->GetXaxis()->SetTitle(Form("%s [DAC units]", dacName1.c_str()));
    histo->GetYaxis()->SetTitle(Form("%s [DAC units]", dacName2.c_str()));
    histo->GetZaxis()->SetTitle("# readouts");


    SaveDacParameters(pixel.GetRoc());
    tbInterface->Flush();

    pixel.ArmPixel();
    tbInterface->Flush();

    int result[dacRange1 * dacRange2];
    pixel.GetRoc().DacDac(dac1, dacRange1, dac2, dacRange2, nTrig, result);

    pixel.DisarmPixel();

    int n = 0;
    for (int i = 0; i < dacRange1; i++) {
        for (int k = 0; k < dacRange2; k++) {
            histo->SetBinContent(i + 1, k + 1, result[n]);
            n++;
        }
    }

    RestoreDacParameters(pixel.GetRoc());

    histograms->Add(histo);
}


void DacDependency::SetDacs(DACParameters::Register d1, DACParameters::Register d2, int range1, int range2)
{
    dac1 = d1;
    dac2 = d2;
    dacRange1 = range1;
    dacRange2 = range2;
}
