/*!
 * \file TBMTest.cc
 * \brief Implementation of TBMTest class.
 */

#include "psi/log.h"

#include "TBMTest.h"
#include "psi46expert/TestModule.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/ConfigParameters.h"
#include <TParameter.h>


// e5 f5

TBMTest::TBMTest(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface)
    : Test("TBMTest", testRange), tbInterface(aTBInterface) {}

void TBMTest::ModuleAction(TestModule& module)
{
    const ConfigParameters& configParameters = ConfigParameters::Singleton();

    result[0] = 0;
    result[1] = 0;

    if (!tbInterface->TBMIsPresent()) return;
    if (configParameters.HalfModule() == 0) DualModeTest(module);
    ReadoutTest(module);

    TParameter<int> *parameter0 = new TParameter<int>("TBM1", result[0]);
    TParameter<int> *parameter1 = new TParameter<int>("TBM2", result[1]);
    histograms->Add(parameter0);
    histograms->Add(parameter1);
}

void TBMTest::ReadoutTest(TestModule& module)
{
    TBM& tbm = module.GetTBM();

    int value;
    bool res = tbm.GetReg(229, value);
    if (debug)
        psi::LogInfo() << "value " << value << std::endl;

    if (!res) {
        result[0] += 4;
        psi::LogInfo() << "[TBMTest] Error: TBM1 readout test failed." << std::endl;
    }

    res = tbm.GetReg(245, value);
    if (debug)
        psi::LogInfo() << "value2 " << value << std::endl;

    if (!res) {
        result[1] += 4;
        psi::LogInfo() << "[TBMTest] Error: TBM2 readout test failed." << std::endl;
    }
}

void TBMTest::DualModeTest(TestModule& module)
{
    TBM& tbm = module.GetTBM();
    const ConfigParameters& configParameters = ConfigParameters::Singleton();

    unsigned short count;
    short data[10000];

    psi::LogInfo() << "[TBMTest] Start." << std::endl;

    int channel = tbInterface->GetTBMChannel();
    int singleDual = 0;
    const bool haveSingleDual = tbm.GetDAC(TBMParameters::Single, singleDual);

    int dtlOrig = configParameters.DataTriggerLevel(), dtl = 0;
    tbInterface->DataTriggerLevel(dtl);

    for (int k = 0; k < 2; k++) {
        module.SetTBMSingle(k);
        tbInterface->SetTBMChannel(k);

        dtl = dtlOrig;
        do {
            tbInterface->DataTriggerLevel(dtl);
            tbInterface->Flush();
            tbInterface->ADCData(data, count);
            dtl += 50;
        } while((count != tbInterface->GetEmptyReadoutLengthADC()) && (dtl < 0));

        if (count != tbInterface->GetEmptyReadoutLengthADC()) {
            result[k] += 1;

            psi::LogInfo() << "[TBMTest] Error: test failed for TBM #" << k
                           << ". No valid analog readout." << std::endl;

            if (k == 1)
                psi::LogInfo() << "[TBMTest] Error: test failed. Does testboard "
                               << "have 2 ADCs?" << std::endl;

            for (int i = 0; i < count; i++)
                psi::LogInfo() << " " << data[i];
            psi::LogInfo() << std::endl;
        }

        tbm.setDualMode();

        dtl = dtlOrig;
        do {
            tbInterface->DataTriggerLevel(dtl);
            tbInterface->Flush();
            tbInterface->ADCData(data, count);
            dtl += 50;
        } while((count != tbInterface->GetEmptyReadoutLengthADCDual()) && (dtl < 0));

        if (count != tbInterface->GetEmptyReadoutLengthADCDual()) {
            result[k] += 2;

            psi::LogInfo() << "[TBMTest] Error: test failed for TBM #" << k
                           << ". No valid dual analog readout." << std::endl;

            if (k == 1)
                psi::LogInfo() << "[TBMTest] Error: test failed. Does testboard "
                               << "have 2 ADCs?" << std::endl;

            for (int i = 0; i < count; i++)
                psi::LogInfo() << " " << data[i];
            psi::LogInfo() << std::endl;
        }
    }

    tbInterface->SetTBMChannel(channel);
    if(haveSingleDual)
        tbm.SetDAC(TBMParameters::Single, singleDual);
    tbInterface->DataTriggerLevel(dtlOrig);
    tbInterface->Flush();
}
