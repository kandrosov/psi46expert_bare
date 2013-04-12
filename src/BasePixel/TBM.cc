/*!
 * \file TBM.cc
 * \brief Implementation of TBM class.
 *
 * \b Changelog
 * 12-04-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Defined enum TBMParameters::Register.
 * 13-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Using TBAnalogInterface instead TBInterface.
 *      - Mask constants moved into TBM.cc.
 *      - TBMParameters class now inherit psi::BaseConifg class.
 * 09-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Corrected questionable language constructions, which was found using -Wall g++ option.
 * 01-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Class SysCommand removed.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new ConfigParameters class definition.
 */


#include <fstream>

#include "BasePixel/TBM.h"
#include "BasePixel/TBMParameters.h"
#include "psi/log.h"
#include "BasePixel/TBAnalogInterface.h"

// define the masks
// Reg0
static const int m2040Readout     = 0x00000001;
static const int mDisableClock      = 0x00000002;
static const int mStackFull32       = 0x00000004;
static const int mPauseReadout      = 0x00000008;
static const int mIgnoreTriggers    = 0x00000010;
static const int mStackReadback     = 0x00000020;
static const int mDisableTriggerOut = 0x00000040;
static const int mDisableAnalogOut  = 0x00000080;
// Reg1
static const int mMode              = 0x000000C0;
static const int mDisableCalDelTrig = 0x00000020;
// Reg2
static const int mInjectTrigger     = 0x00000001;
static const int mInjectSync        = 0x00000002;
static const int mInjectROCReset    = 0x00000004;
static const int mInjectCal         = 0x00000008;
static const int mInjectTBMReset    = 0x00000010;
static const int mClearStack        = 0x00000020;
static const int mClearTokenOut     = 0x00000040;
static const int mClearTrigCounter  = 0x00000080;
// Reg4
static const int mDisableAnalogDrv  = 0x00000001;
static const int mDisableTokenOutDrv = 0x00000002;
static const int mForceReadoutClock = 0x00000004;

// Mode commands
static const int selectModeSync         = 0x00;
static const int selectModeClearCounter = 0x80;
static const int selectModeCal          = 0xC0;

TBM::TBM(int aCNId, boost::shared_ptr<TBAnalogInterface> aTbInterface)
    : controlNetworkId(aCNId), tbInterface(aTbInterface)
{
    if (ConfigParameters::Singleton().HubId() == -1) {
        ConfigParameters::ModifiableSingleton().setHubId(ScanHubIDs());
        ConfigParameters::Singleton().WriteConfigParameterFile();
    }

    hubId = ConfigParameters::Singleton().HubId();

    TBM1Reg0 = 0; //Base+1/0
    TBM1Reg1 = 0; //Base+3/2
    TBM1Reg2 = 0; //Base+5/4
    TBM1Reg3 = 0; //Base+7
    TBM1Reg4 = 0; //Base+9/8
    TBM1Reg5 = 0; // +B/A Analog Input Amplifier Bias = DAC0
    TBM1Reg6 = 0; // +D/C Analog Amplifier Driver Bias = DAC1
    TBM1Reg7 = 0; // +F/E TBM Analog Output Gain = DAC2
    // TBM2 registers (only first 5, no DACs)
    TBM2Reg0 = 0; //Base+1/0
    TBM2Reg1 = 0; //Base+3/2
    TBM2Reg2 = 0; //Base+5/4
    TBM2Reg3 = 0; //Base+7
    TBM2Reg4 = 0; //Base+9/8
}

void TBM::Initialize( const char *tbmParametersFileName)
{
    ReadTBMParameterFile(tbmParametersFileName);
    sleep(2);
}


bool TBM::GetDAC(TBMParameters::Register reg, int &value)
{
    return tbmParameters.Get(reg, value);
}

int TBM::ScanHubIDs()
{
    if (!tbInterface->TBMIsPresent()) return -1;
    int value;
    bool result;
    for (int i = 0; i < 32; i++) {
        tbInterface->ModAddr(i);
        result = GetReg(229, value);
        if (result) {
            psi::LogDebug() << "[TBM] HubID " << i << ": Module found." << std::endl;

            return i;
        }
    }

    psi::LogInfo() << "[TBM] Error: Can not find HubID." << std::endl;

    return -1;
}


void TBM::SetDAC(TBMParameters::Register reg, int value)
{
    tbmParameters.Set(*this, reg, value);
}


void TBM::ReadTBMParameterFile(const std::string& filename)
{
    tbmParameters.Read(filename);
    tbmParameters.Apply(*this);
    tbInterface->Flush();
}


void TBM::WriteTBMParameterFile(const std::string &filename)
{
    tbmParameters.Write(filename);
}


void TBM::SetTBMChannel(int tbmChannel)
{
    tbInterface->SetTBMChannel(tbmChannel);
}

bool TBM::GetReg(int reg, int &value)
{
    return tbInterface->GetTBMReg(reg, value);
}


int TBM::init(void)
{
    if (!tbInterface->TBMIsPresent()) return 0;

    int status = 0;

    status = setTBM1(2, 0xF0); //Clear: trigger count,token-out,stack + resetTBM
    status = setTBM2(2, 0xF0); // Same for TBM2

    if (status != 0) psi::LogInfo() << "Error in TBM init " << status << std::endl;
    return status;
}


// Program the TBM DACs 0,1,2
int TBM::setTBMDAC(const int DACAddress, const int value)
{
    int status = 0;
    // Check the register address
    if(DACAddress < 0 || DACAddress > 2 ) return -1;

    int temp = (value & 0xFF);  // keep only the lower 8 bits
    //psi::LogInfo() << " temp " << temp << endl;
    int registerAddress = DACAddress + 5;
    // For all routines RETURN - 0 sucess, -1 - error
    status = tbInterface->Tbm1write(hubId, registerAddress, temp);
    status = tbInterface->Tbm2write(hubId, registerAddress, temp);
    if(status != 0) return -1; // exit if error

    // Success, update the stored value
    switch(DACAddress) {
    case 0 :
        TBM1Reg5 = temp;
        //psi::LogInfo() << " reg " << TBM1Reg5 << endl;
        break;
    case 1 :
        TBM1Reg6 = temp;
        break;
    case 2 :
        TBM1Reg7 = temp;
        break;
    }
    return status;
}


int TBM::setTBM1(const int registerAddress, const int value)
{
    int status = 0;
    // Check the register address
    if(registerAddress < 0 || registerAddress > 4 ) return -1;

    int temp = value & 0xFF;  // keep only the lower 8 bits
    // For all routines RETURN - 0 sucess, -1 - error
    status = tbInterface->Tbm1write(hubId, registerAddress, temp);
    tbInterface->CDelay(100);
    if(status != 0) return -1; // exit if error

    // Success, update the stored value
    switch(registerAddress) {
    case 0 :
        TBM1Reg0 = temp;
        break;
    case 1 :
        TBM1Reg1 = temp;
        break;
    case 2 :
        TBM1Reg2 = temp;
        break;
    case 3 :
        TBM1Reg3 = temp;
        break;
    case 4 :
        TBM1Reg4 = temp;
        break;
    }
    return status;
}


int TBM::setTBM2(const int registerAddress, const int value)
{
    int status = 0;
    // Check the register address
    if(registerAddress < 0 || registerAddress > 4 ) return -1;

    int temp = value & 0xFF;  // keep only the lower 8 bits
    // For all routines RETURN - 0 sucess, -1 - error
    status = tbInterface->Tbm2write(hubId, registerAddress, temp);
    tbInterface->CDelay(100);
    if(status != 0) return -1; // exit if error

    // Success, update the stored value
    switch(registerAddress) {
    case 0 :
        TBM2Reg0 = temp;
        break;
    case 1 :
        TBM2Reg1 = temp;
        break;
    case 2 :
        TBM2Reg2 = temp;
        break;
    case 3 :
        TBM2Reg3 = temp;
        break;
    case 4 :
        TBM2Reg4 = temp;
        break;
    }
    return status;
}

// Switch ON selected CLEAR & INJECT bits
int TBM::setBit(const int tbm, const int bit)
{
    int status = 0;
    switch(tbm) {
    case 1:  // TBM1
        if(bit >= 0 && bit <= 7) {
            int tmp = 0x01 << bit;
            status = setTBM1Reg2(tmp, 0xFF);
            break;
        } else {
            psi::LogInfo() << " Wrong bit selected " << bit << std::endl;
            return -1;
        }
        break;
    case 2:  // TBM 2
        if(bit >= 0 && bit <= 7) {
            int tmp = 0x01 << bit;
            status = setTBM2Reg2(tmp, 0xFF);
            break;
        } else {
            psi::LogInfo() << " Wrong bit selected " << bit << std::endl;
            return -1;
        }
        break;
    default:  // wrong TBM
        psi::LogInfo() << " wrong TBM  selected, id = " << tbm << std::endl;
        return -1;
    }

    return status;
}

int TBM::set2040Readout(const int value)
{
    //
    int status = setTBM2Reg0(value, m2040Readout); // For both TBMs
    status = setTBM1Reg0(value, m2040Readout);
    return status;
}

int TBM::setIgnoreTriggers(const int value)
{
    int status = setTBM1Reg0(value, mIgnoreTriggers);
    status = setTBM2Reg0(value, mIgnoreTriggers);
    return status;
}

int TBM::setDisableTriggers(const int value)
{
    int status = setTBM1Reg0(value, mDisableTriggerOut);
    status = setTBM2Reg0(value, mDisableTriggerOut);
    return status;
}

int TBM::setMode(const int value)
{
    int status = setTBM1Reg1(value, mMode);
    status = setTBM2Reg1(value, mMode);
    return status;
}

int TBM::setDisableCalDelTrig(const int value)
{
    int status = setTBM1Reg1(value, mDisableCalDelTrig);
    status = setTBM2Reg1(value, mDisableCalDelTrig);
    return status;
}
