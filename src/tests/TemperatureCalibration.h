/*!
 * \file TemperatureCalibration.h
 * \brief Definition of TemperatureCalibration class.
 */

#pragma once

#include <TString.h>
#include <TH1.h>
#include <TGraph.h>
#include "BasePixel/Test.h"

/*!
 * \brief Calibration and Test of last DAC temperature readout of ROCs
 */
class TemperatureCalibration : public Test {
public:
    TemperatureCalibration(TestRange* testRange, TBInterface* aTBInterface);

    virtual void ReadTestParameters();
    virtual void ModuleAction();

protected:
    void ReadTemperature(Float_t& temperature);
    void ModuleAction_fixedTemperature(Bool_t addCalibrationGraph = false, Bool_t addMeasurementGraph = false);
    virtual void RocAction(ofstream* outputFile, Bool_t addCalibrationGraph = false, Bool_t addMeasurementGraph = false);

    static const Int_t fNumROCs = 16;
    static const Int_t fJumoNumTemperatures = 52;
    static Float_t     fJumoTemperatures[fJumoNumTemperatures];
    static Int_t       fJumoMode[fJumoNumTemperatures];
    static Int_t       fJumoSkip[fJumoNumTemperatures];
    static Int_t       fJumoNumStepsTotal;
    static Int_t       fJumoNumStepsBegin;
    static Int_t       fJumoNumStepsEnd;

    static TString fJumoPath;
    static TString fJumoProgram;
    static TString fJumoPrint;
    static TString fJumoNext;
    static TString fJumoCancel;
    static Bool_t  fUseJumo;

    Int_t   fNumTemperatureCycles;
    Int_t   fNumTrigger;
    Float_t fTemperatureTolerance1;
    Float_t fTemperatureTolerance2;

    static Bool_t fPrintDebug;

    TGraph* fDtlGraph;
    TH2* fAdcTemperatureDependenceHistograms[fNumROCs];
    TH1* fAdcFluctuationHistograms[fNumROCs][8];
    ofstream* fOutputFiles[fNumROCs];
};
