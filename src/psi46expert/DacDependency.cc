/*!
 * \file DacDependency.cc
 * \brief Implementation of DacDependency class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - removed deprecated conversion from string constant to char*
 */

#include "DacDependency.h"
#include "ThresholdMap.h"
#include <TH2D.h>
#include "TestParameters.h"

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
	DACParameters* parameters = new DACParameters();
    const char *dacName1 = parameters->GetName(dac1);
    const char *dacName2 = parameters->GetName(dac2);
	delete parameters;
	
	TH2D *histo = new TH2D(Form("%s%s_c%dr%d_C%i", dacName2, dacName1, column, row, chipId),Form("%s%s_c%dr%d_C%i", dacName2, dacName1, column, row, chipId), dacRange1, 0, dacRange1, dacRange2, 0, dacRange2);
	histo->GetXaxis()->SetTitle(Form("%s [DAC units]",dacName1));
	histo->GetYaxis()->SetTitle(Form("%s [DAC units]",dacName2));
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

	int result[dacRange1*dacRange2];
	roc->DacDac(dac1, dacRange1, dac2, dacRange2, nTrig, result);
	
	DisarmPixel();
	
	int n = 0;
	for (int i = 0; i < dacRange1; i++)
	{
		for (int k = 0; k < dacRange2; k++) 
		{
			histo->SetBinContent(i+1, k+1, result[n]);
			n++;
		}
	}

	RestoreDacParameters();
	
	histograms->Add(histo);
}


void DacDependency::SetDacs(int d1, int d2, int range1, int range2)
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
