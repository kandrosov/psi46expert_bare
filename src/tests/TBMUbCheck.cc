/*!
 * \file TBMUbCheck.cc
 * \brief Implementation of TBMUbCheck class.
 */

#include "TBMUbCheck.h"
#include "BasePixel/constants.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/TBM.h"
#include "psi46expert/TestModule.h"

#include "BasePixel/TestParameters.h"

TBMUbCheck::TBMUbCheck(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface)
    : Test("TBMUbCheck", testRange), tbInterface(aTBInterface)
{
    ubTarget = TestParameters::Singleton().TBMUbLevel();
}

void TBMUbCheck::ModuleAction(TestModule& module)
{
    psi::LogInfo() << "Starting TBMUbCheck" << std::endl;

    TBM& tbm = module.GetTBM();
    const ConfigParameters& configParameters = ConfigParameters::Singleton();
    bool halfModule = !(configParameters.HalfModule() == 0);

    short data[psi::FIFOSIZE];
    const int nTrig = 10;
    bool tbmChannelOk;

    int dtlOrig = configParameters.DataTriggerLevel();
    int dtl = TMath::Min(ubTarget + 300, -200);
    tbInterface->DataTriggerLevel(dtl);

    int tbmChannel_saved = tbInterface->GetTBMChannel();
    int tbmMode_saved = 0;
    const bool haveSavedTbmMode = tbm.GetDAC(TBMParameters::Single, tbmMode_saved);
    int tbmGain_saved = 0;
    const bool haveSavedTbmGain = tbm.GetDAC(TBMParameters::Dacgain, tbmGain_saved);

    int tbmGain_target = 0;

    int nChannels;
    if (halfModule) nChannels = 1;
    else nChannels = 2;

    for ( int itbm = 0; itbm < nChannels; itbm++ ) {
        if (!halfModule) {
            tbInterface->SetTBMChannel(itbm);
            module.SetTBMSingle(itbm);
        }
        tbmChannelOk = false;

        int tbmGain = 255;
        for ( int ibit = 7; ibit >= 0; ibit-- ) {
            int bitValue = (ibit >= 1) ? (2 << (ibit - 1)) : 1;
            int tbmGain_new  = tbmGain - bitValue;

            tbm.SetDAC(TBMParameters::Dacgain, tbmGain_new);

            unsigned short count = 0;
            tbInterface->ADCRead(data, count, nTrig);

            if ( count > 0 ) {
                tbmChannelOk = true;
                double ubLevel = (data[0] + data[1] + data[2]) / 3.;
                if ( ubLevel < ubTarget ) tbmGain = tbmGain_new;
            }
        }

        unsigned short count = 0;
        tbInterface->ADCRead(data, count, nTrig);
        if ( count > 0 ) {
            double ubLevel = data[0];
            psi::LogInfo() << "tbmGain = " << tbmGain << ", ubLevel = " << ubLevel << std::endl;
        }

//--- save maximum Dacgain of both TBMs
//    (can only set one Dacgain value for both)
        if ( tbmChannelOk && (tbmGain > tbmGain_target) ) tbmGain_target = tbmGain;
    }

    if (tbmGain_target == 0 && haveSavedTbmGain) tbmGain_target = tbmGain_saved;

    psi::LogInfo() << "setting tbmGain to " << tbmGain_target << std::endl;
    tbm.SetDAC(TBMParameters::Dacgain, tbmGain_target);

//--- restore previous TBM settings
    tbInterface->SetTBMChannel(tbmChannel_saved);
    if(haveSavedTbmMode)
        tbm.SetDAC(TBMParameters::Single, tbmMode_saved);
    tbInterface->DataTriggerLevel(dtlOrig);
    tbInterface->Flush();

    tbm.WriteTBMParameterFile(configParameters.FullTbmParametersFileName().c_str());
}
