/*!
 * \file TestModule.cc
 * \brief Implementation of TestModule class.
 *
 * \b Changelog
 * 02-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using psi::Sleep instead interface/Delay.
 * 01-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Class SysCommand removed.
 * 26-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Removed redundant dependency from Module class.
 * 25-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - DataStorage moved into psi namespace.
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using definitions from PsiCommon.h.
 * 21-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using DataStorage class to save the results.
 * 15-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using boost::units::quantity to represent physical values.
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

#include "psi/log.h"
#include "psi/date_time.h"
#include "TestModule.h"
#include "BasePixel/TBAnalogInterface.h"
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
#include "DataStorage.h"
#include "BareTest.h"

TestModule::TestModule(int aCNId, TBInterface *aTBInterface)
{
  const ConfigParameters& configParameters = ConfigParameters::Singleton();

  if (configParameters.CustomModule() == 0)
  {
    const unsigned   nRocs = configParameters.NumberOfRocs();

    tbm = new TBM(aCNId, aTBInterface);
    tbm->init();
    hubId = configParameters.HubId();

    controlNetworkId = aCNId;
    tbInterface = aTBInterface;

    int offset = 0;
    if (configParameters.HalfModule() == 2) offset = 8;
    for (unsigned i = 0; i < nRocs; i++)
        rocs.push_back( boost::shared_ptr<TestRoc>(new TestRoc(tbInterface, i+offset, hubId, int((i+offset)/4), i)));
  }
  else if (configParameters.CustomModule() == 1)
  {
    psi::Log<psi::Info>() << "[TestModule] Custom module constructor: Ignoring nRocs, "
                   << "hubID, ... in config file." << std::endl;
    psi::Log<psi::Info>() << "[TestModule] Using: 4 ROCs with ChipID/PortID 0,1/0 "
                   << "and 8,9/2." << std::endl;
                
    const unsigned nRocs = 4;
    hubId = 0;
    controlNetworkId = aCNId;
    tbInterface = aTBInterface;

    tbm = new TBM(aCNId, tbInterface);
    tbm->init();

    //nRocs = 4;
    rocs.push_back( boost::shared_ptr<TestRoc>(new TestRoc(tbInterface, 1, hubId, 0, 0)));
    rocs.push_back( boost::shared_ptr<TestRoc>(new TestRoc(tbInterface, 0, hubId, 0, 1)));
    rocs.push_back( boost::shared_ptr<TestRoc>(new TestRoc(tbInterface, 8, hubId, 2, 2)));
    rocs.push_back( boost::shared_ptr<TestRoc>(new TestRoc(tbInterface, 9, hubId, 2, 3)));
  }
}

boost::shared_ptr<TestRoc> TestModule::GetRoc(int iRoc) {
  return rocs[iRoc];
}


//void TestModule::Execute(SysCommand &command)
//{
//  if (command.TargetIsTBM()) {tbm->Execute(command);}
//  else
//  {
//    if (strcmp(command.carg[0],"TestM") == 0) {TestM();}
//    else if (strcmp(command.carg[0],"DigiCurrent") == 0) {DigiCurrent();}
//    else if (command.Keyword("Test")) {DoTest(new FullTest(GetRange(command), tbInterface,1));}
//    else if (command.Keyword("FullTest")) {DoTest(new FullTest(GetRange(command), tbInterface,1));}
//    else if (!strcmp(command.carg[0],"BareTest")) {DoTest(new BareTest(GetRange(command), (TBAnalogInterface*)tbInterface,command.carg[1]));}
//    else if (command.Keyword("xray")) {DoTest(new Xray(GetRange(command), tbInterface));}
//    else if (command.Keyword("FullTestAndCalibration")) {FullTestAndCalibration();}
//    else if (command.Keyword("DumpParameters")) {DumpParameters();}
//    else if (command.Keyword("Temp")) {GetTemperature();}
//    else if (command.Keyword("adc")) {((TBAnalogInterface*)tbInterface)->ADC();}
//	else if (command.Keyword("adc_fix")) {((TBAnalogInterface*)tbInterface)->ADC(200);}//
//    else if (command.Keyword("dtlScan")) {DataTriggerLevelScan();}
//    else if (strcmp(command.carg[0],"adjustUB") == 0) AdjustUltraBlackLevel();
//    else if (strcmp(command.carg[0],"adjustDTL") == 0) AdjustDTL();
//    else if (strcmp(command.carg[0],"adjustSmp") == 0) AdjustSamplingPoint();
//    else if (strcmp(command.carg[0],"adjust") == 0) AdjustDACParameters();
//    else if (strcmp(command.carg[0],"Pretest") == 0) AdjustDACParameters();
//                else if (strcmp(command.carg[0],"NewPretest") == 0) AdjustAllDACParameters();
//    else if (command.Keyword("TBMTest")) {DoTBMTest();}
//    else if (command.Keyword("AnaReadout")) {AnaReadout();}
//    else if (command.Keyword("DACProgramming")) {TestDACProgramming();}
// 		else if (command.Keyword("Scurves")) {Scurves();}
//    else
//    {
//      bool done = false;
//      for (unsigned i = 0; i < rocs.size(); i++)
//      {
//        if (rocs[i]->GetChipId() == command.roc)
//        {
//          GetRoc(i)->Execute(command);
//          done = true;
//        }
//      }
//      if (!done)
//        psi::Log<psi::Info>() << "[TestModule] Error: Roc #" << command.roc
//                       << " does not exist." << std::endl;

//      return;
//    }
//    command.RocsDone();
//  }
  
//}

void TestModule::FullTestAndCalibration()
{
  AdjustDACParameters();
    DoTest(new FullTest(FullRange(), tbInterface,1));

  psi::Log<psi::Info>() << "[TestModule] PhCalibration: Start." << std::endl;

  for (unsigned i = 0; i < rocs.size(); i++) GetRoc(i)->DoPhCalibration();

  psi::Log<psi::Info>() << "[TestModule] PhCalibration: End." << std::endl;

  psi::Log<psi::Info>() << "[TestModule] Trim: Start." << std::endl;

  for (unsigned i = 0; i < rocs.size(); i++) GetRoc(i)->DoTrim();

  psi::Log<psi::Info>() << "[TestModule] Trim: End." << std::endl;
}


void TestModule::ShortCalibration()
{
  AdjustAllDACParameters();        
  VanaVariation();
  DoTest(new PixelAlive(FullRange(), tbInterface));

  psi::Log<psi::Info>() << "[TestModule] PhCalibration: Start." << std::endl;

  for (unsigned i = 0; i < rocs.size(); i++) GetRoc(i)->DoPhCalibration();

  psi::Log<psi::Info>() << "[TestModule] PhCalibration: End." << std::endl;
}


void TestModule::ShortTestAndCalibration()
{
  AdjustAllDACParameters();        
  VanaVariation();
  DoTest(new PixelAlive(FullRange(), tbInterface));
  DoTest(new BumpBonding(FullRange(), tbInterface));

  psi::Log<psi::Info>() << "[TestModule] PhCalibration: Start." << std::endl;

  for (unsigned i = 0; i < rocs.size(); i++) GetRoc(i)->DoPhCalibration();

  psi::Log<psi::Info>() << "[TestModule] PhCalibration: End." << std::endl;
  
  psi::Log<psi::Info>() << "[TestModule] Trim: Start." << std::endl;

  TrimLow *trimLow = new TrimLow(FullRange(), tbInterface);
  trimLow->SetVcal(40);
  //trimLow->NoTrimBits(true);
  DoTest(trimLow);

  psi::Log<psi::Info>() << "[TestModule] Trim: End." << std::endl;
}


void TestModule::DoTest(Test *aTest)
{
  aTest->ModuleAction(this);
}


//TestRange *TestModule::GetRange(SysCommand &command)
//{
//  TestRange *range = new TestRange();
//  for (unsigned i = 0; i < rocs.size(); i++)
//  {
//    if (command.IncludesRoc(GetRoc(i)->GetChipId())) range->CompleteRoc(GetRoc(i)->GetChipId());
//  }
//  return range;
//}


TestRange *TestModule::FullRange()
{
  TestRange *range = new TestRange();
  for (unsigned i = 0; i < rocs.size(); i++) range->CompleteRoc(GetRoc(i)->GetChipId());
  return range;
}


void TestModule::DoTBMTest()
{
  Test *aTest = new TBMTest(new TestRange(), tbInterface);
  psi::Log<psi::Info>().PrintTimestamp();
  aTest->ModuleAction(this);
  psi::Log<psi::Info>().PrintTimestamp();
}


void TestModule::AnaReadout()
{
  Test *aTest = new AnalogReadout(new TestRange(), tbInterface);
  psi::Log<psi::Info>().PrintTimestamp();
  aTest->ModuleAction(this);
  psi::Log<psi::Info>().PrintTimestamp();
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
  //  for(int dacRegister = 1; dacRegister < 28; dacRegister++)
  for(int dacRegister = 3; dacRegister < 4; dacRegister++)
    {
      DACParameters* parameters = new DACParameters();
      const char *dacName = parameters->GetName(dacRegister);
      
//      nRocs = 16;
      for (unsigned iRoc = 0; iRoc < rocs.size(); iRoc++)
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
        const psi::ElectricCurrent dc = ((TBAnalogInterface*)tbInterface)->GetID();
        cout << "Digital current: " << dc << endl;
        currentHist->SetBinContent((dacValue/10)+1,psi::DataStorage::ToStorageUnits(dc));
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
      psi::Log<psi::Debug>() << "[TestModule] Delay " << delay << '.' << std::endl;
    
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
  tbInterface->WriteTBParameterFile(configParameters.FullTbParametersFileName().c_str());
}


void TestModule::AdjustAllDACParameters()
{
  bool tbmPresent = ((TBAnalogInterface*)tbInterface)->TBMPresent();

  psi::Log<psi::Info>() << "[TestModule] Pretest Extended: Start." << std::endl;

  tbInterface->Single(0);
  psi::Log<psi::Info>().PrintTimestamp();
  const ConfigParameters& configParameters = ConfigParameters::Singleton();
  if (tbmPresent)
  {
    ((TBAnalogInterface*)tbInterface)->SetTriggerMode(TRIGGER_MODULE1); // trigger mode 2 only works correctly after adjusting tbm and roc ultrablacks to the same level
      if(!configParameters.TbmEmulator())  AdjustTBMUltraBlack();
    AdjustDTL();
  }
  TestDACProgramming();
  AdjustVana(0.024 * psi::amperes);
  psi::Log<psi::Info>().PrintTimestamp();
  if (tbmPresent) 
  {
    AdjustUltraBlackLevel();
    ((TBAnalogInterface*)tbInterface)->SetTriggerMode(TRIGGER_MODULE2);
    AdjustSamplingPoint();
  }
  psi::Log<psi::Info>().PrintTimestamp();
//        AdjustCalDelVthrComp();
//        DoTest(new TimeWalkStudy(FullRange(), tbInterface));
  AdjustCalDelVthrComp();
        
  AdjustPHRange();
  DoTest(new VsfOptimization(FullRange(), tbInterface));

  psi::Log<psi::Info>().PrintTimestamp();
  MeasureCurrents();
  WriteDACParameterFile(configParameters.FullDacParametersFileName().c_str());
  CalibrateDecoder();
  ADCHisto();

  psi::Log<psi::Info>() << "[TestModule] Pretest Extended: End." << std::endl;
}


void TestModule::AdjustDACParameters()
{
  bool tbmPresent = dynamic_cast<TBAnalogInterface *>( tbInterface)->TBMPresent();

  psi::Log<psi::Info>() << "[TestModule] Pretest: Start." << std::endl;

  tbInterface->Single(0);
  psi::Log<psi::Info>().PrintTimestamp();

  const ConfigParameters& configParameters = ConfigParameters::Singleton();

  if (tbmPresent)
  {
    ((TBAnalogInterface*)tbInterface)->SetTriggerMode(TRIGGER_MODULE1); // trigger mode 2 only works correctly after adjusting tbm and roc ultrablacks to the same level
      if(!configParameters.TbmEmulator())   AdjustTBMUltraBlack();
    AdjustDTL();
  }
  TestDACProgramming();
  AdjustVana(0.024 * psi::amperes);
  MeasureCurrents();
  if (tbmPresent) 
  {
    AdjustUltraBlackLevel();
    ((TBAnalogInterface*)tbInterface)->SetTriggerMode(TRIGGER_MODULE2);
    AdjustSamplingPoint();
  }
  for (unsigned iRoc = 0; iRoc < rocs.size(); iRoc++)
  { 
    psi::Log<psi::Debug>() << "[TestModule] Roc #" << GetRoc(iRoc)->GetChipId()
                    << '.' << std::endl;

    GetRoc(iRoc)->AdjustCalDelVthrComp();
  }
  AdjustVOffsetOp();

  psi::Log<psi::Info>().PrintTimestamp();
  WriteDACParameterFile(configParameters.FullDacParametersFileName().c_str());
  CalibrateDecoder();
  ADCHisto();

  psi::Log<psi::Info>() << "[TestModule] Pretest: End." << std::endl;
}


void TestModule::AdjustCalDelVthrComp()
{
  for (unsigned iRoc = 0; iRoc < rocs.size(); iRoc++)
  { 
    psi::Log<psi::Debug>() << "[TestModule] Roc #" << GetRoc(iRoc)->GetChipId()
                    << '.' << std::endl;

    GetRoc(iRoc)->AdjustCalDelVthrComp();
  }
}


void TestModule::ADCHisto()
{
  unsigned short count;
  short data[psi::FIFOSIZE];
  ((TBAnalogInterface*)tbInterface)->ADCRead(data, count, 100);
  TH1D *hist = new TH1D("ADC","ADC", count, 0., count);
  for (unsigned int n = 0; n < count; n++) hist->SetBinContent(n+1,data[n]);
}


void TestModule::AdjustVOffsetOp()
{
  psi::Log<psi::Info>() << "[TestModule] Adjust VOffsetOp." << std::endl;

  psi::Log<psi::Info>().PrintTimestamp();
  
  TestRange *testRange = new TestRange();
  testRange->CompleteRange();
  Test *aTest = new UbCheck(testRange, tbInterface);
  aTest->ModuleAction(this);
}


void TestModule::AdjustPHRange()
{
  psi::Log<psi::Info>() << "[TestModule] Adjust PhRange." << std::endl;

  psi::Log<psi::Info>().PrintTimestamp();
  
  TestRange *testRange = new TestRange();
  testRange->CompleteRange();
  Test *aTest = new PHRange(testRange, tbInterface);
  aTest->ModuleAction(this);
}


void TestModule::CalibrateDecoder()
{
  psi::Log<psi::Info>() << "[TestModule] Calibrate Decoder (AddressLevels test)."
                 << std::endl;

  psi::Log<psi::Info>().PrintTimestamp();
  
  TestRange *testRange = new TestRange();
  testRange->CompleteRange();
  Test *aTest = new AddressLevels(testRange, tbInterface);
  aTest->ModuleAction(this);
}


void TestModule::AdjustTBMUltraBlack()
{
  psi::Log<psi::Info>().PrintTimestamp();
  
  TestRange *testRange = new TestRange();
  testRange->CompleteRange();
  Test *aTest = new TBMUbCheck(testRange, tbInterface);
  aTest->ModuleAction(this);
}


// Tries to automatically adjust Vana, may not work yet
void TestModule::AdjustVana(psi::ElectricCurrent goalCurrent)
{
  if (!tbInterface->IsAnalogTB()) return;
  TBAnalogInterface* anaInterface = (TBAnalogInterface*)tbInterface;
  int vana[rocs.size()];
  int vsf[rocs.size()];

  for (unsigned iRoc = 0; iRoc < rocs.size(); iRoc++)
  {
    vsf[iRoc] = GetRoc(iRoc)->GetDAC("Vsf");

    GetRoc(iRoc)->SetDAC("Vana", 0);
    GetRoc(iRoc)->SetDAC("Vsf", 0);
  }
  tbInterface->Flush();
  psi::Sleep(0.5 * psi::seconds);
  const psi::ElectricCurrent current0 = anaInterface->GetIA();

  psi::Log<psi::Debug>() << "[TestModule] ZeroCurrent " << current0 << std::endl;

  for (unsigned iRoc = 0; iRoc < rocs.size(); iRoc++)
  {
    vana[iRoc] = GetRoc(iRoc)->AdjustVana(current0, goalCurrent);
    GetRoc(iRoc)->SetDAC("Vana", 0);
  }
  for (unsigned iRoc = 0; iRoc < rocs.size(); iRoc++)
  {
    GetRoc(iRoc)->SetDAC("Vana", vana[iRoc]);
    GetRoc(iRoc)->SetDAC("Vsf", vsf[iRoc]);
  }
  tbInterface->Flush();
  psi::Sleep(0.5 * psi::seconds);
  psi::ElectricCurrent current = anaInterface->GetIA();

  psi::Log<psi::Debug>() << "[TestModule] TotalCurrent " << current << std::endl;
}



void TestModule::VanaVariation()
{
  bool debug = true;

  psi::Log<psi::Info>() << "[TestModule] VanaVariation." << std::endl;

  TBAnalogInterface* anaInterface = (TBAnalogInterface*)tbInterface;
        int vsf[rocs.size()], vana[rocs.size()];
        double x[3], y[3];
        
  for (unsigned iRoc = 0; iRoc < rocs.size(); iRoc++)
  {
    vsf[iRoc] = GetRoc(iRoc)->GetDAC("Vsf");
          vana[iRoc] = GetRoc(iRoc)->GetDAC("Vana");

    GetRoc(iRoc)->SetDAC("Vana", 0);
    GetRoc(iRoc)->SetDAC("Vsf", 0);
  }
  tbInterface->Flush();
  psi::Sleep(2.0 * psi::seconds);
        
  const psi::ElectricCurrent current0 = anaInterface->GetIA();
  psi::Log<psi::Debug>() << "[TestModule] ZeroCurrent " << current0 << std::endl;
        
  for (unsigned iRoc = 0; iRoc < rocs.size(); iRoc++)
  {
    if (debug)
      psi::Log<psi::Debug>() << "[TestModule] Roc #" << iRoc << '.' << std::endl;
        
    GetRoc(iRoc)->SetDAC("Vana", vana[iRoc]-10);
    tbInterface->Flush();
    psi::Sleep(1.0 * psi::seconds);
    x[0] = vana[iRoc]-10; y[0] = psi::DataStorage::ToStorageUnits(anaInterface->GetIA() - current0);
    if (debug)
      psi::Log<psi::Debug>() << "[TestModule] Vana " << x[0] << " Iana " << y[0]
                      << std::endl;

    GetRoc(iRoc)->SetDAC("Vana", vana[iRoc]);
    tbInterface->Flush();
    psi::Sleep(1.0 * psi::seconds);
    x[1] = vana[iRoc]; y[1] = psi::DataStorage::ToStorageUnits(anaInterface->GetIA() - current0);
    if (debug)
      psi::Log<psi::Debug>() << "[TestModule] Vana " << x[1] << " Iana " << y[1]
                      << std::endl;

    GetRoc(iRoc)->SetDAC("Vana", vana[iRoc]+10);
    tbInterface->Flush();
    psi::Sleep(1.0 * psi::seconds);
    x[2] = vana[iRoc]+10; y[2] = psi::DataStorage::ToStorageUnits(anaInterface->GetIA() - current0);
    if (debug)
      psi::Log<psi::Debug>() << "[TestModule] Vana " << x[2] << " Iana " << y[2]
                      << std::endl;
    
    TGraph *graph = new TGraph(3, x, y);
    graph->SetName(Form("VanaIana_C%i", iRoc));
    graph->Write();
                        
    GetRoc(iRoc)->SetDAC("Vana", 0);
                tbInterface->Flush();
  }
        
  for (unsigned iRoc = 0; iRoc < rocs.size(); iRoc++)
  {
    GetRoc(iRoc)->SetDAC("Vana", vana[iRoc]);
    GetRoc(iRoc)->SetDAC("Vsf", vsf[iRoc]);
  }
  tbInterface->Flush();
}


void TestModule::MeasureCurrents()
{
  TBAnalogInterface* anaInterface = (TBAnalogInterface*)tbInterface;
        
  psi::ElectricCurrent ia = anaInterface->GetIA();
  psi::ElectricPotential va = anaInterface->GetVA();
  psi::ElectricCurrent id = anaInterface->GetID();
  psi::ElectricPotential vd = anaInterface->GetVD();
  
  psi::Log<psi::Debug>() << "[TestModule] ============== Currents and Voltages ==============" << std::endl;
  psi::Log<psi::Debug>() << "[TestModule]    > Analog" << std::endl;
  psi::Log<psi::Debug>() << "[TestModule]        I: " << ia << std::endl;
  psi::Log<psi::Debug>() << "[TestModule]        V: " << va << std::endl;
  psi::Log<psi::Debug>() << "[TestModule]    > Digital" << std::endl;
  psi::Log<psi::Debug>() << "[TestModule]        I: " << id << std::endl;
  psi::Log<psi::Debug>() << "[TestModule]        V: " << vd << std::endl;
  psi::Log<psi::Debug>() << "[TestModule] ===================================================" << std::endl;
  
  psi::DataStorage::Active().SaveMeasurement("IA", ia);
  psi::DataStorage::Active().SaveMeasurement("VA", va);
  psi::DataStorage::Active().SaveMeasurement("ID", id);
  psi::DataStorage::Active().SaveMeasurement("VD", vd);
}


void TestModule::AdjustUltraBlackLevel()
{
  unsigned short count;
  short data[10000];
  TBAnalogInterface *anaInterface = (TBAnalogInterface*)tbInterface;
  
  psi::Log<psi::Info>() << "[TestModule] Adjust UltraBlack Levels." << std::endl;

  anaInterface->ADCData(data, count);

  if (count < anaInterface->GetEmptyReadoutLengthADC()) 
  {
    psi::Log<psi::Info>() << "[TestModule] Error: no valid analog readout." << std::endl;

    return;
  }
  
  int ultraBlackLevel = (data[0] + data[1] + data[2])/3;
  
  for (unsigned iRoc = 0; iRoc < rocs.size(); iRoc++)
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

  for (unsigned iRoc = 0; iRoc < rocs.size(); iRoc++)
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
      psi::Log<psi::Info>() << "[TestModule] Error: no valid analog readout." << std::endl;

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
  psi::Log<psi::Info>() << "[TestModule] Test if ROC DACs are programmable." << std::endl;

  bool result = true;
  
  result &= TestDACProgramming(25, 255);
  
  if (!result) printf(">>>>>> Error: DAC programming error\n");
}




// ----------------------------------------------------------------------
void TestModule::DumpParameters() 
{

  WriteDACParameterFile(ConfigParameters::Singleton().FullDacParametersFileName().c_str());
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
    psi::Log<psi::Info>() << "[TestModule] Error: No valid readout for this TBM. "
                   << "Try with channel " << channel << '.' << std::endl;
    
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
  psi::Log<psi::Info>().PrintTimestamp();
  double temperature = 0.;
  for (unsigned i = 0; i < rocs.size(); i++)
  {
    temperature+=GetRoc(i)->GetTemperature();
  }
  temperature/=rocs.size();
  psi::Log<psi::Debug>() << "[TestModule] Temperature: " << temperature << " (˙C)."
                  << std::endl;

  psi::Log<psi::Info>().PrintTimestamp();

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

unsigned TestModule::NRocs()
{
  return rocs.size();
}

int TestModule::GetTBM(int reg)
{
  return tbm->GetDAC(reg);
}

void TestModule::SetTBM(int chipId, int reg, int value)
{
  tbm->SetDAC(reg,value);
}

TBM* TestModule::GetTBM()
{
  return tbm;
}

void TestModule::SetTBMSingle(int tbmChannel)
{
  if (tbmChannel == 0) tbm->SetDAC(0, 0);
  else tbm->SetDAC(0, 2);
}

void TestModule::AdjustDTL()
{
  TBAnalogInterface *anaInterface = (TBAnalogInterface*)tbInterface;
  int dtl = 0, emptyReadoutLength = anaInterface->GetEmptyReadoutLengthADC();
  short data[10000];
  unsigned short count;

  do
  {
    dtl-=50;
    anaInterface->DataTriggerLevel(dtl);
    anaInterface->ADCData(data, count);
  }
  while ((count != emptyReadoutLength) && (dtl > -2000));

  if (dtl == -2000)
  {
    // try with second tbm
    TBM *tbm = GetTBM();
    int channel = anaInterface->GetTBMChannel();
    dtl = 0;

    psi::Log<psi::Info>() << "[Module] Problem: Can not find data trigger level. "
                   << "Try different channel." << std::endl;

    SetTBMSingle((channel + 1) % 2);
    anaInterface->SetTBMChannel((channel + 1) % 2);

    do
    {
      dtl-=50;
      anaInterface->DataTriggerLevel(dtl);
      anaInterface->ADCData(data, count);
    }
    while ((count != emptyReadoutLength) && (dtl > -2000));

    if (dtl != -2000)
        tbm->WriteTBMParameterFile(ConfigParameters::Singleton().FullTbmParametersFileName().c_str());
  }

  if (dtl == -2000)
  {
          // still no valid readout
    psi::Log<psi::Info>() << "[Module] Problem: Can not find data trigger level."
                   << std::endl;
    return;
  }

  dtl = (data[0] + data[1] + data[2]) / 3 + 100;

  anaInterface->DataTriggerLevel(dtl);
  if (dtl < -1200)
    psi::Log<psi::Info>() << "[Module] Warning: Very low data trigger level: "
                   << dtl << ". Check AOUT channels." << std::endl;

  ConfigParameters::ModifiableSingleton().setDataTriggerLevel(dtl);
  psi::Log<psi::Info>() << "[Module] Setting data trigger level to "
                 << dtl << std::endl;
  ConfigParameters::Singleton().WriteConfigParameterFile();
}

void TestModule::Initialize()
{
  tbm->Initialize(ConfigParameters::Singleton().FullTbmParametersFileName().c_str());

  for (unsigned i = 0; i < rocs.size(); i++)
  {
    rocs[i]->Initialize();
  }
}

// -- Write the current DAC settings of all rocs to disk
void TestModule::WriteDACParameterFile( const char* filename)
{
  for (unsigned i = 0; i < rocs.size(); i++) rocs[i]->WriteDACParameterFile(filename);
}
