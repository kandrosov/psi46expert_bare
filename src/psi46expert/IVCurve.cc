/*!
 * \file IVCurve.cc
 * \brief Implementation of IVCurve class.
 *
 * \b Changelog
 * 18-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - IVCurve test algorithm changed for the bare module tests.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new ConfigParameters class definition.
 *      - Adaptation for the new TestParameters class definition.
 * 10-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - IVoltageSource interface was changed.
 * 30-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Changed to support IVoltageSource interface.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Changed to support IHighVoltageSource interface.
 */

#include <iostream>

#include <TGraph.h>

#include "IVCurve.h"
#include "BasePixel/Keithley237.h"
#include "interface/Log.h"
#include "BasePixel/ConfigParameters.h"
#include "TestParameters.h"

IVCurve::IVCurve(TestRange*, TBInterface*)
    : delay(0)
{
  psi::LogDebug() << "[IVCurve] Initialization." << psi::endl;

    ReadTestParameters();
//	ConfigParameters *configParameters = ConfigParameters::Singleton();
//	if (!configParameters->keithleyRemote)
//        THROW_PSI_EXCEPTION("IVCurve::IVCurve ERROR: can't do IV in manual mode");
    const Keithley237::Configuration config("keithley");
    hvSource = boost::shared_ptr<IVoltageSource>(new Keithley237(config));
}

void IVCurve::ReadTestParameters()
{
    const TestParameters& testParameters = TestParameters::Singleton();
    voltStep = testParameters.IVStep();
    voltStart = testParameters.IVStart();
    voltStop = testParameters.IVStop();
    const double delay_in_ms = testParameters.IVDelay() / (0.001 * psi::seconds);
    delay = boost::posix_time::milliseconds(delay_in_ms);
}

void IVCurve::ModuleAction()
{
    double v, c;
	const int nSteps = (voltStop - voltStart)/voltStep;
	float voltage[nSteps], current[nSteps];

	int stepsDone = 0;
    for (psi::ElectricPotential i = voltStart; i < voltStop; i += voltStep)
	{
        IVoltageSource::Value value(i, 1.0 * psi::amperes);
        hvSource->Set(value);
        boost::this_thread::sleep(delay);
        const IVoltageSource::Measurement measurement = hvSource->Measure();
        c = measurement.Current / CURRENT_FACTOR;
        v = measurement.Voltage / VOLTAGE_FACTOR;
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
    psi::ElectricPotential rdStep = voltStep * 4.0;
    for (psi::ElectricPotential i = voltStop; i >= 150.0 * psi::volts; i-=rdStep)
    {
        IVoltageSource::Value value(i, 1.0 * psi::amperes );
        hvSource->Set(value);
        sleep(1);
    }
  psi::LogDebug() << "[IVCurve] Reset Keithley to -150V." << psi::endl;

    IVoltageSource::Value value(150.0 * psi::volts, 1.0 * psi::amperes );
    hvSource->Set(value);
    sleep(3);

//  psi::LogDebug() << "[IVCurve] Reset Keithley to local mode." << psi::endl;

//	keithley->GoLocal();

  psi::LogDebug() << "[IVCurve] Done." << psi::endl;
	
	// write result to file

	char fileName[1000];
    const ConfigParameters& configParameters = ConfigParameters::Singleton();
    sprintf(fileName, "%s/iv.dat", configParameters.Directory().c_str());
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
