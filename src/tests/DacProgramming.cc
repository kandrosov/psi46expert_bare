/*!
 * \file TestDACProgramming.cc
 * \brief Implementation of TestDACProgramming class.
 *
 * \b Changelog
 * 25-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version. Code moved from TestModule.
 */

#include "psi/log.h"
#include "psi46expert/TestRoc.h"
#include "psi/exception.h"

#include "DacProgramming.h"

static const std::string LOG_HEAD = "DAC Programming";

using namespace psi::tests;

DacProgramming::DacProgramming(boost::shared_ptr<TBAnalogInterface> _tbInterface,
                               const std::vector< boost::shared_ptr<TestRoc> >& _rocs)
    : Test("dac_programming"), rocs(_rocs), tbInterface(_tbInterface), dacReg(25), maxDacValue(255)
{
    params->Branch("DacReg", &dacReg);
    params->Branch("MaxDacValue", &maxDacValue);
    params->Fill();
}

void DacProgramming::ModuleAction()
{
    psi::LogInfo(LOG_HEAD) << "Test if ROC DACs are programmable." << std::endl;

    unsigned short count, count2;
    short data[10000], data2[10000];
    int dacValue;

    for (unsigned iRoc = 0; iRoc < rocs.size(); iRoc++) {
        dacValue = rocs[iRoc]->GetDAC(dacReg);
        rocs[iRoc]->SetDAC(dacReg, 0);
        tbInterface->Flush();
        tbInterface->ADCData(data, count);
        rocs[iRoc]->SetDAC(dacReg, maxDacValue);
        tbInterface->Flush();
        tbInterface->ADCData(data2, count2);
        rocs[iRoc]->SetDAC(dacReg, dacValue);
        tbInterface->Flush();

        psi::LogDebug(LOG_HEAD) << "roc " << iRoc << ":" << std::endl <<  "count: " << count << std::endl;
        for (int i = 0; i < count; i++)
            psi::LogDebug() << data[i] << " ";
        psi::LogDebug() << std::endl << "count2: " << count2 << std::endl;
        for (int i = 0; i < count2; i++)
            psi::LogDebug() << data2[i] << " ";
        psi::LogDebug() << std::endl;

        if ((count != tbInterface->GetEmptyReadoutLengthADC()) || (count2 != tbInterface->GetEmptyReadoutLengthADC()))
            THROW_PSI_EXCEPTION("no valid analog readout.");

        int offset;
        if (tbInterface->TBMPresent()) offset = 10;
        else if(ConfigParameters::Singleton().TbmEmulator()) offset = 10;
        else offset = 3;
        if (std::abs(data[offset + iRoc * 3] - data2[offset + iRoc * 3]) < 20)
            THROW_PSI_EXCEPTION("ROC " << iRoc << ": DAC programming error");
    }

    psi::LogDebug() << "dac " << dacReg << " ok\n";
}
