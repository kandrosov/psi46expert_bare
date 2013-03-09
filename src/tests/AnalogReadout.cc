/*!
 * \file AnalogReadout.cc
 * \brief Implementation of AnalogReadout class.
 *
 * \b Changelog
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using definitions from PsiCommon.h.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#include <iomanip>
#include "AnalogReadout.h"
#include "psi46expert/TestModule.h"
#include "BasePixel/TBAnalogInterface.h"


AnalogReadout::AnalogReadout(TestRange *aTestRange, TBInterface *aTBInterface)
{
	testRange = aTestRange;
	tbInterface = aTBInterface;
	debug = false;
}


void AnalogReadout::ModuleAction()
{
	unsigned short counter;
    short data[psi::FIFOSIZE];

	TBAnalogInterface *anaInterface = (TBAnalogInterface*)tbInterface;
	int emptyReadoutLengthADC = anaInterface->GetEmptyReadoutLengthADC();
	anaInterface->ADCRead(data, counter, 100);

	int max = counter;
	if (counter > emptyReadoutLengthADC) max = emptyReadoutLengthADC;
	TH1D *histo = new TH1D("AnalogReadout", "AnalogReadout", emptyReadoutLengthADC, 0, emptyReadoutLengthADC);
	for (int i = 0; i < max; i++)
	{
		histo->SetBinContent(i+1, data[i]);
        if (debug)
            psi::LogInfo() << std::setw(4) << data[i] << " ";
	}
    if (debug)
        psi::LogInfo() << std::endl;
	histograms->Add(histo);
}

