/*!
 * \file TestModule.cc
 * \brief Implementation of TestModule class.
 *
 * \b Changelog
 * 13-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Using TBAnalogInterface instead TBInterface.
 *      - TBMParameters class now inherit psi::BaseConifg class.
 * 09-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Corrected questionable language constructions, which was found using -Wall g++ option.
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

#include "psi/log.h"
#include "psi/date_time.h"

#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/TestRange.h"
#include "BasePixel/ThresholdMap.h"
#include "BasePixel/DataStorage.h"

#include "tests/SCurveTest.h"
#include "tests/FullTest.h"
#include "tests/PHCalibration.h"
#include "tests/IVCurve.h"
#include "tests/UbCheck.h"
#include "tests/TBMUbCheck.h"
#include "tests/TBMTest.h"
#include "tests/AnalogReadout.h"
#include "tests/AddressLevels.h"
#include "tests/VsfOptimization.h"
#include "tests/TimeWalkStudy.h"
#include "tests/PixelAlive.h"
#include "tests/BumpBonding.h"
#include "tests/TrimLow.h"
#include "tests/PHRange.h"
#include "tests/Xray.h"
#include "tests/BareTest.h"

#include "TestModule.h"

TestModule::TestModule(int aCNId, boost::shared_ptr<TBAnalogInterface> aTBInterface)
    : controlNetworkId(aCNId), tbInterface(aTBInterface)
{
    const ConfigParameters& configParameters = ConfigParameters::Singleton();

    if (configParameters.CustomModule() == 0)
    {
        const unsigned   nRocs = configParameters.NumberOfRocs();

        tbm = new TBM(aCNId, tbInterface);
        tbm->init();
        hubId = configParameters.HubId();

        int offset = 0;
        if (configParameters.HalfModule() == 2) offset = 8;
        for (unsigned i = 0; i < nRocs; i++)
            rocs.push_back( boost::shared_ptr<TestRoc>(new TestRoc(tbInterface, i + offset, hubId, int((i + offset) / 4), i)));
    }
    else if (configParameters.CustomModule() == 1)
    {
        psi::LogInfo() << "[TestModule] Custom module constructor: Ignoring nRocs, "
                       << "hubID, ... in config file." << std::endl;
        psi::LogInfo() << "[TestModule] Using: 4 ROCs with ChipID/PortID 0,1/0 "
                       << "and 8,9/2." << std::endl;

        hubId = 0;

        tbm = new TBM(aCNId, tbInterface);
        tbm->init();

        //nRocs = 4;
        rocs.push_back( boost::shared_ptr<TestRoc>(new TestRoc(tbInterface, 1, hubId, 0, 0)));
        rocs.push_back( boost::shared_ptr<TestRoc>(new TestRoc(tbInterface, 0, hubId, 0, 1)));
        rocs.push_back( boost::shared_ptr<TestRoc>(new TestRoc(tbInterface, 8, hubId, 2, 2)));
        rocs.push_back( boost::shared_ptr<TestRoc>(new TestRoc(tbInterface, 9, hubId, 2, 3)));
    }
}

boost::shared_ptr<TestRoc> TestModule::GetRoc(int iRoc)
{
    return rocs[iRoc];
}

void TestModule::FullTestAndCalibration()
{
    AdjustDACParameters();
    DoTest(new FullTest(FullRange(), tbInterface.get(), 1));

    psi::LogInfo() << "[TestModule] PhCalibration: Start." << std::endl;

    for (unsigned i = 0; i < rocs.size(); i++) GetRoc(i)->DoPhCalibration();

    psi::LogInfo() << "[TestModule] PhCalibration: End." << std::endl;

    psi::LogInfo() << "[TestModule] Trim: Start." << std::endl;

    for (unsigned i = 0; i < rocs.size(); i++) GetRoc(i)->DoTrim();

    psi::LogInfo() << "[TestModule] Trim: End." << std::endl;
}


void TestModule::ShortCalibration()
{
    AdjustAllDACParameters();
    VanaVariation();
    DoTest(new PixelAlive(FullRange(), tbInterface.get()));

    psi::LogInfo() << "[TestModule] PhCalibration: Start." << std::endl;

    for (unsigned i = 0; i < rocs.size(); i++) GetRoc(i)->DoPhCalibration();

    psi::LogInfo() << "[TestModule] PhCalibration: End." << std::endl;
}


void TestModule::ShortTestAndCalibration()
{
    AdjustAllDACParameters();
    VanaVariation();
    DoTest(new PixelAlive(FullRange(), tbInterface.get()));
    DoTest(new BumpBonding(FullRange(), tbInterface.get()));

    psi::LogInfo() << "[TestModule] PhCalibration: Start." << std::endl;

    for (unsigned i = 0; i < rocs.size(); i++) GetRoc(i)->DoPhCalibration();

    psi::LogInfo() << "[TestModule] PhCalibration: End." << std::endl;

    psi::LogInfo() << "[TestModule] Trim: Start." << std::endl;

    TrimLow *trimLow = new TrimLow(FullRange(), tbInterface.get());
    trimLow->SetVcal(40);
    //trimLow->NoTrimBits(true);
    DoTest(trimLow);

    psi::LogInfo() << "[TestModule] Trim: End." << std::endl;
}


void TestModule::DoTest(Test *aTest)
{
    aTest->ModuleAction(this);
}

TestRange *TestModule::FullRange()
{
    TestRange *range = new TestRange();
    for (unsigned i = 0; i < rocs.size(); i++) range->CompleteRoc(GetRoc(i)->GetChipId());
    return range;
}


void TestModule::DoTBMTest()
{
    Test *aTest = new TBMTest(new TestRange(), tbInterface.get());
    psi::LogInfo().PrintTimestamp();
    aTest->ModuleAction(this);
    psi::LogInfo().PrintTimestamp();
}


void TestModule::AnaReadout()
{
    Test *aTest = new AnalogReadout(new TestRange(), tbInterface.get());
    psi::LogInfo().PrintTimestamp();
    aTest->ModuleAction(this);
    psi::LogInfo().PrintTimestamp();
}


// -- Function where you can quickly implement some tests
// -- This code is not persistent
void TestModule::TestM()
{
    VanaVariation();
}

void TestModule::DigiCurrent()
{
    //  for(int dacRegister = 1; dacRegister < 28; dacRegister++)
    for(int dacRegister = 3; dacRegister < 4; dacRegister++)
    {
        DACParameters* parameters = new DACParameters();
        const char *dacName = parameters->GetName(dacRegister);

        for (unsigned iRoc = 0; iRoc < rocs.size(); iRoc++)
        {

            TH1D *currentHist = new TH1D(Form("currentHist%i_ROC%i", dacRegister, iRoc), Form("%s", dacName), 26, 0, 260);

            psi::LogInfo() << "Testing ROC " << iRoc << std::endl;

            GetRoc(iRoc)->SaveDacParameters();
            for(int dacValue = 0; dacValue < 260; dacValue += 10)
            {
                GetRoc(iRoc)->SetDAC(dacRegister, dacValue);

                psi::LogInfo() << dacName << " set to " << dacValue << std::endl;
                tbInterface->Flush();
                sleep(2);
                //dc = ((TBAnalogInterface*)tbInterface)->GetIA();
                const psi::ElectricCurrent dc = tbInterface->GetID();
                psi::LogInfo() << "Digital current: " << dc << std::endl;
                currentHist->SetBinContent((dacValue / 10) + 1, psi::DataStorage::ToStorageUnits(dc));
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

    if (debug)
        psi::LogInfo() << "clk " << clk << std::endl;
    const ConfigParameters& configParameters = ConfigParameters::Singleton();

    for (int delay = 0; delay < 25; ++delay)
    {
        if (debug)
            psi::LogDebug() << "[TestModule] Delay " << delay << '.' << std::endl;

        tbInterface->SetTBParameter("clk", (clk + delay) );
        tbInterface->SetTBParameter("sda", (sda + delay) );
        tbInterface->SetTBParameter("ctr", (ctr + delay) );
        tbInterface->SetTBParameter("tin", (tin + delay) );
        if(configParameters.TbmEmulator()) tbInterface->SetTBParameter("rda", (100 - (tin + delay)) ); // ask chris to be true with the tbmemulator


        tbInterface->Flush();

        int nTrig = 10;

        GetRoc(0)->ArmPixel(5, 5); //pixel must not be enabled during setting of the tb parameters above
        tbInterface->Flush();

        tbInterface->ADCRead(data, count, nTrig);

        GetRoc(0)->DisarmPixel(5, 5);
        tbInterface->Flush();

        if (count > 16) ph[delay] = data[16];
        else ph[delay] = -9999;

        if (debug)
        {

            psi::LogInfo() << "count " << count << std::endl;
            fprintf(file, "%+.3i: ", clk + delay);
            for (int i = 0; i < count; i++) fprintf(file, "%+.3i ", data[i]);
            fprintf(file, "\n");
        }
    }
    if (debug) fclose(file);

    GetRoc(0)->SetDAC("CtrlReg", ctrlReg);
    GetRoc(0)->SetDAC("Vcal", vcal);
    tbInterface->Flush();

    short maxPH = -9999;
    int maxDelay = 0, lowerDelay, upperDelay;
    for (int i = 0; i < 25; i++)
    {
        if (debug)
            psi::LogInfo() << "ph " << ph[i] << std::endl;
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

    if (debug)
        psi::LogInfo() <<  "maxPH " << maxPH << " max " << maxDelay << " lower " << lowerDelay
                       << " upper " << upperDelay << std::endl;

    int diff, bestDelay;
    if (upperDelay > lowerDelay) diff = upperDelay - lowerDelay;
    else diff = 25 - (lowerDelay - upperDelay);
    if (debug)
        psi::LogInfo() << "diff " << diff << std::endl;
    bestDelay = lowerDelay + diff / 2;
    if (bestDelay > 24) bestDelay -= 25;

    psi::LogInfo() << "Setting sampling point to " << (clk + bestDelay) << std::endl;

    int bestRda = rda - bestDelay;  //opposite direction than other delays
    if (bestRda < 0) bestRda += 25;

    tbInterface->SetTBParameter("clk", (clk + bestDelay));
    tbInterface->SetTBParameter("sda", (sda + bestDelay));
    tbInterface->SetTBParameter("ctr", (ctr + bestDelay));
    tbInterface->SetTBParameter("tin", (tin + bestDelay));
    tbInterface->SetTBParameter("rda", bestRda);
    if(configParameters.TbmEmulator())
    {
        psi::LogInfo() << "I'm in the tbmEmulator" << std::endl;
        tbInterface->SetTBParameter("rda", (100 - (tin + bestDelay)) ); // ask chris
    }


    tbInterface->Flush();
    tbInterface->WriteTBParameterFile(configParameters.FullTbParametersFileName().c_str());
}


void TestModule::AdjustAllDACParameters()
{
    bool tbmPresent = tbInterface->TBMPresent();

    psi::LogInfo() << "[TestModule] Pretest Extended: Start." << std::endl;

    tbInterface->Single(0);
    psi::LogInfo().PrintTimestamp();
    const ConfigParameters& configParameters = ConfigParameters::Singleton();
    if (tbmPresent)
    {
        tbInterface->SetTriggerMode(TRIGGER_MODULE1); // trigger mode 2 only works correctly after adjusting tbm and roc ultrablacks to the same level
        if(!configParameters.TbmEmulator())  AdjustTBMUltraBlack();
        AdjustDTL();
    }
    TestDACProgramming();
    AdjustVana(0.024 * psi::amperes);
    psi::LogInfo().PrintTimestamp();
    if (tbmPresent)
    {
        AdjustUltraBlackLevel();
        tbInterface->SetTriggerMode(TRIGGER_MODULE2);
        AdjustSamplingPoint();
    }
    psi::LogInfo().PrintTimestamp();
//        AdjustCalDelVthrComp();
//        DoTest(new TimeWalkStudy(FullRange(), tbInterface));
    AdjustCalDelVthrComp();

    AdjustPHRange();
    DoTest(new VsfOptimization(FullRange(), tbInterface.get()));

    psi::LogInfo().PrintTimestamp();
    MeasureCurrents();
    WriteDACParameterFile(configParameters.FullDacParametersFileName().c_str());
    CalibrateDecoder();
    ADCHisto();

    psi::LogInfo() << "[TestModule] Pretest Extended: End." << std::endl;
}


void TestModule::AdjustDACParameters()
{
    bool tbmPresent = tbInterface->TBMPresent();

    psi::LogInfo() << "[TestModule] Pretest: Start." << std::endl;

    tbInterface->Single(0);
    psi::LogInfo().PrintTimestamp();

    const ConfigParameters& configParameters = ConfigParameters::Singleton();

    if (tbmPresent)
    {
        tbInterface->SetTriggerMode(TRIGGER_MODULE1); // trigger mode 2 only works correctly after adjusting tbm and roc ultrablacks to the same level
        if(!configParameters.TbmEmulator())   AdjustTBMUltraBlack();
        AdjustDTL();
    }
    TestDACProgramming();
    AdjustVana(0.024 * psi::amperes);
    MeasureCurrents();
    if (tbmPresent)
    {
        AdjustUltraBlackLevel();
        tbInterface->SetTriggerMode(TRIGGER_MODULE2);
        AdjustSamplingPoint();
    }
    for (unsigned iRoc = 0; iRoc < rocs.size(); iRoc++)
    {
        psi::LogDebug() << "[TestModule] Roc #" << GetRoc(iRoc)->GetChipId()
                        << '.' << std::endl;

        GetRoc(iRoc)->AdjustCalDelVthrComp();
    }
    AdjustVOffsetOp();

    psi::LogInfo().PrintTimestamp();
    WriteDACParameterFile(configParameters.FullDacParametersFileName().c_str());
    CalibrateDecoder();
    ADCHisto();

    psi::LogInfo() << "[TestModule] Pretest: End." << std::endl;
}


void TestModule::AdjustCalDelVthrComp()
{
    for (unsigned iRoc = 0; iRoc < rocs.size(); iRoc++)
    {
        psi::LogDebug() << "[TestModule] Roc #" << GetRoc(iRoc)->GetChipId()
                        << '.' << std::endl;

        GetRoc(iRoc)->AdjustCalDelVthrComp();
    }
}


void TestModule::ADCHisto()
{
    unsigned short count;
    short data[psi::FIFOSIZE];
    tbInterface->ADCRead(data, count, 100);
    TH1D *hist = new TH1D("ADC", "ADC", count, 0., count);
    for (unsigned int n = 0; n < count; n++) hist->SetBinContent(n + 1, data[n]);
}


void TestModule::AdjustVOffsetOp()
{
    psi::LogInfo() << "[TestModule] Adjust VOffsetOp." << std::endl;

    psi::LogInfo().PrintTimestamp();

    TestRange *testRange = new TestRange();
    testRange->CompleteRange();
    Test *aTest = new UbCheck(testRange, tbInterface.get());
    aTest->ModuleAction(this);
}


void TestModule::AdjustPHRange()
{
    psi::LogInfo() << "[TestModule] Adjust PhRange." << std::endl;

    psi::LogInfo().PrintTimestamp();

    TestRange *testRange = new TestRange();
    testRange->CompleteRange();
    Test *aTest = new PHRange(testRange, tbInterface.get());
    aTest->ModuleAction(this);
}


void TestModule::CalibrateDecoder()
{
    psi::LogInfo() << "[TestModule] Calibrate Decoder (AddressLevels test)."
                   << std::endl;

    psi::LogInfo().PrintTimestamp();

    TestRange *testRange = new TestRange();
    testRange->CompleteRange();
    Test *aTest = new AddressLevels(testRange, tbInterface.get());
    aTest->ModuleAction(this);
}


void TestModule::AdjustTBMUltraBlack()
{
    psi::LogInfo().PrintTimestamp();

    TestRange *testRange = new TestRange();
    testRange->CompleteRange();
    Test *aTest = new TBMUbCheck(testRange, tbInterface.get());
    aTest->ModuleAction(this);
}


// Tries to automatically adjust Vana, may not work yet
void TestModule::AdjustVana(psi::ElectricCurrent goalCurrent)
{
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
    const psi::ElectricCurrent current0 = tbInterface->GetIA();

    psi::LogDebug() << "[TestModule] ZeroCurrent " << current0 << std::endl;

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
    psi::ElectricCurrent current = tbInterface->GetIA();

    psi::LogDebug() << "[TestModule] TotalCurrent " << current << std::endl;
}



void TestModule::VanaVariation()
{
    bool debug = true;

    psi::LogInfo() << "[TestModule] VanaVariation." << std::endl;

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

    const psi::ElectricCurrent current0 = tbInterface->GetIA();
    psi::LogDebug() << "[TestModule] ZeroCurrent " << current0 << std::endl;

    for (unsigned iRoc = 0; iRoc < rocs.size(); iRoc++)
    {
        if (debug)
            psi::LogDebug() << "[TestModule] Roc #" << iRoc << '.' << std::endl;

        GetRoc(iRoc)->SetDAC("Vana", vana[iRoc] - 10);
        tbInterface->Flush();
        psi::Sleep(1.0 * psi::seconds);
        x[0] = vana[iRoc] - 10;
        y[0] = psi::DataStorage::ToStorageUnits(tbInterface->GetIA() - current0);
        if (debug)
            psi::LogDebug() << "[TestModule] Vana " << x[0] << " Iana " << y[0]
                            << std::endl;

        GetRoc(iRoc)->SetDAC("Vana", vana[iRoc]);
        tbInterface->Flush();
        psi::Sleep(1.0 * psi::seconds);
        x[1] = vana[iRoc];
        y[1] = psi::DataStorage::ToStorageUnits(tbInterface->GetIA() - current0);
        if (debug)
            psi::LogDebug() << "[TestModule] Vana " << x[1] << " Iana " << y[1]
                            << std::endl;

        GetRoc(iRoc)->SetDAC("Vana", vana[iRoc] + 10);
        tbInterface->Flush();
        psi::Sleep(1.0 * psi::seconds);
        x[2] = vana[iRoc] + 10;
        y[2] = psi::DataStorage::ToStorageUnits(tbInterface->GetIA() - current0);
        if (debug)
            psi::LogDebug() << "[TestModule] Vana " << x[2] << " Iana " << y[2]
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
    psi::ElectricCurrent ia = tbInterface->GetIA();
    psi::ElectricPotential va = tbInterface->GetVA();
    psi::ElectricCurrent id = tbInterface->GetID();
    psi::ElectricPotential vd = tbInterface->GetVD();

    psi::LogDebug() << "[TestModule] ============== Currents and Voltages ==============" << std::endl;
    psi::LogDebug() << "[TestModule]    > Analog" << std::endl;
    psi::LogDebug() << "[TestModule]        I: " << ia << std::endl;
    psi::LogDebug() << "[TestModule]        V: " << va << std::endl;
    psi::LogDebug() << "[TestModule]    > Digital" << std::endl;
    psi::LogDebug() << "[TestModule]        I: " << id << std::endl;
    psi::LogDebug() << "[TestModule]        V: " << vd << std::endl;
    psi::LogDebug() << "[TestModule] ===================================================" << std::endl;

    psi::DataStorage::Active().SaveMeasurement("IA", ia);
    psi::DataStorage::Active().SaveMeasurement("VA", va);
    psi::DataStorage::Active().SaveMeasurement("ID", id);
    psi::DataStorage::Active().SaveMeasurement("VD", vd);
}


void TestModule::AdjustUltraBlackLevel()
{
    unsigned short count;
    short data[10000];

    psi::LogInfo() << "[TestModule] Adjust UltraBlack Levels." << std::endl;

    tbInterface->ADCData(data, count);

    if (count < tbInterface->GetEmptyReadoutLengthADC())
    {
        psi::LogInfo() << "[TestModule] Error: no valid analog readout." << std::endl;

        return;
    }

    int ultraBlackLevel = (data[0] + data[1] + data[2]) / 3;

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

    for (unsigned iRoc = 0; iRoc < rocs.size(); iRoc++)
    {
        dacValue = GetRoc(iRoc)->GetDAC(dacReg);
        GetRoc(iRoc)->SetDAC(dacReg, 0);
        tbInterface->Flush();
        tbInterface->ADCData(data, count);
        GetRoc(iRoc)->SetDAC(dacReg, max);
        tbInterface->Flush();
        tbInterface->ADCData(data2, count2);
        GetRoc(iRoc)->SetDAC(dacReg, dacValue);
        tbInterface->Flush();

        if (debug)
        {
            psi::LogInfo() << "count: " << count << std::endl;
            for (int i = 0; i < count; i++)
                psi::LogInfo() << data[i] << " ";
            psi::LogInfo() << std::endl << "count2: " << count2 << std::endl;
            for (int i = 0; i < count2; i++)
                psi::LogInfo() << data2[i] << " ";
            psi::LogInfo() << std::endl;
        }

        if ((count != tbInterface->GetEmptyReadoutLengthADC()) || (count2 != tbInterface->GetEmptyReadoutLengthADC()))
        {
            psi::LogInfo() << "[TestModule] Error: no valid analog readout." << std::endl;

            return false;
        }

        int offset;
        if (tbInterface->TBMPresent()) offset = 10;
        else if(ConfigParameters::Singleton().TbmEmulator()) offset = 10;
        else offset = 3;
        if (TMath::Abs(data[offset + iRoc * 3] - data2[offset + iRoc * 3]) < 20)
        {
            result = false;
            psi::LogInfo() << ">>>>>> Error ROC " << iRoc << ": DAC programming error\n";
        }
    }

    if (debug && result)
        psi::LogInfo() << "dac " << dacReg << " ok\n";
    if (debug && !result)
        psi::LogInfo() << ">>>>>>>> Error: dac " << dacReg << " not ok\n";
    return result;
}



void TestModule::TestDACProgramming()
{
    psi::LogInfo() << "[TestModule] Test if ROC DACs are programmable." << std::endl;

    bool result = true;

    result &= TestDACProgramming(25, 255);

    if (!result)
        psi::LogInfo() << ">>>>>> Error: DAC programming error\n";
}




// ----------------------------------------------------------------------
void TestModule::DumpParameters()
{
    WriteDACParameterFile(ConfigParameters::Singleton().FullDacParametersFileName().c_str());
}


void TestModule::DataTriggerLevelScan()
{
    int dtlOrig = ConfigParameters::Singleton().DataTriggerLevel();

    if ((!tbInterface->DataTriggerLevelScan()) && (ConfigParameters::Singleton().HalfModule() == 0))
    {
        TBM *tbm = GetTBM();
        int channel = tbInterface->GetTBMChannel();
        int singleDual = 0;
        const bool haveSingleDual = tbm->GetDAC(0, singleDual);

        // try for second tbm
        psi::LogInfo() << "[TestModule] Error: No valid readout for this TBM. "
                       << "Try with channel " << channel << '.' << std::endl;

        SetTBMSingle((channel + 1) % 2);
        tbInterface->SetTBMChannel((channel + 1) % 2);

        tbInterface->DataTriggerLevelScan();

        tbInterface->SetTBMChannel(channel);
        if(haveSingleDual)
            tbm->SetDAC(0, singleDual);
    }

    tbInterface->DataTriggerLevel(dtlOrig);
}


double TestModule::GetTemperature()
{
    psi::LogInfo().PrintTimestamp();
    double temperature = 0.;
    for (unsigned i = 0; i < rocs.size(); i++)
    {
        temperature += GetRoc(i)->GetTemperature();
    }
    temperature /= rocs.size();
    psi::LogDebug() << "[TestModule] Temperature: " << temperature << " (˙C)."
                    << std::endl;

    psi::LogInfo().PrintTimestamp();

    return temperature;
}

void TestModule::Scurves()
{
    DoTest(new FullTest(FullRange(), tbInterface.get(), 0));
}

unsigned TestModule::NRocs()
{
    return rocs.size();
}

bool TestModule::GetTBM(unsigned reg, int &value)
{
    return tbm->GetDAC(reg, value);
}

void TestModule::SetTBM(int chipId, unsigned reg, int value)
{
    tbm->SetDAC(reg, value);
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
    int dtl = 0, emptyReadoutLength = tbInterface->GetEmptyReadoutLengthADC();
    short data[10000];
    unsigned short count;

    do
    {
        dtl -= 50;
        tbInterface->DataTriggerLevel(dtl);
        tbInterface->ADCData(data, count);
    }
    while ((count != emptyReadoutLength) && (dtl > -2000));

    if (dtl == -2000)
    {
        // try with second tbm
        TBM *tbm = GetTBM();
        int channel = tbInterface->GetTBMChannel();
        dtl = 0;

        psi::LogInfo() << "[Module] Problem: Can not find data trigger level. "
                       << "Try different channel." << std::endl;

        SetTBMSingle((channel + 1) % 2);
        tbInterface->SetTBMChannel((channel + 1) % 2);

        do
        {
            dtl -= 50;
            tbInterface->DataTriggerLevel(dtl);
            tbInterface->ADCData(data, count);
        }
        while ((count != emptyReadoutLength) && (dtl > -2000));

        if (dtl != -2000)
            tbm->WriteTBMParameterFile(ConfigParameters::Singleton().FullTbmParametersFileName().c_str());
    }

    if (dtl == -2000)
    {
        // still no valid readout
        psi::LogInfo() << "[Module] Problem: Can not find data trigger level."
                       << std::endl;
        return;
    }

    dtl = (data[0] + data[1] + data[2]) / 3 + 100;

    tbInterface->DataTriggerLevel(dtl);
    if (dtl < -1200)
        psi::LogInfo() << "[Module] Warning: Very low data trigger level: "
                       << dtl << ". Check AOUT channels." << std::endl;

    ConfigParameters::ModifiableSingleton().setDataTriggerLevel(dtl);
    psi::LogInfo() << "[Module] Setting data trigger level to "
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
