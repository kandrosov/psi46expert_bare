/*!
 * \file IVCurve.cc
 * \brief Implementation of IVCurve class.
 *
 * \b Changelog
 * 30-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Changed to support IVoltageSource interface.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Changed to support IHighVoltageSource interface.
 */

#include <iostream>

#include <TGraph.h>

#include "IVCurve.h"
#include "BasePixel/Keithley.h"
#include "interface/Log.h"
#include "BasePixel/ConfigParameters.h"

IVCurve::IVCurve(TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface)
{
  psi::LogDebug() << "[IVCurve] Initialization." << psi::endl;

	ReadTestParameters(testParameters);
	
	ConfigParameters *configParameters = ConfigParameters::Singleton();
	if (!configParameters->keithleyRemote)
        THROW_PSI_EXCEPTION("IVCurve::IVCurve ERROR: can't do IV in manual mode");
    hvSource = boost::shared_ptr<IVoltageSource>(new Keithley());
}

void IVCurve::ReadTestParameters(TestParameters *testParameters)
{
	voltStep = (*testParameters).IVStep;
	voltStart = (*testParameters).IVStart;
	voltStop = (*testParameters).IVStop;
	delay = (*testParameters).IVDelay;
}

void IVCurve::ModuleAction()
{
    double v, c;
	const int nSteps = (voltStop - voltStart)/voltStep;
	
	float voltage[nSteps], current[nSteps];

	int stepsDone = 0;
	for (int i = voltStart; i < voltStop; i+=voltStep)
	{
        hvSource->Set(i);
        sleep(delay);
        const IVoltageSource::Measurement measurement = hvSource->Measure();
        c = measurement.Current;
        v = measurement.Voltage;
		voltage[stepsDone] = TMath::Abs(v);
		current[stepsDone] = TMath::Abs(c);

    psi::LogDebug() << "[IVCurve] Voltage " << v << ", Current " << c << '.'
                    << psi::endl;

		if ((c < -9.9e-5) && (v != 0.))
		{
			cout << "KEITHLEY TRIP, IV test will end" << endl;
			voltStop = i; 
			break;
		}
		stepsDone++;
	}

	cout << "ramping down from voltStop: " << voltStop << endl;

	// ramp down voltage
	int rdStep = voltStep * 4;
    for (int i = voltStop; i >= 150; i-=rdStep)
    {
        hvSource->Set(i);
        sleep(1);
    }
  psi::LogDebug() << "[IVCurve] Reset Keithley to -150V." << psi::endl;

    hvSource->Set(150);
    sleep(3);

//  psi::LogDebug() << "[IVCurve] Reset Keithley to local mode." << psi::endl;

//	keithley->GoLocal();

  psi::LogDebug() << "[IVCurve] Done." << psi::endl;
	
	// write result to file

	char fileName[1000];
	ConfigParameters *configParameters = ConfigParameters::Singleton();
	sprintf(fileName, "%s/iv.dat", configParameters->directory);	
	FILE* f = fopen(fileName, "w");
	fprintf(f, "Voltage [V] Current [A]\n\n");
	
	TGraph *graph = new TGraph(nSteps, voltage, current);
	graph->SetTitle("IVCurve");
	graph->SetName("IVCurve");
	histograms->Add(graph);
	graph->Write();
	
	for (int i = 0; i < stepsDone; i++) fprintf(f, "%e %e\n", voltage[i], current[i]);
	fclose(f);
}
