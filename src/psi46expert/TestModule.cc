/*!
 * \file TestModule.cc
 * \brief Implementation of TestModule class.
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
#include "tests/DacProgramming.h"

#include "TestModule.h"

TestModule::TestModule(int aCNId, boost::shared_ptr<TBAnalogInterface> aTBInterface)
    : controlNetworkId(aCNId), tbInterface(aTBInterface), fullRange(new TestRange())
{
    const ConfigParameters& configParameters = ConfigParameters::Singleton();

    if (configParameters.CustomModule() == 0) {
        const unsigned   nRocs = configParameters.NumberOfRocs();

        tbm = boost::shared_ptr<TBM>(new TBM(aCNId, tbInterface));
        tbm->init();
        hubId = configParameters.HubId();

        int offset = 0;
        if (configParameters.HalfModule() == 2) offset = 8;
        for (unsigned i = 0; i < nRocs; i++)
            rocs.push_back( boost::shared_ptr<TestRoc>(new TestRoc(tbInterface, *this, i + offset, hubId,
                                                                   int((i + offset) / 4), i)));
    } else if (configParameters.CustomModule() == 1) {
        psi::LogInfo() << "[TestModule] Custom module constructor: Ignoring nRocs, "
                       << "hubID, ... in config file." << std::endl;
        psi::LogInfo() << "[TestModule] Using: 4 ROCs with ChipID/PortID 0,1/0 "
                       << "and 8,9/2." << std::endl;

        hubId = 0;

        tbm = boost::shared_ptr<TBM>(new TBM(aCNId, tbInterface));
        tbm->init();

        //nRocs = 4;
        rocs.push_back( boost::shared_ptr<TestRoc>(new TestRoc(tbInterface, *this, 1, hubId, 0, 0)));
        rocs.push_back( boost::shared_ptr<TestRoc>(new TestRoc(tbInterface, *this, 0, hubId, 0, 1)));
        rocs.push_back( boost::shared_ptr<TestRoc>(new TestRoc(tbInterface, *this, 8, hubId, 2, 2)));
        rocs.push_back( boost::shared_ptr<TestRoc>(new TestRoc(tbInterface, *this, 9, hubId, 2, 3)));
    }

    for (unsigned i = 0; i < rocs.size(); i++)
        fullRange->CompleteRoc(GetRoc(i).GetChipId());
}

void TestModule::FullTestAndCalibration()
{
    AdjustDACParameters();
    DoTest(boost::shared_ptr<FullTest>(new FullTest(FullRange(), tbInterface)));

    psi::LogInfo() << "[TestModule] PhCalibration: Start." << std::endl;

    for (unsigned i = 0; i < rocs.size(); i++) GetRoc(i).DoPhCalibration();

    psi::LogInfo() << "[TestModule] PhCalibration: End." << std::endl;

    psi::LogInfo() << "[TestModule] Trim: Start." << std::endl;

    for (unsigned i = 0; i < rocs.size(); i++) GetRoc(i).DoTrim();

    psi::LogInfo() << "[TestModule] Trim: End." << std::endl;
}


void TestModule::ShortCalibration()
{
    AdjustAllDACParameters();
    VanaVariation();
    DoTest(boost::shared_ptr<Test>(new PixelAlive(FullRange(), tbInterface)));

    psi::LogInfo() << "[TestModule] PhCalibration: Start." << std::endl;

    for (unsigned i = 0; i < rocs.size(); i++) GetRoc(i).DoPhCalibration();

    psi::LogInfo() << "[TestModule] PhCalibration: End." << std::endl;
}


void TestModule::ShortTestAndCalibration()
{
    AdjustAllDACParameters();
    VanaVariation();
    DoTest(boost::shared_ptr<Test>(new PixelAlive(FullRange(), tbInterface)));
    DoTest(boost::shared_ptr<Test>(new BumpBonding(FullRange(), tbInterface)));

    psi::LogInfo() << "[TestModule] PhCalibration: Start." << std::endl;

    for (unsigned i = 0; i < rocs.size(); i++) GetRoc(i).DoPhCalibration();

    psi::LogInfo() << "[TestModule] PhCalibration: End." << std::endl;

    psi::LogInfo() << "[TestModule] Trim: Start." << std::endl;

    boost::shared_ptr<TrimLow> trimLow(new TrimLow(FullRange(), tbInterface));
    trimLow->SetVcal(40);
    //trimLow->NoTrimBits(true);
    DoTest(trimLow);

    psi::LogInfo() << "[TestModule] Trim: End." << std::endl;
}


void TestModule::DoTest(boost::shared_ptr<Test> aTest)
{
    aTest->ModuleAction(*this);
}

void TestModule::DoTBMTest()
{
    DoTest(boost::shared_ptr<Test>(new TBMTest(fullRange, tbInterface)));
}

void TestModule::AnaReadout()
{
    DoTest(boost::shared_ptr<Test>(new AnalogReadout(fullRange, tbInterface)));
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
    for(int dacRegister = 3; dacRegister < 4; dacRegister++) {
        const std::string& dacName = DACParameters::GetRegisterName((DACParameters::Register)dacRegister);

        for (unsigned iRoc = 0; iRoc < rocs.size(); iRoc++) {

            TH1D *currentHist = new TH1D(Form("currentHist%i_ROC%i", dacRegister, iRoc), Form("%s", dacName.c_str()), 26, 0, 260);

            psi::LogInfo() << "Testing ROC " << iRoc << std::endl;

            GetRoc(iRoc).SaveDacParameters();
            for(int dacValue = 0; dacValue < 260; dacValue += 10) {
                GetRoc(iRoc).SetDAC((DACParameters::Register)dacRegister, dacValue);

                psi::LogInfo() << dacName << " set to " << dacValue << std::endl;
                tbInterface->Flush();
                sleep(2);
                //dc = ((TBAnalogInterface*)tbInterface)->GetIA();
                const psi::ElectricCurrent dc = tbInterface->GetID();
                psi::LogInfo() << "Digital current: " << dc << std::endl;
                currentHist->SetBinContent((dacValue / 10) + 1, psi::DataStorage::ToStorageUnits(dc));
            }
            GetRoc(iRoc).RestoreDacParameters();
        }
    }

}

void TestModule::AdjustSamplingPoint()
{
    bool debug = false;
    short data[10000], ph[25];
    unsigned short count;
    FILE *file;

    if (debug) {
        char fname[1000];
        sprintf(fname, "sampling.dat");
        file = fopen(fname, "w");
        if (!file) return;
    }

    int ctrlReg = GetRoc(0).GetDAC(DACParameters::CtrlReg);
    int vcal = GetRoc(0).GetDAC(DACParameters::Vcal);

    GetRoc(0).SetDAC(DACParameters::CtrlReg, 4);
    GetRoc(0).SetDAC(DACParameters::Vcal, 255);
    GetRoc(0).AdjustCalDelVthrComp(5, 5, 255, 0);

    int clk = tbInterface->GetParameter(TBParameters::clk);
    int sda = tbInterface->GetParameter(TBParameters::sda);
    int ctr = tbInterface->GetParameter(TBParameters::ctr);
    int tin = tbInterface->GetParameter(TBParameters::tin);
    int rda = tbInterface->GetParameter(TBParameters::rda);

    if (debug)
        psi::LogInfo() << "clk " << clk << std::endl;
    const ConfigParameters& configParameters = ConfigParameters::Singleton();

    for (int delay = 0; delay < 25; ++delay) {
        if (debug)
            psi::LogDebug() << "[TestModule] Delay " << delay << '.' << std::endl;

        tbInterface->SetTBParameter(TBParameters::clk, (clk + delay) );
        tbInterface->SetTBParameter(TBParameters::sda, (sda + delay) );
        tbInterface->SetTBParameter(TBParameters::ctr, (ctr + delay) );
        tbInterface->SetTBParameter(TBParameters::tin, (tin + delay) );
        if(configParameters.TbmEmulator()) tbInterface->SetTBParameter(TBParameters::rda, (100 - (tin + delay)) ); // ask chris to be true with the tbmemulator


        tbInterface->Flush();

        int nTrig = 10;

        GetRoc(0).ArmPixel(5, 5); //pixel must not be enabled during setting of the tb parameters above
        tbInterface->Flush();

        tbInterface->ADCRead(data, count, nTrig);

        GetRoc(0).DisarmPixel(5, 5);
        tbInterface->Flush();

        if (count > 16) ph[delay] = data[16];
        else ph[delay] = -9999;

        if (debug) {

            psi::LogInfo() << "count " << count << std::endl;
            fprintf(file, "%+.3i: ", clk + delay);
            for (int i = 0; i < count; i++) fprintf(file, "%+.3i ", data[i]);
            fprintf(file, "\n");
        }
    }
    if (debug) fclose(file);

    GetRoc(0).SetDAC(DACParameters::CtrlReg, ctrlReg);
    GetRoc(0).SetDAC(DACParameters::Vcal, vcal);
    tbInterface->Flush();

    short maxPH = -9999;
    int maxDelay = 0, lowerDelay, upperDelay;
    for (int i = 0; i < 25; i++) {
        if (debug)
            psi::LogInfo() << "ph " << ph[i] << std::endl;
        if (ph[i] > maxPH) {
            maxPH = ph[i];
            maxDelay = i;
        }
    }

    upperDelay = maxDelay;
    do {
        upperDelay++;
        if (upperDelay == 25) upperDelay = 0;
    } while (ph[upperDelay] > maxPH - 20);

    lowerDelay = maxDelay;
    do {
        lowerDelay--;
        if (lowerDelay == -1) lowerDelay = 24;
    } while (ph[lowerDelay] > maxPH - 20);

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

    tbInterface->SetTBParameter(TBParameters::clk, (clk + bestDelay));
    tbInterface->SetTBParameter(TBParameters::sda, (sda + bestDelay));
    tbInterface->SetTBParameter(TBParameters::ctr, (ctr + bestDelay));
    tbInterface->SetTBParameter(TBParameters::tin, (tin + bestDelay));
    tbInterface->SetTBParameter(TBParameters::rda, bestRda);
    if(configParameters.TbmEmulator()) {
        psi::LogInfo() << "I'm in the tbmEmulator" << std::endl;
        tbInterface->SetTBParameter(TBParameters::rda, (100 - (tin + bestDelay)) ); // ask chris
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
    if (tbmPresent) {
        tbInterface->SetTriggerMode(TRIGGER_MODULE1); // trigger mode 2 only works correctly after adjusting tbm and roc ultrablacks to the same level
        if(!configParameters.TbmEmulator())  AdjustTBMUltraBlack();
        AdjustDTL();
    }
    DoTest(boost::shared_ptr<Test>(new psi::tests::DacProgramming(fullRange, tbInterface)));
    AdjustVana(0.024 * psi::amperes);
    psi::LogInfo().PrintTimestamp();
    if (tbmPresent) {
        AdjustUltraBlackLevel();
        tbInterface->SetTriggerMode(TRIGGER_MODULE2);
        AdjustSamplingPoint();
    }
    psi::LogInfo().PrintTimestamp();
//        AdjustCalDelVthrComp();
//        DoTest(new TimeWalkStudy(FullRange(), tbInterface));
    AdjustCalDelVthrComp();

    AdjustPHRange();
    DoTest(boost::shared_ptr<Test>(new VsfOptimization(FullRange(), tbInterface)));

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

    if (tbmPresent) {
        // trigger mode 2 only works correctly after adjusting tbm and roc ultrablacks to the same level
        tbInterface->SetTriggerMode(TRIGGER_MODULE1);
        if(!configParameters.TbmEmulator())   AdjustTBMUltraBlack();
        AdjustDTL();
    }
    DoTest(boost::shared_ptr<Test>(new psi::tests::DacProgramming(fullRange, tbInterface)));
    AdjustVana(0.024 * psi::amperes);
    MeasureCurrents();
    if (tbmPresent) {
        AdjustUltraBlackLevel();
        tbInterface->SetTriggerMode(TRIGGER_MODULE2);
        AdjustSamplingPoint();
    }
    for (unsigned iRoc = 0; iRoc < rocs.size(); iRoc++) {
        psi::LogDebug() << "[TestModule] Roc #" << GetRoc(iRoc).GetChipId()
                        << '.' << std::endl;

        GetRoc(iRoc).AdjustCalDelVthrComp();
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
    for (unsigned iRoc = 0; iRoc < rocs.size(); iRoc++) {
        psi::LogDebug() << "[TestModule] Roc #" << GetRoc(iRoc).GetChipId()
                        << '.' << std::endl;

        GetRoc(iRoc).AdjustCalDelVthrComp();
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

    DoTest(boost::shared_ptr<Test>(new UbCheck(fullRange, tbInterface)));
}

void TestModule::AdjustPHRange()
{
    psi::LogInfo() << "[TestModule] Adjust PhRange." << std::endl;

    psi::LogInfo().PrintTimestamp();

    DoTest(boost::shared_ptr<Test>(new PHRange(fullRange, tbInterface)));
}


void TestModule::CalibrateDecoder()
{
    psi::LogInfo() << "[TestModule] Calibrate Decoder (AddressLevels test)."
                   << std::endl;

    psi::LogInfo().PrintTimestamp();

    DoTest(boost::shared_ptr<Test>(new AddressLevels(fullRange, tbInterface)));
}


void TestModule::AdjustTBMUltraBlack()
{
    psi::LogInfo().PrintTimestamp();

    DoTest(boost::shared_ptr<Test>(new TBMUbCheck(fullRange, tbInterface)));
}

// Tries to automatically adjust Vana, may not work yet
void TestModule::AdjustVana(psi::ElectricCurrent goalCurrent)
{
    int vana[rocs.size()];
    int vsf[rocs.size()];

    for (unsigned iRoc = 0; iRoc < rocs.size(); iRoc++) {
        vsf[iRoc] = GetRoc(iRoc).GetDAC(DACParameters::Vsf);

        GetRoc(iRoc).SetDAC(DACParameters::Vana, 0);
        GetRoc(iRoc).SetDAC(DACParameters::Vsf, 0);
    }
    tbInterface->Flush();
    psi::Sleep(0.5 * psi::seconds);
    const psi::ElectricCurrent current0 = tbInterface->GetIA();

    psi::LogDebug() << "[TestModule] ZeroCurrent " << current0 << std::endl;

    for (unsigned iRoc = 0; iRoc < rocs.size(); iRoc++) {
        vana[iRoc] = GetRoc(iRoc).AdjustVana(current0, goalCurrent);
        GetRoc(iRoc).SetDAC(DACParameters::Vana, 0);
    }
    for (unsigned iRoc = 0; iRoc < rocs.size(); iRoc++) {
        GetRoc(iRoc).SetDAC(DACParameters::Vana, vana[iRoc]);
        GetRoc(iRoc).SetDAC(DACParameters::Vsf, vsf[iRoc]);
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

    for (unsigned iRoc = 0; iRoc < rocs.size(); iRoc++) {
        vsf[iRoc] = GetRoc(iRoc).GetDAC(DACParameters::Vsf);
        vana[iRoc] = GetRoc(iRoc).GetDAC(DACParameters::Vana);

        GetRoc(iRoc).SetDAC(DACParameters::Vana, 0);
        GetRoc(iRoc).SetDAC(DACParameters::Vsf, 0);
    }
    tbInterface->Flush();
    psi::Sleep(2.0 * psi::seconds);

    const psi::ElectricCurrent current0 = tbInterface->GetIA();
    psi::LogDebug() << "[TestModule] ZeroCurrent " << current0 << std::endl;

    for (unsigned iRoc = 0; iRoc < rocs.size(); iRoc++) {
        if (debug)
            psi::LogDebug() << "[TestModule] Roc #" << iRoc << '.' << std::endl;

        GetRoc(iRoc).SetDAC(DACParameters::Vana, vana[iRoc] - 10);
        tbInterface->Flush();
        psi::Sleep(1.0 * psi::seconds);
        x[0] = vana[iRoc] - 10;
        y[0] = psi::DataStorage::ToStorageUnits(tbInterface->GetIA() - current0);
        if (debug)
            psi::LogDebug() << "[TestModule] Vana " << x[0] << " Iana " << y[0]
                            << std::endl;

        GetRoc(iRoc).SetDAC(DACParameters::Vana, vana[iRoc]);
        tbInterface->Flush();
        psi::Sleep(1.0 * psi::seconds);
        x[1] = vana[iRoc];
        y[1] = psi::DataStorage::ToStorageUnits(tbInterface->GetIA() - current0);
        if (debug)
            psi::LogDebug() << "[TestModule] Vana " << x[1] << " Iana " << y[1]
                            << std::endl;

        GetRoc(iRoc).SetDAC(DACParameters::Vana, vana[iRoc] + 10);
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

        GetRoc(iRoc).SetDAC(DACParameters::Vana, 0);
        tbInterface->Flush();
    }

    for (unsigned iRoc = 0; iRoc < rocs.size(); iRoc++) {
        GetRoc(iRoc).SetDAC(DACParameters::Vana, vana[iRoc]);
        GetRoc(iRoc).SetDAC(DACParameters::Vsf, vsf[iRoc]);
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

    if (count < tbInterface->GetEmptyReadoutLengthADC()) {
        psi::LogInfo() << "[TestModule] Error: no valid analog readout." << std::endl;

        return;
    }

    int ultraBlackLevel = (data[0] + data[1] + data[2]) / 3;

    for (unsigned iRoc = 0; iRoc < rocs.size(); iRoc++) {
        GetRoc(iRoc).AdjustUltraBlackLevel(ultraBlackLevel);
    }
}

// ----------------------------------------------------------------------
void TestModule::DumpParameters()
{
    WriteDACParameterFile(ConfigParameters::Singleton().FullDacParametersFileName().c_str());
}


void TestModule::DataTriggerLevelScan()
{
    int dtlOrig = ConfigParameters::Singleton().DataTriggerLevel();

    if ((!tbInterface->DataTriggerLevelScan()) && (ConfigParameters::Singleton().HalfModule() == 0)) {
        TBM& tbm = GetTBM();
        int channel = tbInterface->GetTBMChannel();
        int singleDual = 0;
        const bool haveSingleDual = tbm.GetDAC(TBMParameters::Single, singleDual);

        // try for second tbm
        psi::LogInfo() << "[TestModule] Error: No valid readout for this TBM. "
                       << "Try with channel " << channel << '.' << std::endl;

        SetTBMSingle((channel + 1) % 2);
        tbInterface->SetTBMChannel((channel + 1) % 2);

        tbInterface->DataTriggerLevelScan();

        tbInterface->SetTBMChannel(channel);
        if(haveSingleDual)
            tbm.SetDAC(TBMParameters::Single, singleDual);
    }

    tbInterface->DataTriggerLevel(dtlOrig);
}


double TestModule::GetTemperature()
{
    psi::LogInfo().PrintTimestamp();
    double temperature = 0.;
    for (unsigned i = 0; i < rocs.size(); i++) {
        temperature += GetRoc(i).GetTemperature();
    }
    temperature /= rocs.size();
    psi::LogDebug() << "[TestModule] Temperature: " << temperature << " (˙C)."
                    << std::endl;

    psi::LogInfo().PrintTimestamp();

    return temperature;
}

void TestModule::Scurves()
{
    DoTest(boost::shared_ptr<Test>(new SCurveTest(fullRange, tbInterface)));
}

unsigned TestModule::NRocs()
{
    return rocs.size();
}

bool TestModule::GetTBM(TBMParameters::Register reg, int &value)
{
    return tbm->GetDAC(reg, value);
}

void TestModule::SetTBM(int chipId, TBMParameters::Register reg, int value)
{
    tbm->SetDAC(reg, value);
}

void TestModule::SetTBMSingle(int tbmChannel)
{
    if (tbmChannel == 0) tbm->SetDAC(TBMParameters::Single, 0);
    else tbm->SetDAC(TBMParameters::Single, 2);
}

void TestModule::AdjustDTL()
{
    int dtl = 0, emptyReadoutLength = tbInterface->GetEmptyReadoutLengthADC();
    short data[10000];
    unsigned short count;

    do {
        dtl -= 50;
        tbInterface->DataTriggerLevel(dtl);
        tbInterface->ADCData(data, count);
    } while ((count != emptyReadoutLength) && (dtl > -2000));

    if (dtl == -2000) {
        // try with second tbm
        TBM& tbm = GetTBM();
        int channel = tbInterface->GetTBMChannel();
        dtl = 0;

        psi::LogInfo() << "[Module] Problem: Can not find data trigger level. "
                       << "Try different channel." << std::endl;

        SetTBMSingle((channel + 1) % 2);
        tbInterface->SetTBMChannel((channel + 1) % 2);

        do {
            dtl -= 50;
            tbInterface->DataTriggerLevel(dtl);
            tbInterface->ADCData(data, count);
        } while ((count != emptyReadoutLength) && (dtl > -2000));

        if (dtl != -2000)
            tbm.WriteTBMParameterFile(ConfigParameters::Singleton().FullTbmParametersFileName().c_str());
    }

    if (dtl == -2000) {
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

    for (unsigned i = 0; i < rocs.size(); i++) {
        rocs[i]->Initialize();
    }
}

// -- Write the current DAC settings of all rocs to disk
void TestModule::WriteDACParameterFile( const char* filename)
{
    for (unsigned i = 0; i < rocs.size(); i++) rocs[i]->WriteDACParameterFile(filename);
}
