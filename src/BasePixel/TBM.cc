/*!
 * \file TBM.cc
 * \brief Implementation of TBM class.
 *
 * \b Changelog
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

TBM::TBM(int aCNId, TBInterface *aTbInterface)
{
    tbInterface = aTbInterface;

    if (ConfigParameters::Singleton().HubId() == -1)
    {
        ConfigParameters::ModifiableSingleton().setHubId(ScanHubIDs());
        ConfigParameters::Singleton().WriteConfigParameterFile();
    }

    hubId = ConfigParameters::Singleton().HubId();
    controlNetworkId = aCNId;
    tbmParameters = new TBMParameters(this);

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


TBM::~TBM() {}


void TBM::Initialize( const char *tbmParametersFileName)
{
    ReadTBMParameterFile(tbmParametersFileName);
    sleep(2);
};


int TBM::GetDAC(int reg)
{
    return tbmParameters->GetDAC(reg);
}


int TBM::ScanHubIDs()
{
    if (!tbInterface->TBMIsPresent()) return -1;
    int value;
    bool result;
    for (int i = 0; i < 32; i++)
    {
        ((TBAnalogInterface*)tbInterface)->ModAddr(i);
        result = GetReg(229, value);
        if (result)
        {
            psi::LogDebug() << "[TBM] HubID " << i << ": Module found." << std::endl;

            return i;
        }
    }

    psi::LogInfo() << "[TBM] Error: Can not find HubID." << std::endl;

    return -1;
}


void TBM::SetDAC(int reg, int value)
{
    return tbmParameters->SetParameter(reg, value);
}


bool TBM::ReadTBMParameterFile( const char *filename)
{
    bool result = tbmParameters->ReadTBMParameterFile( filename);
    tbInterface->Flush();
    return result;
}


bool TBM::WriteTBMParameterFile(const char* filename)
{
    return tbmParameters->WriteTBMParameterFile(filename);
}


void TBM::SetTBMChannel(int tbmChannel)
{
    ((TBAnalogInterface*)tbInterface)->SetTBMChannel(tbmChannel);
}

bool TBM::GetReg(int reg, int &value)
{
    return ((TBAnalogInterface*)tbInterface)->GetTBMReg(reg, value);
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
    switch(DACAddress)
    {
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
    switch(registerAddress)
    {
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
    switch(registerAddress)
    {
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
    switch(tbm)
    {
    case 1:  // TBM1
        if(bit >= 0 && bit <= 7)
        {
            int tmp = 0x01 << bit;
            status = setTBM1Reg2(tmp, 0xFF);
            break;
        }
        else
        {
            psi::LogInfo() << " Wrong bit selected " << bit << std::endl;
            return -1;
        }
        break;
    case 2:  // TBM 2
        if(bit >= 0 && bit <= 7)
        {
            int tmp = 0x01 << bit;
            status = setTBM2Reg2(tmp, 0xFF);
            break;
        }
        else
        {
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





