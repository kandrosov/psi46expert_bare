/*!
 * \file TemperatureCalibration.cc
 * \brief Implementation of TemperatureCalibration class.
 *
 * \b Changelog
 * 02-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using psi::Sleep instead interface/Delay.
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using definitions from PsiCommon.h.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new ConfigParameters class definition.
 *      - Adaptation for the new TestParameters class definition.
 */

#include <fstream>

#include <TGraph.h>
#include <TF1.h>
#include <TMath.h>
#include <iomanip>

#include "TemperatureCalibration.h"
#include "psi/log.h"
#include "BasePixel/TBAnalogInterface.h"
#include "psi46expert/TestModule.h"
#include "BasePixel/TestParameters.h"
#include "psi/date_time.h"

Bool_t TemperatureCalibration::fPrintDebug = true;
//Bool_t TemperatureCalibration::fPrintDebug = false;

//--- temperatures to be reached in JUMO cooling/heating cycle
//    
//    WARNING: these values must exactly match those defined in the JUMO program !
//
Float_t TemperatureCalibration::fJumoTemperatures[fJumoNumTemperatures] 
 = {  30.,  28.,  26.,  24.,  22.,  20., 18., 16., 14., 12., 10., 8., 6., 4., 2.,  0., -2., -4., -6., -8., -10., -12., -14., -16., -18., -20, 
     -20., -18., -16., -14., -12., -10., -8., -6., -4., -2.,  0., 2., 4., 6., 8., 10., 12., 14., 16., 18.,  20.,  22.,  24.,  26.,  28.,  30 };

//--- mode of temperature approach
//
//    The mode flag indicates what JUMO commands need to be send to reach the target temperature.
//    Sometimes, two JUMO commands (one cooling and one heating step) are neccessary to **reach and stay** at the target temperature,
//    in order to compensate for the temperature drift of the cooling box towards the room temperature.
//
//      0 : just wait
//         --> no JUMO next command neccessary
//      1 : simple cool-down
//          (target temperature below room temperature)
//         --> only one JUMO next command neccessary
//      2 : cool-down with subsequent heating step to stay at target temperature 
//          (target temperature above room temperature)
//         --> two JUMO next commands neccessary
//      3 : simple heat-up
//          (target temperature above room temperature)
//         --> only one JUMO next command neccessary
//      4 : heat-up with subsequent cooling step to stay at target temperature 
//          (target temperature below room temperature)
//         --> two JUMO next commands neccessary
//    
//    WARNING: these values must exactly match those defined in the JUMO program !
//
Int_t TemperatureCalibration::fJumoMode[fJumoNumTemperatures] 
 = { 3, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3 };

//--- flags indicating whether or not to take last DAC temperature calibration data 
//    at certain temperatures
//    (useful to accelerate debugging...)
Int_t TemperatureCalibration::fJumoSkip[fJumoNumTemperatures] 
// = { 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
//     0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0 };
 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

//--- number of JUMO steps to be executed before and after temperature cycling begins
Int_t    TemperatureCalibration::fJumoNumStepsTotal = 76;
Int_t    TemperatureCalibration::fJumoNumStepsBegin = 1;
Int_t    TemperatureCalibration::fJumoNumStepsEnd   = 0;

//--- system commands to be send to JUMO for
//    setting program, reading temperature (and humidity) values,
//    starting the next step of the JUMO program and terminating the JUMO program
TString TemperatureCalibration::fJumoPath    = "/home/l_tester/ptr/JUMO/";
TString TemperatureCalibration::fJumoProgram = "Jumo -p 22"; // 23 = number of cooling/heating program in Jumo (subtract 1 to account for different index conventions)
TString TemperatureCalibration::fJumoPrint   = "Jumo > tmp_TemperatureCalibration.dat";
TString TemperatureCalibration::fJumoNext    = "Jumo -c n";
TString TemperatureCalibration::fJumoCancel  = "Jumo -c c";
Bool_t  TemperatureCalibration::fUseJumo     = false;
//Bool_t  TemperatureCalibration::fUseJumo     = true;

TemperatureCalibration::TemperatureCalibration(TestRange* aTestRange, TBInterface* aTBInterface)
{
//--- initialise data-structures inherited from "Test" base-class
  psi::Log<psi::Debug>() << "[TemperatureCalibration] Initialization." << std::endl;

  testRange = aTestRange;
  tbInterface = aTBInterface;

//--- initialise internal data-structures to default values
  fNumTemperatureCycles  = 2;
  fNumTrigger            = 1;
  fTemperatureTolerance1 = 0.10;
  fTemperatureTolerance2 = 0.25;

//--- set parameter values specified in steering file
  ReadTestParameters();

//--- check that values for target temperatures and modes of temperature approach
//    are consistent with the number of steps in the JUMO program
  Int_t sumJumoSteps = 0;
  for ( Int_t itemperature = 0; itemperature < fJumoNumTemperatures; itemperature++){
    switch ( fJumoMode[itemperature] ){
    case 0 : 
      break;
    case 1: 
      sumJumoSteps++;
      break;
    case 2 :
      sumJumoSteps += 2;
      break;
    case 3: 
      sumJumoSteps++;
      break;
    case 4 :
      sumJumoSteps += 2;
      break;
    default:
      std::cerr << "Error in <TemperatureCalibration::TemperatureCalibration>: mode " << fJumoMode[itemperature] << " not defined !" << std::endl;
    }
  }
  sumJumoSteps += fJumoNumStepsBegin;
  sumJumoSteps += fJumoNumStepsEnd;
  if ( sumJumoSteps != fJumoNumStepsTotal ){
    std::cerr << "Error in <TemperatureCalibration::TemperatureCalibration>: steps in JUMO program not consistent"
   << " (sumJumoSteps = " << sumJumoSteps << ", fJumoNumStepsTotal = " << fJumoNumStepsTotal << ") !" << std::endl;
  }
}

void TemperatureCalibration::ReadTestParameters()
{
    fNumTrigger = TestParameters::Singleton().TempNTrig();

//--- setting the number of triggers to a value higher than one seems to work now !
  //if ( fNumTrigger != 1 ){
  //  cerr << "Warning in <TemperatureCalibration::ReadTestParameters>: last DAC temperature readout only reliably works if number of trigger = 1 !" << endl;
  //  cerr << " (reason not yet known)" << endl;
  //}
}

void TemperatureCalibration::ReadTemperature(Float_t& temperature)
{
//--- read actual temperature from JUMO and write value into ASCII file
  if ( fUseJumo ){
    system(TString(fJumoPath).Append(fJumoPrint));

//--- read ASCII file written by JUMO
    TString fileName = "tmp_TemperatureCalibration.dat";
    ifstream inputFile(fileName);
    if ( !inputFile.is_open() ){
      std::cerr << "Error in <TemperatureCalibration::ReadTemperature>: file " << fileName << " does not exist !" << std::endl;
      temperature = 0;
      return;
    }
    char dummyString[100];
    inputFile >> dummyString;
    //cout << " 1. string = " << dummyString << endl;
    inputFile >> dummyString;
    //cout << " 2. string = " << dummyString << endl;
    inputFile >> temperature;
    //cout << " temperature = " << temperature << endl;
    inputFile >> dummyString;
    //cout << " 3. string = " << dummyString << endl;
  }
}


void TemperatureCalibration::ModuleAction()
{
//--- open files in which to write ASCII output
  fDtlGraph = new TGraph();
  fDtlGraph->SetName("DataTriggerLevel as function of Temperature");
  
  for ( Int_t iroc = 0; iroc < fNumROCs; iroc++ ){
    if ( testRange->IncludesRoc(module->GetRoc(iroc)->GetChipId()) ){
      char fileName[100];
      sprintf(fileName, "TemperatureCalibration_C%i.dat", module->GetRoc(iroc)->GetChipId());
      fOutputFiles[iroc] = new std::ofstream(TString(ConfigParameters::Singleton().Directory()).Append("/").Append(fileName), std::ios::out);

      TString histogramName = Form("adcTemperatureDependence_C%i", iroc);
      if ( fPrintDebug ) std::cout << "creating histogram " << histogramName << std::endl;
      fAdcTemperatureDependenceHistograms[iroc] = new TH2D(histogramName, histogramName, 26, -21, +31, 400, -2000, 2000);

      for ( Int_t rangeTemp = 0; rangeTemp < 8; rangeTemp++ ){
  TString histogramName = Form("adcFluctuation_C%i_TempRange%i", iroc, rangeTemp);
  if ( fPrintDebug ) std::cout << "creating histogram " << histogramName << std::endl;
  fAdcFluctuationHistograms[iroc][rangeTemp] = new TH1D(histogramName, histogramName, 2000, 0, 2000);
      }
    } else {
      fOutputFiles[iroc] = 0;
      fAdcTemperatureDependenceHistograms[iroc] = 0;
    }
  }

//--- initialise Jumo controller of cooling box
  if ( fUseJumo ){
    system(TString(fJumoPath).Append(fJumoProgram));
  }

//--- start cooling program
//    (flush N2 and cool down to +17 degrees Celsius)
  //for ( Int_t istep = 0; istep < fJumoNumStepsBegin; istep++ ){
  //  if ( fUseJumo ){
  //    system(TString(fJumoPath).Append(fJumoNext));
  //  }
  //}
//--- wait for the N2 flushing step (takes 2 minutes) to finish
  std::cout << "flushing JUMO with nitrogen (this will take 2 minutes)..." << std::endl;
  if ( fUseJumo ){
      psi::Sleep(120.0 * psi::seconds);
  }

//--- loop over all temperatures
  for ( Int_t itempcycle = 0; itempcycle < fNumTemperatureCycles; itempcycle++){
    for ( Int_t itemperature = 0; itemperature < fJumoNumTemperatures; itemperature++){
      Float_t targetTemperature = fJumoTemperatures[itemperature];
      Float_t actualTemperature;

      if ( fJumoSkip[itemperature] ){
  std::cout << "skipping steps corresponding to temperature " << targetTemperature << " degrees C in JUMO program..." << std::endl;
  switch ( fJumoMode[itemperature] ){
//    
//    WARNING: these steps must exactly match those defined in the JUMO program !
//
  case 0 : 
    break;
  case 1: 
    if ( fUseJumo ){
      system(TString(fJumoPath).Append(fJumoNext));
      psi::Sleep(2.5 * psi::seconds);
    }
    break;
  case 2 :
    if ( fUseJumo ){
      system(TString(fJumoPath).Append(fJumoNext));
      psi::Sleep(2.5 * psi::seconds);
      system(TString(fJumoPath).Append(fJumoNext));
      psi::Sleep(2.5 * psi::seconds);
    }
    break;
  case 3: 
    if ( fUseJumo ){
      system(TString(fJumoPath).Append(fJumoNext));
      psi::Sleep(2.5 * psi::seconds);
    }
    break;
  case 4 :
    if ( fUseJumo ){
      system(TString(fJumoPath).Append(fJumoNext));
      psi::Sleep(2.5 * psi::seconds);
      system(TString(fJumoPath).Append(fJumoNext));
      psi::Sleep(2.5 * psi::seconds);
    }
    break;
  }
  continue;
      } 

      std::cout << "setting JUMO temperature to " << targetTemperature << " degrees C..." << std::endl;

      const psi::Time waitPeriod = 10.0 * psi::seconds; // repeat temperature measurement every 10s
      const psi::Time waitMax = 15.0 * 60.0 * psi::seconds;   // wait for maximal 15 minutes
      psi::Time waitTotal = 0.0 * psi::seconds;

      switch ( fJumoMode[itemperature] ){
//    
//    WARNING: these steps must exactly match those defined in the JUMO program !
//
      case 0 : 
  break;
      case 1: 
  if ( fUseJumo ){
    system(TString(fJumoPath).Append(fJumoNext));
  }
  break;
      case 2 :
  if ( fUseJumo ){
    system(TString(fJumoPath).Append(fJumoNext));
    do {
      psi::Sleep(waitPeriod);
      waitTotal += waitPeriod;
      
      ReadTemperature(actualTemperature);
      
      if ( fPrintDebug ) std::cout << " waited for " << waitTotal << " : temperature = " << actualTemperature << " degrees C" << std::endl;
    } while ( actualTemperature > targetTemperature && waitTotal < waitMax );
    system(TString(fJumoPath).Append(fJumoNext));
  }
  break;
      case 3: 
  if ( fUseJumo ){
    system(TString(fJumoPath).Append(fJumoNext));
  }
  break;
      case 4 :
  if ( fUseJumo ){
    system(TString(fJumoPath).Append(fJumoNext));
    do {
        psi::Sleep(waitPeriod);
      waitTotal += waitPeriod;
      
      ReadTemperature(actualTemperature);
      
      if ( fPrintDebug ) std::cout << " waited for " << waitTotal << " : temperature = " << actualTemperature << " degrees C" << std::endl;
    } while ( actualTemperature < targetTemperature && waitTotal < waitMax );
    system(TString(fJumoPath).Append(fJumoNext));
  }
  break;
      }
      
      if ( fUseJumo ){
  Int_t numStableReadings1 = 0;
  Int_t numStableReadings2 = 0;
  do {
      psi::Sleep(waitPeriod);
    waitTotal += waitPeriod;
    
    ReadTemperature(actualTemperature);
    
    if ( fPrintDebug ) std::cout << " waited for " << waitTotal << " : temperature = " << actualTemperature << " degrees C" << std::endl;
    
    if ( TMath::Abs(actualTemperature - targetTemperature) <= fTemperatureTolerance1 )
      numStableReadings1++;
    else
      numStableReadings1 = 0;
    if ( TMath::Abs(actualTemperature - targetTemperature) <= fTemperatureTolerance2 )
      numStableReadings2++;
    else
      numStableReadings2 = 0;
  } while ( (!(numStableReadings2 >= 2 && numStableReadings1 >= 1)) && waitTotal < waitMax );
      
        
  if ( waitTotal >= waitMax ){
    std::cerr << "Error in <TemperatureCalibration::ModuleAction>: temperature does not stabilise !" << std::endl;
//--- terminate JUMO program
    system(TString(fJumoPath).Append(fJumoCancel));
    return;
  }
      }

      std::cout << "temperature in cooling box reached, starting temperature calibration..." << std::endl;
      std::cout << " actual temperature in cooling-box before test = " << actualTemperature << std::endl;

      ModuleAction_fixedTemperature();

      ReadTemperature(actualTemperature);
      std::cout << " actual temperature in cooling-box after test = " << actualTemperature << std::endl;
    }
  }

//--- terminate JUMO program
  if ( fUseJumo ){
    system(TString(fJumoPath).Append(fJumoCancel));
  }

//--- close output files 
//    and save histograms
  histograms->Add(fDtlGraph);
  fDtlGraph->Write();
  for ( Int_t iroc = 0; iroc < fNumROCs; iroc++ ){
    if ( fOutputFiles[iroc] != 0 ){
      delete fOutputFiles[iroc];
    }
    
    if ( fAdcTemperatureDependenceHistograms[iroc] != 0 ){
      histograms->Add(fAdcTemperatureDependenceHistograms[iroc]);
      fAdcTemperatureDependenceHistograms[iroc]->Write();
    }

    if ( !fUseJumo ){
      for ( Int_t rangeTemp = 0; rangeTemp < 8; rangeTemp++ ){
  if ( fAdcFluctuationHistograms[iroc][rangeTemp] != 0 ){
    histograms->Add(fAdcFluctuationHistograms[iroc][rangeTemp]);
    fAdcFluctuationHistograms[iroc][rangeTemp]->Write();
  }
      }
    }
  }
}

void TemperatureCalibration::ModuleAction_fixedTemperature(Bool_t addCalibrationGraph, Bool_t addMeasurementGraph)
{
//--- adjust data trigger level
//    (varies as function of temperature)
  std::cout << "adjusting data trigger-level..." << std::endl;
  module->AdjustDTL();
  std::cout << " data trigger-level set to " << ConfigParameters::Singleton().DataTriggerLevel() << std::endl;
  Float_t actualTemperature;
  ReadTemperature(actualTemperature);
  fDtlGraph->SetPoint(fDtlGraph->GetN(), actualTemperature, ConfigParameters::Singleton().DataTriggerLevel());

//--- take last DAC temperature calibration data for all selected ROCs
  for ( Int_t iroc = 0; iroc < module->NRocs(); iroc++ ){
    if ( testRange->IncludesRoc(module->GetRoc(iroc)->GetChipId()) ){
      std::cout << "taking calibration data for ROC " << module->GetRoc(iroc)->GetChipId() << "..." << std::endl;
      SetRoc(module->GetRoc(iroc).get());
      RocAction(fOutputFiles[iroc], addCalibrationGraph, addMeasurementGraph);
    }
  }
}

void TemperatureCalibration::RocAction(ofstream* outputFile, Bool_t addCalibrationGraph, Bool_t addMeasurementGraph)
{
  if ( roc->GetChipId() >= fNumROCs ){
    std::cerr << "Error in <TemperatureCalibration::RocAction>: no data-structures initialised for ROC " << roc->GetChipId() << " !" << std::endl;
    return;
  }

//--- measure black level
//    (substract as pedestal)
  unsigned short count;
  short data[psi::FIFOSIZE];
  
  TBAnalogInterface* anaInterface = (TBAnalogInterface*)tbInterface;
  if ( fPrintDebug ) std::cout << "NumTrigger = " << fNumTrigger << std::endl;
  anaInterface->ADCRead(data, count, fNumTrigger);
  short blackLevel = data[9 + aoutChipPosition*3];

  if ( fPrintDebug ) std::cout << "ADC(black level) = " << blackLevel << std::endl;
  
//--- measure ADC as function of calibration voltage

  TGraph* calibrationGraph = 0;
  if ( addCalibrationGraph ){
    calibrationGraph = new TGraph();
    calibrationGraph->SetName(Form("TempCalibration_C%i", chipId));
  }

  *outputFile << "T = ";
  Float_t actualTemperature;
  ReadTemperature(actualTemperature);
  if ( actualTemperature > 0 ){
    *outputFile << "+";
  }
  *outputFile << actualTemperature << " : blackLevel = " << blackLevel << ", Vcalibration = { ";

  for ( Int_t rangeTemp = 0; rangeTemp < 8; rangeTemp++ ){
    SetDAC("RangeTemp", rangeTemp + 8);
    Flush();

    psi::Sleep(0.25 * psi::seconds);

    int adcDifference_average = anaInterface->LastDAC(fNumTrigger, aoutChipPosition) - blackLevel;
    
    double voltageDifference = 470. - (399.5 + rangeTemp*23.5);
    if ( addCalibrationGraph ) calibrationGraph->SetPoint(rangeTemp, voltageDifference, adcDifference_average);
    
    if ( !fUseJumo ) fAdcFluctuationHistograms[roc->GetChipId()][rangeTemp]->Fill(adcDifference_average);

    if ( fPrintDebug ) std::cout << " ADC(deltaV = " << voltageDifference << " mV) = " << adcDifference_average << std::endl;

    *outputFile << std::setw(6) << adcDifference_average << " ";
  }
  
  *outputFile << "}, Vtemperature = { ";

  if ( addCalibrationGraph ){
    histograms->Add(calibrationGraph);
    calibrationGraph->Write();
  }
  
//--- measure ADC for actual temperature
  TGraph* measurementGraph = 0;
  if ( addMeasurementGraph ){
    measurementGraph = new TGraph();
    measurementGraph->SetName(Form("TempMeasurement_C%i", chipId));
  }

  for ( Int_t rangeTemp = 0; rangeTemp < 8; rangeTemp++ ){
    SetDAC("RangeTemp", rangeTemp);
    Flush();

    psi::Sleep(0.25 * psi::seconds);

    int adcDifference_average = anaInterface->LastDAC(fNumTrigger, aoutChipPosition) - blackLevel;
    
    fAdcTemperatureDependenceHistograms[roc->GetChipId()]->Fill(actualTemperature, adcDifference_average);

    if ( addMeasurementGraph ) measurementGraph->SetPoint(rangeTemp, rangeTemp, adcDifference_average);

    if ( fPrintDebug ) std::cout << "ADC(T = " << actualTemperature << ", " << 399.5 + rangeTemp*23.5 << " mV) = " << adcDifference_average << std::endl;
    
    *outputFile << std::setw(6) << adcDifference_average << " ";
  }

  *outputFile << "}" << std::endl;

  if ( addMeasurementGraph ){
    histograms->Add(measurementGraph);
    measurementGraph->Write();
  }
}


