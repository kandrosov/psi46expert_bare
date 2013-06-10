/*!
 * \file TestDACProgramming.cc
 * \brief Implementation of TestDACProgramming class.
 */

#include "psi/log.h"
#include "psi46expert/TestRoc.h"
#include "psi/exception.h"

#include "DacProgramming.h"

static const std::string LOG_HEAD = "DAC Programming";

using namespace psi::tests;

DacProgramming::DacProgramming(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface)
    : Test("DacProgramming", testRange), tbInterface(aTBInterface), dacReg(DACParameters::Vcal), maxDacValue(255)
{
    params->Branch("DacReg", reinterpret_cast<int*>(&dacReg));
    params->Branch("MaxDacValue", &maxDacValue);
    params->Fill();
}

void DacProgramming::RocAction(TestRoc& roc)
{
    psi::LogInfo(LOG_HEAD) << "Test if ROC DACs are programmable." << std::endl;

    unsigned short count, count2;
    short data[10000], data2[10000];
    int dacValue;

    dacValue = roc.GetDAC(dacReg);
    roc.SetDAC(dacReg, 0);
    tbInterface->Flush();
    tbInterface->ADCData(data, count);
    roc.SetDAC(dacReg, maxDacValue);
    tbInterface->Flush();
    tbInterface->ADCData(data2, count2);
    roc.SetDAC(dacReg, dacValue);
    tbInterface->Flush();

    psi::LogInfo(LOG_HEAD) << "roc " << roc.GetChipId() << ":\ncount: " << count << std::endl;
    for (int i = 0; i < count; i++)
        psi::LogInfo() << data[i] << " ";
    psi::LogInfo() << std::endl << "count2: " << count2 << std::endl;
    for (int i = 0; i < count2; i++)
        psi::LogInfo() << data2[i] << " ";
    psi::LogInfo() << std::endl;

    if ((count != tbInterface->GetEmptyReadoutLengthADC()) || (count2 != tbInterface->GetEmptyReadoutLengthADC()))
        THROW_PSI_EXCEPTION("no valid analog readout.");

    int offset;
    if (tbInterface->TBMPresent()) offset = 10;
    else if(ConfigParameters::Singleton().TbmEmulator()) offset = 10;
    else offset = 3;
    if (std::abs(data[offset + roc.GetChipId() * 3] - data2[offset + roc.GetChipId() * 3]) < 20)
        THROW_PSI_EXCEPTION("ROC " << roc.GetChipId() << ": DAC programming error");

    psi::LogDebug() << "dac " << dacReg << " ok\n";
}
