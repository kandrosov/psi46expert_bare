/*!
 * \file TestModule.cc
 * \brief Implementation of TestModule class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new ConfigParameters class definition.
 *      - MainFrame removed due to compability issues.
 *      - Adaptation for the new TestParameters class definition.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - removed deprecated conversion from string constant to char*
 * 22-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Added BareTest command support.
 */

#include <TH1D.h>
#include <TFile.h>
#include <TApplication.h>
#include <TParameter.h>
#include "SCurveTest.h"

#include "interface/Delay.h"
#include "interface/Log.h"

#include "TestModule.h"
#include "BasePixel/TBAnalogInterface.h"
#include "interface/Delay.h"
#include "TestRange.h"
#include "FullTest.h"
#include "PHCalibration.h"
#include "ThresholdMap.h"
#include "IVCurve.h"
#include "UbCheck.h"
#include "TBMUbCheck.h"
#include "TBMTest.h"
#include "AnalogReadout.h"
#include "AddressLevels.h"
#include "VsfOptimization.h"
#include "TimeWalkStudy.h"
#include "PixelAlive.h"
#include "BumpBonding.h"
#include "TrimLow.h"
#include "PHRange.h"
#include "Xray.h"

#include "BareTest.h"

TestModule::TestModule(int aCNId, TBInterface *aTBInterface)
{
  const ConfigParameters& configParameters = ConfigParameters::Singleton();

  if (configParameters.CustomModule() == 0)
  {
      nRocs = configParameters.NumberOfRocs();

    tbm = new TBM(aCNId, aTBInterface);
    tbm->init();
    hubId = configParameters.HubId();

    controlNetworkId = aCNId;
    tbInterface = aTBInterface;

    int offset = 0;
    if (configParameters.HalfModule() == 2) offset = 8;
    for (int i = 0; i < nRocs; i++)
    {
      roc[i] = new TestRoc(tbInterface, i+offset, hubId, int((i+offset)/4), i);
    }
  }
  else if (configParameters.CustomModule() == 1)
  {
    psi::LogInfo() << "[TestModule] Custom module constructor: Ignoring nRocs, "
                   << "hubID, ... in config file." << psi::endl;
    psi::LogInfo() << "[TestModule] Using: 4 ROCs with ChipID/PortID 0,1/0 "
                   << "and 8,9/2." << psi::endl;
                
    nRocs = 4;
    hubId = 0;
    controlNetworkId = aCNId;
    tbInterface = aTBInterface;

    tbm = new TBM(aCNId, tbInterface);
    tbm->init();

    nRocs = 4;
    roc[1] = new TestRoc(tbInterface, 0, hubId, 0, 1);
    roc[0] = new TestRoc(tbInterface, 1, hubId, 0, 0);
    roc[2] = new TestRoc(tbInterface, 8, hubId, 2, 2);
    roc[3] = new TestRoc(tbInterface, 9, hubId, 2, 3);
  }
}


TestRoc* TestModule::GetRoc(int iRoc) {
  return (TestRoc*)roc[iRoc];
}


void TestModule::Execute(SysCommand &command)
{
  if (command.TargetIsTBM()) {tbm->Execute(command);}
  else
  {
    if (strcmp(command.carg[0],"TestM") == 0) {TestM();}
    else if (strcmp(command.carg[0],"DigiCurrent") == 0) {DigiCurrent();}
    else if (command.Keyword("Test")) {DoTest(new FullTest(GetRange(command), tbInterface,1));}
    else if (command.Keyword("FullTest")) {DoTest(new FullTest(GetRange(command), tbInterface,1));}
    else if (!strcmp(command.carg[0],"BareTest")) {DoTest(new BareTest(GetRange(command), (TBAnalogInterface*)tbInterface,command.carg[1]));}
    else if (command.Keyword("xray")) {DoTest(new Xray(GetRange(command), tbInterface));}
    else if (command.Keyword("FullTestAndCalibration")) {FullTestAndCalibration();}
    else if (command.Keyword("DumpParameters")) {DumpParameters();}
    else if (command.Keyword("Temp")) {GetTemperature();}
    else if (command.Keyword("adc")) {((TBAnalogInterface*)tbInterface)->ADC();}
	else if (command.Keyword("adc_fix")) {((TBAnalogInterface*)tbInterface)->ADC(200);}//
    else if (command.Keyword("dtlScan")) {DataTriggerLevelScan();}
    else if (strcmp(command.carg[0],"adjustUB") == 0) AdjustUltraBlackLevel();
    else if (strcmp(command.carg[0],"adjustDTL") == 0) AdjustDTL();
    else if (strcmp(command.carg[0],"adjustSmp") == 0) AdjustSamplingPoint();
    else if (strcmp(command.carg[0],"adjust") == 0) AdjustDACParameters();
    else if (strcmp(command.carg[0],"Pretest") == 0) AdjustDACParameters();
                else if (strcmp(command.carg[0],"NewPretest") == 0) AdjustAllDACParameters();
    else if (command.Keyword("TBMTest")) {DoTBMTest();}
    else if (command.Keyword("AnaReadout")) {AnaReadout();}
    else if (command.Keyword("DACProgramming")) {TestDACProgramming();}
 		else if (command.Keyword("Scurves")) {Scurves();}
    else 
    {
      bool done = false;
      for (int i = 0; i < nRocs; i++)
      {
        if (roc[i]->GetChipId() == command.roc)
        {
          GetRoc(i)->Execute(command);
          done = true;
        }
      }
      if (!done)
        psi::LogInfo() << "[TestModule] Error: Roc #" << command.roc
                       << " does not exist." << psi::endl;

      return;
    }
    command.RocsDone();
  }
  
}

void TestModule::FullTestAndCalibration()
{
  AdjustDACParameters();
    DoTest(new FullTest(FullRange(), tbInterface,1));

  psi::LogInfo() << "[TestModule] PhCalibration: Start." << psi::endl;

  for (int i = 0; i < nRocs; i++) GetRoc(i)->DoPhCalibration();

  psi::LogInfo() << "[TestModule] PhCalibration: End." << psi::endl;

  psi::LogInfo() << "[TestModule] Trim: Start." << psi::endl;

  for (int i = 0; i < nRocs; i++) GetRoc(i)->DoTrim();

  psi::LogInfo() << "[TestModule] Trim: End." << psi::endl;
}


void TestModule::ShortCalibration()
{
  AdjustAllDACParameters();        
  VanaVariation();
  DoTest(new PixelAlive(FullRange(), tbInterface));

  psi::LogInfo() << "[TestModule] PhCalibration: Start." << psi::endl;

  for (int i = 0; i < nRocs; i++) GetRoc(i)->DoPhCalibration();

  psi::LogInfo() << "[TestModule] PhCalibration: End." << psi::endl;
}


void TestModule::ShortTestAndCalibration()
{
  AdjustAllDACParameters();        
  VanaVariation();
  DoTest(new PixelAlive(FullRange(), tbInterface));
  DoTest(new BumpBonding(FullRange(), tbInterface));

  psi::LogInfo() << "[TestModule] PhCalibration: Start." << psi::endl;

  for (int i = 0; i < nRocs; i++) GetRoc(i)->DoPhCalibration();

  psi::LogInfo() << "[TestModule] PhCalibration: End." << psi::endl;
  
  psi::LogInfo() << "[TestModule] Trim: Start." << psi::endl;

  TrimLow *trimLow = new TrimLow(FullRange(), tbInterface);
  trimLow->SetVcal(40);
  //trimLow->NoTrimBits(true);
  DoTest(trimLow);

  psi::LogInfo() << "[TestModule] Trim: End." << psi::endl;
}


void TestModule::DoTest(Test *aTest)
{
  aTest->ModuleAction(this);
}


TestRange *TestModule::GetRange(SysCommand &command)
{
  TestRange *range = new TestRange();
  for (int i = 0; i < nRocs; i++)
  {
    if (command.IncludesRoc(GetRoc(i)->GetChipId())) range->CompleteRoc(GetRoc(i)->GetChipId());
  }
  return range;
}


TestRange *TestModule::FullRange()
{
  TestRange *range = new TestRange();
  for (int i = 0; i < nRocs; i++) range->CompleteRoc(GetRoc(i)->GetChipId());
  return range;
}


void TestModule::DoTBMTest()
{
  Test *aTest = new TBMTest(new TestRange(), tbInterface);
  gDelay->Timestamp();
  aTest->ModuleAction(this);
  gDelay->Timestamp();
}


void TestModule::AnaReadout()
{
  Test *aTest = new AnalogReadout(new TestRange(), tbInterface);
  gDelay->Timestamp();
  aTest->ModuleAction(this);
  gDelay->Timestamp();
}


// -- Function where you can quickly implement some tests
// -- This code is not persistent
void TestModule::TestM()
{
   VanaVariation();
}


// void TestModule::DigiCurrent()
// {
//   double dc = 0.;
//   //  for(int dacRegister = 1; dacRegister < 28; dacRegister++)
//       for(int dacRegister = 3; dacRegister < 4; dacRegister++)
//     {
//       DACParameters* parameters = new DACParameters();
//       char *dacName = parameters->GetName(dacRegister);

//       TH1D *currentHist = new TH1D(Form("currentHist%i",dacRegister), Form("%s",dacName), 26, 0, 260);

//       for(int dacValue = 0; dacValue < 260; dacValue+=10)
//  {
//    nRocs = 16;
//    //    for (int iRoc = 1; iRoc < nRocs; iRoc++)
//            for (int iRoc = 0; iRoc < nRocs; iRoc++)
//      {
//        GetRoc(iRoc)->SaveDacParameters();
//        GetRoc(iRoc)->SetDAC(dacRegister, dacValue);
//      }
//    cout << dacName << " set to " << dacValue << endl;
//    tbInterface->Flush();
//    sleep(2.);
//    //dc = ((TBAnalogInterface*)tbInterface)->GetIA();
//    dc = ((TBAnalogInterface*)tbInterface)->GetID();
//    cout << "Digital current: " << dc << endl;
//    currentHist->SetBinContent((dacValue/10)+1,dc);
//    for (int iRoc = 0; iRoc < nRocs; iRoc++) GetRoc(iRoc)->RestoreDacParameters();    
//  }
//     }
  

// }




void TestModule::DigiCurrent()
{
  double dc = 0.;
  //  for(int dacRegister = 1; dacRegister < 28; dacRegister++)
  for(int dacRegister = 3; dacRegister < 4; dacRegister++)
    {
      DACParameters* parameters = new DACParameters();
      const char *dacName = parameters->GetName(dacRegister);
      
      nRocs = 16;
      for (int iRoc = 0; iRoc < nRocs; iRoc++)
  {
    
    TH1D *currentHist = new TH1D(Form("currentHist%i_ROC%i",dacRegister,iRoc), Form("%s",dacName), 26, 0, 260);

    cout << "Testing ROC " << iRoc << endl;  
 
    GetRoc(iRoc)->SaveDacParameters();
    for(int dacValue = 0; dacValue < 260; dacValue+=10)
      {
        GetRoc(iRoc)->SetDAC(dacRegister, dacValue);
        
        cout << dacName << " set to " << dacValue << endl;
        tbInterface->Flush();
        sleep(2);
        //dc = ((TBAnalogInterface*)tbInterface)->GetIA();
        dc = ((TBAnalogInterface*)tbInterface)->GetID();
        cout << "Digital current: " << dc << endl;
        currentHist->SetBinContent((dacValue/10)+1,dc);
      }
    GetRoc(iRoc)->RestoreDacParameters();   
  }
    }

}




void TestModule::AdjustSamplingPoint()
{
  bool debug = false;
  short data[10000], ph[25];
  unsigned short count;
  FILE *file;

  if (debug)
  {
    char fname[1000];
    sprintf(fname, "sampling.dat");
    file = fopen(fname, "w");
    if (!file) return;
  }

  int ctrlReg = GetRoc(0)->GetDAC("CtrlReg");
  int vcal = GetRoc(0)->GetDAC("Vcal");
  
  GetRoc(0)->SetDAC("CtrlReg", 4);
  GetRoc(0)->SetDAC("Vcal", 255);
  GetRoc(0)->AdjustCalDelVthrComp(5, 5, 255, 0);
  
  int clk = tbInterface->GetParameter("clk");
  int sda = tbInterface->GetParameter("sda");
  int ctr = tbInterface->GetParameter("ctr");
  int tin = tbInterface->GetParameter("tin");
  int rda = tbInterface->GetParameter("rda");
  
  if (debug) printf("clk %i\n", clk);
  const ConfigParameters& configParameters = ConfigParameters::Singleton();

  for (int delay = 0; delay < 25; ++delay)
  { 
    if (debug)
      psi::LogDebug() << "[TestModule] Delay " << delay << '.' << psi::endl;
    
    tbInterface->SetTBParameter("clk", (clk + delay) );
    tbInterface->SetTBParameter("sda", (sda + delay) );
    tbInterface->SetTBParameter("ctr", (ctr + delay) );
    tbInterface->SetTBParameter("tin", (tin + delay) );
    if(configParameters.TbmEmulator()) tbInterface->SetTBParameter("rda", (100- (tin + delay)) );  // ask chris to be true with the tbmemulator


    tbInterface->Flush();
    
    int nTrig = 10;
    
    GetRoc(0)->ArmPixel(5,5);  //pixel must not be enabled during setting of the tb parameters above
    tbInterface->Flush();
    
    ((TBAnalogInterface*)tbInterface)->ADCRead(data, count, nTrig);
    
    GetRoc(0)->DisarmPixel(5,5);
    tbInterface->Flush();
    
    if (count > 16) ph[delay] = data[16]; else ph[delay] = -9999;
    
    if (debug)
    {
      printf("count %i\n", count);    
      fprintf(file, "%+.3i: ", clk+delay);
      for (int i = 0; i < count; i++) fprintf(file, "%+.3i ", data[i]);
      fprintf(file, "\n");
    }
  }
  if (debug) fclose(file);
  
  GetRoc(0)->SetDAC("CtrlReg", ctrlReg);
  GetRoc(0)->SetDAC("Vcal", vcal);
  tbInterface->Flush();
  
  short maxPH = -9999;
  int maxDelay, lowerDelay, upperDelay;
  for (int i = 0; i < 25; i++) 
  {
    if (debug) printf("ph %i\n", ph[i]);
    if (ph[i] > maxPH) 
    {
      maxPH = ph[i];
      maxDelay = i;
    }
  }
  
  upperDelay = maxDelay;
  do
  {
    upperDelay++;
    if (upperDelay == 25) upperDelay = 0;
  }
  while (ph[upperDelay] > maxPH - 20);

  lowerDelay = maxDelay;
  do
  {
    lowerDelay--;
    if (lowerDelay == -1) lowerDelay = 24;
  }
  while (ph[lowerDelay] > maxPH - 20);
    
  if (debug) printf("maxPH %i max %i lower %i upper %i\n", maxPH, maxDelay, lowerDelay, upperDelay);
  
  int diff, bestDelay;
  if (upperDelay > lowerDelay) diff = upperDelay - lowerDelay; else diff = 25 - (lowerDelay - upperDelay);
  if (debug) printf("diff %i\n", diff);
  bestDelay = lowerDelay + diff/2;
  if (bestDelay > 24) bestDelay -= 25;
  
  printf("Setting sampling point to %i\n", (clk + bestDelay));
  
  int bestRda = rda - bestDelay;  //opposite direction than other delays
  if (bestRda < 0) bestRda+=25;
  
  tbInterface->SetTBParameter("clk", (clk + bestDelay));
  tbInterface->SetTBParameter("sda", (sda + bestDelay));
  tbInterface->SetTBParameter("ctr", (ctr + bestDelay));
  tbInterface->SetTBParameter("tin", (tin + bestDelay));
  tbInterface->SetTBParameter("rda", bestRda);
  if(configParameters.TbmEmulator())
	  {
	    cout<<"I'm in the tbmEmulator"<<endl;
	    tbInterface->SetTBParameter("rda", (100- (tin + bestDelay)) );  // ask chris
	  }


  tbInterface->Flush();
  tbInterface->WriteTBParameterFile(configParameters.TbParametersFileName().c_str());
}


void TestModule::AdjustAllDACParameters()
{
  bool tbmPresent = ((TBAnalogInterface*)tbInterface)->TBMPresent();

  psi::LogInfo() << "[TestModule] Pretest Extended: Start." << psi::endl;

  tbInterface->Single(0);
  gDelay->Timestamp();

  const ConfigParameters& configParameters = ConfigParameters::Singleton();
  if (tbmPresent)
  {
    ((TBAnalogInterface*)tbInterface)->SetTriggerMode(TRIGGER_MODULE1); // trigger mode 2 only works correctly after adjusting tbm and roc ultrablacks to the same level
      if(!configParameters.TbmEmulator())  AdjustTBMUltraBlack();
    AdjustDTL();
  }
  TestDACProgramming();
  AdjustVana(0.024);
  gDelay->Timestamp();
  if (tbmPresent) 
  {
    AdjustUltraBlackLevel();
    ((TBAnalogInterface*)tbInterface)->SetTriggerMode(TRIGGER_MODULE2);
    AdjustSamplingPoint();
  }
  gDelay->Timestamp();
//        AdjustCalDelVthrComp();
//        DoTest(new TimeWalkStudy(FullRange(), tbInterface));
  AdjustCalDelVthrComp();
        
  AdjustPHRange();
  DoTest(new VsfOptimization(FullRange(), tbInterface));

  gDelay->Timestamp();
  MeasureCurrents();
  WriteDACParameterFile(configParameters.DacParametersFileName().c_str());
  CalibrateDecoder();
  ADCHisto();

  psi::LogInfo() << "[TestModule] Pretest Extended: End." << psi::endl;
}


void TestModule::AdjustDACParameters()
{
  bool tbmPresent = dynamic_cast<TBAnalogInterface *>( tbInterface)->TBMPresent();

  psi::LogInfo() << "[TestModule] Pretest: Start." << psi::endl;

  tbInterface->Single(0);
  gDelay->Timestamp();

  const ConfigParameters& configParameters = ConfigParameters::Singleton();

  if (tbmPresent)
  {
    ((TBAnalogInterface*)tbInterface)->SetTriggerMode(TRIGGER_MODULE1); // trigger mode 2 only works correctly after adjusting tbm and roc ultrablacks to the same level
      if(!configParameters.TbmEmulator())   AdjustTBMUltraBlack();
    AdjustDTL();
  }
  TestDACProgramming();
  AdjustVana(0.024);
  MeasureCurrents();
  if (tbmPresent) 
  {
    AdjustUltraBlackLevel();
    ((TBAnalogInterface*)tbInterface)->SetTriggerMode(TRIGGER_MODULE2);
    AdjustSamplingPoint();
  }
  for (int iRoc = 0; iRoc < nRocs; iRoc++)
  { 
    psi::LogDebug() << "[TestModule] Roc #" << GetRoc(iRoc)->GetChipId()
                    << '.' << psi::endl;

    GetRoc(iRoc)->AdjustCalDelVthrComp();
  }
  AdjustVOffsetOp();

  gDelay->Timestamp();
  WriteDACParameterFile(configParameters.DacParametersFileName().c_str());
  CalibrateDecoder();
  ADCHisto();

  psi::LogInfo() << "[TestModule] Pretest: End." << psi::endl;
}


void TestModule::AdjustCalDelVthrComp()
{
  for (int iRoc = 0; iRoc < nRocs; iRoc++)
  { 
    psi::LogDebug() << "[TestModule] Roc #" << GetRoc(iRoc)->GetChipId()
                    << '.' << psi::endl;

    GetRoc(iRoc)->AdjustCalDelVthrComp();
  }
}


void TestModule::ADCHisto()
{
  unsigned short count;
  short data[FIFOSIZE];
  ((TBAnalogInterface*)tbInterface)->ADCRead(data, count, 100);
  TH1D *hist = new TH1D("ADC","ADC", count, 0., count);
  for (unsigned int n = 0; n < count; n++) hist->SetBinContent(n+1,data[n]);
}


void TestModule::AdjustVOffsetOp()
{
  psi::LogInfo() << "[TestModule] Adjust VOffsetOp." << psi::endl;

  gDelay->Timestamp();
  
  TestRange *testRange = new TestRange();
  testRange->CompleteRange();
  Test *aTest = new UbCheck(testRange, tbInterface);
  aTest->ModuleAction(this);
}


void TestModule::AdjustPHRange()
{
  psi::LogInfo() << "[TestModule] Adjust PhRange." << psi::endl;

  gDelay->Timestamp();
  
  TestRange *testRange = new TestRange();
  testRange->CompleteRange();
  Test *aTest = new PHRange(testRange, tbInterface);
  aTest->ModuleAction(this);
}


void TestModule::CalibrateDecoder()
{
  psi::LogInfo() << "[TestModule] Calibrate Decoder (AddressLevels test)."
                 << psi::endl;

  gDelay->Timestamp();
  
  TestRange *testRange = new TestRange();
  testRange->CompleteRange();
  Test *aTest = new AddressLevels(testRange, tbInterface);
  aTest->ModuleAction(this);
}


void TestModule::AdjustTBMUltraBlack()
{
  gDelay->Timestamp();
  
  TestRange *testRange = new TestRange();
  testRange->CompleteRange();
  Test *aTest = new TBMUbCheck(testRange, tbInterface);
  aTest->ModuleAction(this);
}


// Tries to automatically adjust Vana, may not work yet
void TestModule::AdjustVana(double goalCurrent)
{
  if (!tbInterface->IsAnalogTB()) return;
  TBAnalogInterface* anaInterface = (TBAnalogInterface*)tbInterface;
  int vana[nRocs];
  int vsf[nRocs];

  for (int iRoc = 0; iRoc < nRocs; iRoc++)
  {
    vsf[iRoc] = GetRoc(iRoc)->GetDAC("Vsf");

    GetRoc(iRoc)->SetDAC("Vana", 0);
    GetRoc(iRoc)->SetDAC("Vsf", 0);
  }
  tbInterface->Flush();
  gDelay->Mdelay(500);
  double current0 = anaInterface->GetIA();

  psi::LogDebug() << "[TestModule] ZeroCurrent " << current0 << psi::endl;

  for (int iRoc = 0; iRoc < nRocs; iRoc++)
  {
    vana[iRoc] = GetRoc(iRoc)->AdjustVana(current0, goalCurrent);
    GetRoc(iRoc)->SetDAC("Vana", 0);
  }
  for (int iRoc = 0; iRoc < nRocs; iRoc++)
  {
    GetRoc(iRoc)->SetDAC("Vana", vana[iRoc]);
    GetRoc(iRoc)->SetDAC("Vsf", vsf[iRoc]);
  }
  tbInterface->Flush();
  gDelay->Mdelay(500);
  double current = anaInterface->GetIA();

  psi::LogDebug() << "[TestModule] TotalCurrent " << current << psi::endl;
}



void TestModule::VanaVariation()
{
  bool debug = true;

  psi::LogInfo() << "[TestModule] VanaVariation." << psi::endl;

  TBAnalogInterface* anaInterface = (TBAnalogInterface*)tbInterface;
        int vsf[nRocs], vana[nRocs];
        double x[3], y[3];
        
  for (int iRoc = 0; iRoc < nRocs; iRoc++)
  {
    vsf[iRoc] = GetRoc(iRoc)->GetDAC("Vsf");
          vana[iRoc] = GetRoc(iRoc)->GetDAC("Vana");

    GetRoc(iRoc)->SetDAC("Vana", 0);
    GetRoc(iRoc)->SetDAC("Vsf", 0);
  }
  tbInterface->Flush();
  gDelay->Mdelay(2000);
        
  double current0 = anaInterface->GetIA();
  psi::LogDebug() << "[TestModule] ZeroCurrent " << current0 << psi::endl;
        
  for (int iRoc = 0; iRoc < nRocs; iRoc++)
  {
    if (debug)
      psi::LogDebug() << "[TestModule] Roc #" << iRoc << '.' << psi::endl;
        
    GetRoc(iRoc)->SetDAC("Vana", vana[iRoc]-10);
    tbInterface->Flush();
    gDelay->Mdelay(1000);
    x[0] = vana[iRoc]-10; y[0] = anaInterface->GetIA() - current0;
    if (debug)
      psi::LogDebug() << "[TestModule] Vana " << x[0] << " Iana " << y[0]
                      << psi::endl;

    GetRoc(iRoc)->SetDAC("Vana", vana[iRoc]);
    tbInterface->Flush();
    gDelay->Mdelay(1000);
    x[1] = vana[iRoc]; y[1] = anaInterface->GetIA() - current0;
    if (debug)
      psi::LogDebug() << "[TestModule] Vana " << x[1] << " Iana " << y[1]
                      << psi::endl;

    GetRoc(iRoc)->SetDAC("Vana", vana[iRoc]+10);
    tbInterface->Flush();
    gDelay->Mdelay(1000);
    x[2] = vana[iRoc]+10; y[2] = anaInterface->GetIA() - current0;
    if (debug)
      psi::LogDebug() << "[TestModule] Vana " << x[2] << " Iana " << y[2]
                      << psi::endl;
    
    TGraph *graph = new TGraph(3, x, y);
    graph->SetName(Form("VanaIana_C%i", iRoc));
    graph->Write();
                        
    GetRoc(iRoc)->SetDAC("Vana", 0);
                tbInterface->Flush();
  }
        
  for (int iRoc = 0; iRoc < nRocs; iRoc++)
  {
    GetRoc(iRoc)->SetDAC("Vana", vana[iRoc]);
    GetRoc(iRoc)->SetDAC("Vsf", vsf[iRoc]);
  }
  tbInterface->Flush();
}


void TestModule::MeasureCurrents()
{
  TBAnalogInterface* anaInterface = (TBAnalogInterface*)tbInterface;
        
  double ia = anaInterface->GetIA();
  double va = anaInterface->GetVA();
  double id = anaInterface->GetID();
  double vd = anaInterface->GetVD();
  
  psi::LogDebug() << "[TestModule] ============== Currents and Voltages ==============" << psi::endl;
  psi::LogDebug() << "[TestModule]    > Analog" << psi::endl;
  psi::LogDebug() << "[TestModule]        I: " << ia << psi::endl;
  psi::LogDebug() << "[TestModule]        V: " << va << psi::endl;
  psi::LogDebug() << "[TestModule]    > Digital" << psi::endl;
  psi::LogDebug() << "[TestModule]        I: " << id << psi::endl;
  psi::LogDebug() << "[TestModule]        V: " << vd << psi::endl;
  psi::LogDebug() << "[TestModule] ===================================================" << psi::endl;
  
  TParameter<double> *parameter0 = new TParameter<double>("IA", ia);
  TParameter<double> *parameter1 = new TParameter<double>("VA", va);
  TParameter<double> *parameter2 = new TParameter<double>("ID", id);
  TParameter<double> *parameter3 = new TParameter<double>("VD", vd);
  
  parameter0->Write();
  parameter1->Write();
  parameter2->Write();
  parameter3->Write();
}


void TestModule::AdjustUltraBlackLevel()
{
  unsigned short count;
  short data[10000];
  TBAnalogInterface *anaInterface = (TBAnalogInterface*)tbInterface;
  
  psi::LogInfo() << "[TestModule] Adjust UltraBlack Levels." << psi::endl;

  anaInterface->ADCData(data, count);

  if (count < anaInterface->GetEmptyReadoutLengthADC()) 
  {
    psi::LogInfo() << "[TestModule] Error: no valid analog readout." << psi::endl;

    return;
  }
  
  int ultraBlackLevel = (data[0] + data[1] + data[2])/3;
  
  for (int iRoc = 0; iRoc < nRocs; iRoc++)
  {
    GetRoc(iRoc)->AdjustUltraBlackLevel(ultraBlackLevel);
  }
}


bool TestModule::TestDACProgramming(int dacReg, int max)
{
  bool debug = false, result = true;
  unsigned short count, count2;
  short data[10000], data2[10000];
  int dacValue;
  TBAnalogInterface *anaInterface = (TBAnalogInterface*)tbInterface;

  for (int iRoc = 0; iRoc < nRocs; iRoc++) 
  {
    dacValue = GetRoc(iRoc)->GetDAC(dacReg);
    GetRoc(iRoc)->SetDAC(dacReg, 0);
    anaInterface->Flush();
    anaInterface->ADCData(data, count);
    GetRoc(iRoc)->SetDAC(dacReg, max);
    anaInterface->Flush();
    anaInterface->ADCData(data2, count2);
    GetRoc(iRoc)->SetDAC(dacReg, dacValue);
    anaInterface->Flush();
  
    if (debug)
    {
      printf("count: %i\n", count);
      for (int i = 0; i < count; i++) printf("%i ", data[i]);
      printf("\n");
      printf("count2: %i\n", count2);
      for (int i = 0; i < count2; i++) printf("%i ", data2[i]);
      printf("\n");
    }
    
    if ((count != anaInterface->GetEmptyReadoutLengthADC()) || (count2 != anaInterface->GetEmptyReadoutLengthADC())) 
    {
      psi::LogInfo() << "[TestModule] Error: no valid analog readout." << psi::endl;

      return false;
    }
  
    int offset;
    if (anaInterface->TBMPresent()) offset = 10;
    else if(ConfigParameters::Singleton().TbmEmulator()) offset = 10;
    else offset = 3;
    if (TMath::Abs(data[offset+iRoc*3] - data2[offset+iRoc*3]) < 20) 
    {
      result = false;
      printf(">>>>>> Error ROC %i: DAC programming error\n", iRoc);
    }
  }
  
  if (debug && result) printf("dac %i ok\n", dacReg);
  if (debug && !result) printf(">>>>>>>> Error: dac %i not ok\n", dacReg);
  return result;
}



void TestModule::TestDACProgramming()
{
  psi::LogInfo() << "[TestModule] Test if ROC DACs are programmable." << psi::endl;

  bool result = true;
  
  result &= TestDACProgramming(25, 255);
  
  if (!result) printf(">>>>>> Error: DAC programming error\n");
}




// ----------------------------------------------------------------------
void TestModule::DumpParameters() 
{

  WriteDACParameterFile(ConfigParameters::Singleton().DacParametersFileName().c_str());
/*  cout << "Dumping all parameters" << endl;

  char line[1000];

  cout << "==>CN dumpHardwareConfiguration" << endl;
  sprintf(line, "/tmp/expert-dac");
  WriteDACParameterFile(line);
  sprintf(line, "/tmp/expert-trim");
  WriteTrimConfiguration(line);

  cout << "==>TB dumpHardwareConfiguration" << endl;
  sprintf(line, "/tmp/expert-testboard.dat");
  tbInterface->WriteTBParameterFile(line);*/
 
}


void TestModule::DataTriggerLevelScan()
{
  TBAnalogInterface* anaInterface = (TBAnalogInterface*)tbInterface;
  int dtlOrig = ConfigParameters::Singleton().DataTriggerLevel();
  
  if ((!anaInterface->DataTriggerLevelScan()) && (ConfigParameters::Singleton().HalfModule() == 0))
  {
    TBM *tbm = GetTBM();
    int channel = anaInterface->GetTBMChannel();
    int singleDual = tbm->GetDAC(0);
    
    // try for second tbm
    psi::LogInfo() << "[TestModule] Error: No valid readout for this TBM. "
                   << "Try with channel " << channel << '.' << psi::endl;
    
    SetTBMSingle((channel + 1) % 2);
    anaInterface->SetTBMChannel((channel + 1) % 2);
    
    anaInterface->DataTriggerLevelScan();
    
    anaInterface->SetTBMChannel(channel);
    tbm->SetDAC(0, singleDual);
  }
  
  anaInterface->DataTriggerLevel(dtlOrig);
}


double TestModule::GetTemperature()
{
  gDelay->Timestamp();
  double temperature = 0.;
  for (int i = 0; i < nRocs; i++)
  {
    temperature+=GetRoc(i)->GetTemperature();
  }
  temperature/=nRocs;
  psi::LogDebug() << "[TestModule] Temperature: " << temperature << " (˙C)."
                  << psi::endl;

  gDelay->Timestamp();

  return temperature;
}

void TestModule::Scurves()
{	
  
  //TestRange *testRange = new TestRange();
	//testRange->CompleteRange();
//AdjustDACParameters();
    DoTest(new FullTest(FullRange(), tbInterface,0));
 // Test *Stest = new SCurveTest(testRange, tbInterface);
 // Stest->ModuleAction();
  
}


