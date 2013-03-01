/*!
 * \file AnalogTestBoard.cc
 * \brief Implementation of AnalogTestBoard class.
 *
 * \b Changelog
 * 01-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using a new PSI Logging System.
 *      - Class SysCommand removed.
 * 28-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - All functionality extracted from TBAnalogInterface class to AnalogTestBoard class. TBAnalogInterface is now
 *        abstract.
 *
 * (From TBAnalogInterface.cc)
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using definitions from PsiCommon.h.
 * 15-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using boost::units::quantity to represent physical values.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new ConfigParameters class definition.
 */

#include "BasePixel/AnalogTestBoard.h"
#include "BasePixel/TBAnalogParameters.h"
#include "constants.h"
#include "BasePixel/RawPacketDecoder.h"
#include "psi/log.h"
#include "interface/USBInterface.h"
#include "interface/Delay.h"
#include "psi/exception.h"

AnalogTestBoard::AnalogTestBoard()
{
    Initialize();
}

AnalogTestBoard::~AnalogTestBoard()
{
    HVoff();
    Poff();
    Cleanup();

    delete cTestboard;
}

//void AnalogTestBoard::Execute(SysCommand &command)
//{
//    int buf[2];
//    int *value=&buf[0]; int *reg=&buf[1];
//    int delay;
//    if (command.Keyword("pon"))    {Pon();}
//    else if (command.Keyword("poff"))   {Poff();}
//    else if (command.Keyword("hvoff"))   {HVoff();}
//    else if (command.Keyword("hvon"))   {HVon();}
//    else if (command.Keyword("loop"))   {Intern(rctk_flag);}
//    else if (command.Keyword("stop"))   {Single(0);}
//    else if (command.Keyword("single")) {Single(rctk_flag);}
//    else if (command.Keyword("setreg",&reg,&value)) {SetReg(*reg,*value);}
//    else if (command.Keyword("ext"))    {Extern(rctk_flag);}
//    else if (command.Keyword("ia"))    {
//    psi::Log<psi::Info>() << "[AnalogTestBoard] Analog current " << GetIA() << std::endl;
//  }
//    else if (command.Keyword("id"))    {
//    psi::Log<psi::Info>() << "[AnalogTestBoard] Digital current " << GetID() << std::endl;
//  }
//    else if (command.Keyword("getvd"))    {
//    psi::Log<psi::Info>() << "[AnalogTestBoard] Digital voltage " << GetVD() << std::endl;
//  }
//    else if (command.Keyword("getva"))    {
//    psi::Log<psi::Info>() << "[AnalogTestBoard] Analog voltage " << GetVA() << std::endl;
//  }
//    else if (command.Keyword("res"))    {Single(0x08);}  //reset
//    else if (command.Keyword("reseton"))    {ResetOn();}
//    else if (command.Keyword("resetoff"))    {ResetOff();}
//    else if (command.Keyword("dtlScan")) {DataTriggerLevelScan();}

//    else if (strcmp(command.carg[0],"dv") == 0) SetVD(((double)*command.iarg[1])*SysCommand::VOLTAGE_FACTOR);
//    else if (strcmp(command.carg[0],"av") == 0) SetVA(((double)*command.iarg[1])*SysCommand::VOLTAGE_FACTOR);
//    else if (strcmp(command.carg[0],"dtl") == 0) {DataTriggerLevel(-*command.iarg[1]);}
//    else if (strcmp(command.carg[0],"enableAll") == 0) SetEnableAll(*command.iarg[1]);

//    else if (command.Keyword("t_res_cal", &value))
//    {delay=RangeCheck(*value,5,255); SetTBParameter("trc",delay);}
//    else if (command.Keyword("trc", &value))
//    {delay=RangeCheck(*value,5,255); SetTBParameter("trc",delay);}
//    else if (command.Keyword("t_cal_cal", &value))
//    {delay=RangeCheck(*value,5,255); SetTBParameter("tcc",delay);}
//    else if (command.Keyword("tcc", &value))
//    {delay=RangeCheck(*value,5,255); SetTBParameter("tcc",delay);}
//    else if (command.Keyword("t_cal_trig", &value))
//    {delay=RangeCheck(*value,5,255); SetTBParameter("tct",delay);}
//    else if (command.Keyword("tct", &value))
//    {delay=RangeCheck(*value,5,255); SetTBParameter("tct",delay);}
//    else if (command.Keyword("t_trg_tok", &value))
//    {delay=RangeCheck(*value,5,255); SetTBParameter("ttk",delay);}
//    else if (command.Keyword("ttk", &value))
//    {delay=RangeCheck(*value,5,255); SetTBParameter("ttk",delay);}
//    else if (command.Keyword("t_rep", &value))
//    {delay=RangeCheck(*value,1,120); SetTBParameter("trep",delay);}
//    else if (command.Keyword("trep", &value))
//    {delay=RangeCheck(*value,1,120); SetTBParameter("trep",delay);}
//    else if (command.Keyword("calrep", &value))
//    {delay=RangeCheck(*value,1,120); SetTBParameter("cc",delay);}
//    else if (command.Keyword("cc", &value))
//    {delay=RangeCheck(*value,1,120); SetTBParameter("cc",delay);}
//    else if (command.Keyword("seq", &value))
//    {rctk_flag=RangeCheck(*value,0,15);}
//    else if (command.Keyword("ctr", &value))
//    {delay=RangeCheck(*value,1,120); SetTBParameter("ctr",delay);}
//    else
//    {
//        if (!tbParameters->Execute(command))
//      psi::Log<psi::Info>() << "[AnalogTestBoard] Unknown testboard command: "
//                     << command.carg[0] << std::endl;
//    }

//}


// == General functions ================================================


void  AnalogTestBoard::Pon()
{
    cTestboard->Pon();
    cTestboard->Flush();
}


void AnalogTestBoard::Poff()
{
    cTestboard->Poff();
    cTestboard->Flush();
}


void AnalogTestBoard::Set(int reg, int value)
{
    cTestboard->Set(reg,value);
}


void AnalogTestBoard::SetReg(int reg, int value)
{
    cTestboard->SetReg(reg,value);
}


void AnalogTestBoard::Single(int mask)
{
    cTestboard->Single(mask);
}


void AnalogTestBoard::Intern(int mask)
{
    cTestboard->Intern(mask);
}


void AnalogTestBoard::Extern(int mask)
{
    cTestboard->Extern(mask);
}


int AnalogTestBoard::GetRoCnt()
{
    return cTestboard->GetRoCntEx();
}


void AnalogTestBoard::Initialize()
{
    const ConfigParameters& configParameters = ConfigParameters::Singleton();
    tbParameters = (TBParameters*)new TBAnalogParameters(this);

    signalCounter = 0;
    readPosition = 0;
    writePosition = 0;
    triggerSource = 0;

    cTestboard = new CTestboard();
    if (!cTestboard->Open(configParameters.TestboardName().c_str()))
        return;
    fIsPresent = 1;

    cTestboard->Welcome();

    char s[260];
    GetVersion(s,260);
    psi::Log<psi::Info>() << "---- TestBoard Version" << s << std::endl;


    if (configParameters.TbmEnable()) {
      tbmenable = 1;
      SetTriggerMode(TRIGGER_MODULE2);
    }
    else {
      tbmenable = 0;
      SetTriggerMode(TRIGGER_ROC);
    }

    if (configParameters.TbmEmulator()){
      cTestboard->TBMEmulatorOn();
      psi::Log<psi::Info>() <<"TBM emulator on"<< std::endl;
    }
    else {
      cTestboard->TBMEmulatorOff();
      psi::Log<psi::Info>() <<"TBM emulator off"<< std::endl;
    }

    Pon();

    I2cAddr(0);
    rctk_flag=15;

    SetTBMChannel(configParameters.TbmChannel());
    Tbmenable(configParameters.TbmEnable());

    SetIA(configParameters.IA());
    SetID(configParameters.ID());
    SetVA(configParameters.VA());
    SetVD(configParameters.VD());

    SetEmptyReadoutLength(configParameters.EmptyReadoutLength());
    SetEmptyReadoutLengthADC(configParameters.EmptyReadoutLengthADC());
    SetEmptyReadoutLengthADCDual(configParameters.EmptyReadoutLengthADCDual());

    if (configParameters.HighVoltageOn()) HVon();
    DataTriggerLevel(configParameters.DataTriggerLevel());

    cTestboard->SetHubID(configParameters.HubId());
    cTestboard->SetNRocs(configParameters.NumberOfRocs());
    cTestboard->SetEnableAll(0);

    DataEnable(true);
    cTestboard->ResetOn(); // send hard reset to connected modules / TBMs
    cTestboard->Flush();
    gDelay->Mdelay(100);
    cTestboard->ResetOff();
    cTestboard->Flush();

    ReadTBParameterFile(configParameters.FullTbParametersFileName().c_str());  //only after power on
}


void AnalogTestBoard::Clear()
{
    cTestboard->Clear();
}


int AnalogTestBoard::Startup(int port)
{
    return 1;
}


void AnalogTestBoard::Cleanup()
{
    cTestboard->Close();
}


int AnalogTestBoard::Present()
{
    return 1;
}


void AnalogTestBoard::I2cAddr(int id)
{
    cTestboard->I2cAddr(id);
}


void AnalogTestBoard::SetTriggerMode(unsigned short mode)
{
        cTestboard->SetTriggerMode(mode);
}


void AnalogTestBoard::SetEmptyReadoutLength(int length)
{
    emptyReadoutLength = length;
    cTestboard->SetEmptyReadoutLength(length);
}


int AnalogTestBoard::GetEmptyReadoutLength()
{
    return emptyReadoutLength;
}


void AnalogTestBoard::SetEmptyReadoutLengthADC(int length)
{
    emptyReadoutLengthADC = length;
    cTestboard->SetEmptyReadoutLengthADC(length);
}


void AnalogTestBoard::SetEmptyReadoutLengthADCDual(int length)
{
    emptyReadoutLengthADCDual = length;
}


int AnalogTestBoard::GetEmptyReadoutLengthADC()
{
    return emptyReadoutLengthADC;
}


int AnalogTestBoard::GetEmptyReadoutLengthADCDual()
{
    return emptyReadoutLengthADCDual;
}


void AnalogTestBoard::SetEnableAll(int value)
{
    cTestboard->SetEnableAll(value);
}


unsigned short AnalogTestBoard::GetModRoCnt(unsigned short index)
{
        return cTestboard->GetModRoCnt(index);
}


// == TBM functions ======================================================

void AnalogTestBoard::Tbmenable(int on)
{
    TBMpresent = on;
    cTestboard->tbm_Enable(on);
    SetReg41();
}


void AnalogTestBoard::ModAddr(int hub)
{
    cTestboard->mod_Addr(hub);
}


void AnalogTestBoard::TbmAddr(int hub, int port)
{
    cTestboard->tbm_Addr(hub, port);
}


int AnalogTestBoard::TbmWrite(const int hubAddr, const int addr, const int value)
{
    if (!cTestboard->TBMPresent()) return -1;
    cTestboard->TbmWrite(hubAddr, addr, value);
    return 0;
}


int AnalogTestBoard::Tbm1write(const int hubAddr, const int registerAddress, const int value)
{
    if (!cTestboard->TBMPresent()) return -1;
    cTestboard->Tbm1Write(hubAddr, registerAddress, value);
    return 0;
}


int AnalogTestBoard::Tbm2write(const int hubAddr, const int registerAddress, const int value)
{
    if (!cTestboard->TBMPresent()) return -1;
    cTestboard->Tbm2Write(hubAddr, registerAddress, value);
    return 0;
}


bool AnalogTestBoard::GetTBMReg(int reg, int &value)
{
    unsigned char v, r = (unsigned char)reg;
    bool result = cTestboard->tbm_Get(r, v);
    value = (int)v;
    return result;
}


// == ROC functions ======================================================


void AnalogTestBoard::SetChip(int chipId, int hubId, int portId, int aoutChipPosition)
{
    cTestboard->tbm_Addr(hubId, portId);
    cTestboard->roc_I2cAddr(chipId);
        cTestboard->SetAoutChipPosition(aoutChipPosition);
}

void AnalogTestBoard::RocClrCal()
{
    cTestboard->roc_ClrCal();
}


void AnalogTestBoard::RocSetDAC(int reg, int value)
{
    cTestboard->roc_SetDAC(reg, value);
}


void AnalogTestBoard::RocPixTrim(int col, int row, int value)
{
    cTestboard->roc_Pix_Trim(col, row, value);
}


void AnalogTestBoard::RocPixMask(int col, int row)
{
    cTestboard->roc_Pix_Mask(col, row);
}


void AnalogTestBoard::RocPixCal(int col, int row, int sensorcal)
{
    cTestboard->roc_Pix_Cal(col, row, sensorcal);
}


void AnalogTestBoard::RocColEnable(int col, int on)
{
    cTestboard->roc_Col_Enable(col, on);
}


void AnalogTestBoard::Flush()
{
    cTestboard->Flush();
}


void AnalogTestBoard::SetClock(int n)
{
    cTestboard->SetClock(n);
    Flush();
}


// == Analog functions =================================================


void AnalogTestBoard::DataTriggerLevel(int level)
{
    cTestboard->DataTriggerLevel(TBMChannel, level);
    cTestboard->SetDTL(level);
}


void AnalogTestBoard::DataCtrl(bool clear, bool trigger, bool cont)
{
    cTestboard->DataCtrl(TBMChannel, clear, trigger, cont);
}


void AnalogTestBoard::DataEnable(bool on)
{
        cTestboard->DataEnable(on);
}



bool AnalogTestBoard::DataRead(short buffer[], unsigned short buffersize, unsigned short &wordsread)
{
    return cTestboard->DataRead(TBMChannel, buffer, buffersize, wordsread);
}


void AnalogTestBoard::SetDelay(int signal, int ns)
{
    cTestboard->SetDelay(signal, ns);
    Flush();
}

void AnalogTestBoard::SetClockStretch(unsigned char src, unsigned short delay, unsigned short width)
{
  cTestboard->SetClockStretch(src, delay, width);
  Flush();
}


void AnalogTestBoard::CDelay(unsigned int clocks)
{
    cTestboard->cDelay(clocks);
}


bool AnalogTestBoard::SendRoCnt()
{
        //works only for trigger mode MODULE1
    if (signalCounter == 30000) ReadBackData();
    signalCounter++;
    return cTestboard->SendRoCntEx();
}


int AnalogTestBoard::RecvRoCnt()
{
        //works only for trigger mode MODULE1
    if (signalCounter == 0  && readPosition == writePosition)  //buffer empty and nothing to read
    {
    psi::Log<psi::Info>() << "[AnalogTestBoard] Error: no signal to read from testboard."
                   << std::endl;
        return -1;
    }
    else if (readPosition == writePosition) {   //buffer is empty
        signalCounter--;
        return cTestboard->RecvRoCntEx();
    }
    else {
        int data = dataBuffer[readPosition];   //buffer not empty
        readPosition++;
        if (readPosition == bufferSize) readPosition = 0;
        return data;
    }
}


void AnalogTestBoard::SingleCal()
{
    Single(RES|CAL|TRG|TOK);
    CDelay(500); //CDelay(100) is too short
}


int AnalogTestBoard::CountReadouts(int count, int chipId)
{
        return cTestboard->CountReadouts(count, chipId);
}


void AnalogTestBoard::SendCal(int nTrig)
{
    for (int i = 0; i < nTrig; i++)
    {
        SingleCal();
        SendRoCnt();
    }
}


int AnalogTestBoard::CountADCReadouts(int count)
{
    unsigned short counter;
    short data[psi::FIFOSIZE];

    int n = 0;
    for (int i = 0; i < count; i++)
    {
        DataCtrl(false, true); // no clear, trigger
        Single(RES|CAL|TRG|TOK);
        CDelay(100);
        Flush();
        DataRead(data,psi::FIFOSIZE,counter);
        n+= ((int)counter - 56) / 6;
    }
    return n;
}


bool AnalogTestBoard::ADCData(short buffer[], unsigned short &wordsread)
{
    ADCRead(buffer, wordsread);
    return true;
}




unsigned short AnalogTestBoard::ADC()
{
    unsigned short count;
    short data[psi::FIFOSIZE];
    ADCRead(data, count);
    //cTestboard->ProbeSelect(0,PROBE_ADC_COMP);
    //cTestboard->ProbeSelect(1,PROBE_ADC_GATE);

  psi::Log<psi::Debug>() << "[AnalogTestBoard] Count " << count << std::endl;
  std::cout<<"[AnalogTestBoard] Count " << count << std::endl;

  //	for (unsigned int n = 0; n < count; n++) data[n] &= 0xf000;
  psi::Log<psi::Debug>() << "[AnalogTestBoard] Data: ";
  std::cout<< "[AnalogTestBoard] Data: "<< std::endl;
    for (unsigned int n = 0; n < count; n++)
    {
      psi::Log<psi::Debug>() <<" "<< data[n];
      std::cout<<" "<< data[n];
    }

    psi::Log<psi::Debug>() << std::endl;
  std::cout<<std::endl;
    return count;
}

unsigned short AnalogTestBoard::ADC(int nbsize)
{

    unsigned short count;
    short data[psi::FIFOSIZE];
    ADCRead(data, count);
    // probe with the scope the Gate and the comp output signal
    //cTestboard->ProbeSelect(0,PROBE_ADC_COMP);
    //cTestboard->ProbeSelect(1,PROBE_ADC_GATE);
    //	cout<<"&&&&&&& AnalogTestBoard::ADCData "<<endl;
    //	cout<<"start testing the reset"<<endl;
    //	cTestboard->ResetOn();
    //	cTestboard->mDelay(1000);
    //	cTestboard->ResetOff();
    //	cout<<"reset On and off done!"<<endl;
    //	cTestboard->Flush();

    if(nbsize>0)
      {
        // run adc with fix trigger mode
        cTestboard->SetReg(41,32);
        cTestboard->SetTriggerMode(TRIGGER_FIXED);
        cTestboard->DataBlockSize(200);
        cTestboard->DataCtrl(0, false, true, false);
        cTestboard->Single(RES|CAL|TRG);
        cTestboard->mDelay(100);
        cTestboard->DataRead(0, data, psi::FIFOSIZE, count);
        cTestboard->mDelay(100);
        cTestboard->Intern(RES|CAL|TRG);
        cTestboard->Flush();
        // 	cTestboard->Welcome();
      }

  psi::Log<psi::Debug>() << "[AnalogTestBoard] Count " << count << std::endl;
  std::cout<<"[AnalogTestBoard] Count " << count << std::endl;

// 	for (unsigned int n = 0; n < count; n++) data[n] &= 0xf000;

  psi::Log<psi::Debug>() << "[AnalogTestBoard] Data: ";
  std::cout<< "[AnalogTestBoard] Data: ";
    for (unsigned int n = 0; n < count; n++)
    {
    psi::Log<psi::Debug>() <<" " << data[n];
    std::cout<<" " << data[n];
    }
    psi::Log<psi::Debug>() << std::endl;
  std::cout<<std::endl;


        if (tbmenable)SetTriggerMode(TRIGGER_MODULE2);
        else SetTriggerMode(TRIGGER_ROC);

    return count;
}




// -- sends n calibrate signals and gives back the resulting ADC readout
void AnalogTestBoard::SendADCTrigs(int nTrig)
{
    for (int i = 0; i < nTrig; i++)
    {
        DataCtrl(false, true); // no clear, trigger
        Single(RES|CAL|TRG|TOK);
        CDelay(500);
    }
}


int AnalogTestBoard::LastDAC(int nTriggers, int chipId)
{
        int numRepetitions = 0;

    unsigned short count = 0;
    short data[psi::FIFOSIZE];
    while ( count == 0 && numRepetitions < 100 ){
      ADCRead(data, count, nTriggers);

      //cout << "ADC = { ";
      //for ( int i = 0; i < count; i++ ){
      //  cout << data[i] << " ";
      //}
      //cout << "} " << endl;

      numRepetitions++;
    }

    if ( numRepetitions >= 100 ){
      std::cerr << "Error in <AnalogTestBoard::LastDAC>: cannot find ADC signal !" << std::endl;
      return 0;
    }

    return data[10 + chipId*3];
}


void AnalogTestBoard::SendADCTrigsNoReset(int nTrig)
{
        for (int i = 0; i < nTrig; i++)
        {
                DataCtrl(false, true); // no clear, trigger
                Single(CAL|TRG);
                CDelay(500);
        }
}


bool AnalogTestBoard::GetADC(short buffer[], unsigned short buffersize, unsigned short &wordsread, int nTrig, int startBuffer[], int &nReadouts)
{
    RawPacketDecoder *gDecoder = RawPacketDecoder::Singleton();
    nReadouts = 0;

    while (!DataRead(buffer, buffersize, wordsread))
    {
        Clear();
        std::cout << "usb cleared" << std::endl;
        return false;
    }


    if (wordsread > 0) {
        for (int pos = 0; pos < (wordsread - 2); pos++)
        {
            if (gDecoder->isUltraBlackTBM(buffer[pos]) && gDecoder->isUltraBlackTBM(buffer[pos+1]) && gDecoder->isUltraBlackTBM(buffer[pos+2]))
            {
                if (nReadouts < nTrig) startBuffer[nReadouts] = pos;
                nReadouts++;
            }
        }
    }

    return (nReadouts <= nTrig);
}



bool AnalogTestBoard::DataTriggerLevelScan()
{
    unsigned short count;
    bool result = false;
    for (int delay = 0; delay < 2000; delay = delay + 50)
    {
    psi::Log<psi::Debug>() << "[AnalogTestBoard] dtl: " << delay
                    << " -------------------------------------" << std::endl;

        DataTriggerLevel(-delay);
        Flush();
        count = ADC();
        if (count == emptyReadoutLengthADC) result = true;
    }
    return result;
}




void AnalogTestBoard::SetVA(psi::ElectricPotential V)
{
    cTestboard->SetVA(V);
}


void AnalogTestBoard::SetIA(psi::ElectricCurrent A)
{
    cTestboard->SetIA(A);
}

void AnalogTestBoard::SetVD(psi::ElectricPotential V)
{
    cTestboard->SetVD(V);
}


void AnalogTestBoard::SetID(psi::ElectricCurrent A)
{
    return cTestboard->SetID(A);
}


psi::ElectricPotential AnalogTestBoard::GetVA()
{
    return cTestboard->GetVA();
}


psi::ElectricCurrent AnalogTestBoard::GetIA()
{
    return cTestboard->GetIA();
}


psi::ElectricPotential AnalogTestBoard::GetVD()
{
    return cTestboard->GetVD();
}


psi::ElectricCurrent AnalogTestBoard::GetID()
{
    return cTestboard->GetID();
}


void AnalogTestBoard::HVon()
{
    cTestboard->HVon();
}


void AnalogTestBoard::HVoff()
{
    cTestboard->HVoff();
}


void AnalogTestBoard::ResetOn()
{
    cTestboard->ResetOn();
}


void AnalogTestBoard::ResetOff()
{
    cTestboard->ResetOff();
}


void AnalogTestBoard::SetTBMChannel(int channel)
{
        TBMChannel = channel;
    cTestboard->SetTbmChannel(channel);
    SetReg41();
}


int AnalogTestBoard::GetTBMChannel()
{
        return TBMChannel;
}


// ----------------------------------------------------------------------
bool AnalogTestBoard::Mem_ReadOut(FILE *f, unsigned int addr, unsigned int size) {

  unsigned short BLOCKSIZE = 32767;
  unsigned char buffer[BLOCKSIZE];
  for (int i = 0; i < BLOCKSIZE; i++) buffer[i] = 0;

  Flush();
  Clear();
  unsigned int bound= static_cast<unsigned int>( 2. * size / BLOCKSIZE);
  unsigned int start=addr;

//   cout << "r/o of " << 2.*size  << " bytes with blocksize " << BLOCKSIZE << " starting from memory address " << addr << endl;
  for(unsigned int j=0;j<bound;++j) {
    cTestboard->MemRead(start,BLOCKSIZE,buffer);
    start+=BLOCKSIZE;
    fwrite(buffer, BLOCKSIZE, 1, f);
  }

  unsigned short rest = (addr + 2 * size - start);
  cTestboard->MemRead(start, rest, buffer);
  fwrite(buffer, rest, 1, f);
  Clear();
}



void AnalogTestBoard::SetReg41()
{
        int value(0);
    value = TBMChannel;
    if (TBMpresent) value += 2;
    if (triggerSource) value += 16;
    else value += 32;

    SetReg(41, value);
}


void AnalogTestBoard::StartDataTaking()
{
    DataCtrl(false, false, true); // go
    SetReg(43, 2);
    SetReg(41, 0x2A);
    Flush();
}


void AnalogTestBoard::StopDataTaking()
{
    SetReg(41, 0x22);
    DataCtrl(false, false, false); // stop
    Flush();
}


// == buffer functions ===========================================================================

void AnalogTestBoard::ReadBackData()
{
    Flush();
    for (int i = 0; i < signalCounter; i++)
    {
        dataBuffer[writePosition] = cTestboard->RecvRoCntEx();
        writePosition++;
        if (writePosition == bufferSize) {writePosition = 0;}
        if (writePosition == readPosition)
        {
      psi::Log<psi::Info>() << "[AnalogTestBoard] Error: Signalbuffer full in "
                     << "AnalogTestBoard ! Data loss possible !!!"
                     << std::endl;
            return;
        }
    }
    signalCounter = 0;
}


int AnalogTestBoard::AoutLevel(int position, int nTriggers)
{
    return cTestboard->AoutLevel(position, nTriggers);
}


void AnalogTestBoard::DoubleColumnADCData(int doubleColumn, short data[], int readoutStop[])
{
    cTestboard->DoubleColumnADCData(doubleColumn, data, readoutStop);
}


int AnalogTestBoard::ChipThreshold(int start, int step, int thrLevel, int nTrig, int dacReg, int xtalk, int cals, int trim[], int res[])
{
        DataEnable(false);
    int n =  cTestboard->ChipThreshold(start, step, thrLevel, nTrig, dacReg, xtalk, cals, trim, res);
        DataEnable(true);
        return n;
}


int AnalogTestBoard::AoutLevelChip(int position, int nTriggers, int trims[], int res[])
{
    return cTestboard->AoutLevelChip(position, nTriggers, trims, res);
}


int AnalogTestBoard::AoutLevelPartOfChip(int position, int nTriggers, int trims[], int res[], bool pxlFlags[])
{
        return cTestboard->AoutLevelPartOfChip(position, nTriggers, trims, res, pxlFlags);
}


int AnalogTestBoard::ChipEfficiency(int nTriggers, int trim[], double res[])
{
        DataEnable(false);
        int n = cTestboard->ChipEfficiency(nTriggers, trim, res);
        DataEnable(true);
        return n;
}


int AnalogTestBoard::MaskTest(short nTriggers, short res[])
{
        DataEnable(false);
    int n = cTestboard->MaskTest(nTriggers, res);
        DataEnable(true);
        return n;
}



int AnalogTestBoard::PixelThreshold(int col, int row, int start, int step, int thrLevel, int nTrig, int dacReg, int xtalk, int cals, int trim)
{
        DataEnable(false);
    int n = cTestboard->PixelThreshold(col, row, start, step, thrLevel, nTrig, dacReg, xtalk, cals, trim);
        DataEnable(true);
        return n;
}


int AnalogTestBoard::SCurve(int nTrig, int dacReg, int threshold, int res[])
{
        DataEnable(false);
    int n = cTestboard->SCurve(nTrig, dacReg, threshold, res);
        DataEnable(true);
        return n;
}


int AnalogTestBoard::SCurveColumn(int column, int nTrig, int dacReg, int thr[], int trims[], int chipId[], int res[])
{
        DataEnable(false);
    int n = cTestboard->SCurveColumn(column, nTrig, dacReg, thr, trims, chipId, res);
        DataEnable(true);
        return n;
}


void AnalogTestBoard::ADCRead(short buffer[], unsigned short &wordsread, short nTrig)
{
    cTestboard->ADCRead(buffer, wordsread, nTrig);
}


void AnalogTestBoard::DacDac(int dac1, int dacRange1, int dac2, int dacRange2, int nTrig, int result[])
{
        DataEnable(false);
    cTestboard->DacDac(dac1, dacRange1, dac2, dacRange2, nTrig, result);
        DataEnable(true);
}


void AnalogTestBoard::PHDac(int dac, int dacRange, int nTrig, int position, short result[])
{
    cTestboard->PHDac(dac, dacRange, nTrig, position, result);
}


void AnalogTestBoard::AddressLevels(int position, int result[])
{
    cTestboard->AddressLevels(position, result);
}


void AnalogTestBoard::TBMAddressLevels(int result[])
{
    cTestboard->TBMAddressLevels(result);
}


void AnalogTestBoard::TrimAboveNoise(short nTrigs, short thr, short mode, short result[])
{
        DataEnable(false);
    cTestboard->TrimAboveNoise(nTrigs, thr, mode, result);
        DataEnable(true);
}

// --------------------------------------------------------

void AnalogTestBoard::ProbeSelect(unsigned char port, unsigned char signal){
  cTestboard->ProbeSelect(port, signal);
}


int AnalogTestBoard::demo(short x)
{
  return cTestboard->demo(x);
}


void AnalogTestBoard::ScanAdac(unsigned short chip, unsigned char dac,
      unsigned char min, unsigned char max, char step,
      unsigned char rep, unsigned int usDelay, unsigned char res[])
{
  DataEnable(false);
  cTestboard->ScanAdac(chip, dac, min, max, step, rep, usDelay, res);
  DataEnable(true);
}

void AnalogTestBoard::CdVc(unsigned short chip, unsigned char wbcmin, unsigned char wbcmax, unsigned char vcalstep,
          unsigned char cdinit, unsigned short &lres, unsigned short res[])
{
  DataEnable(false);
  cTestboard->CdVc(chip, wbcmin, wbcmax, vcalstep, cdinit, lres, res);
  DataEnable(true);
}

char AnalogTestBoard::CountAllReadouts(int nTrig, int counts[], int amplitudes[])
{
  return cTestboard->CountAllReadouts(nTrig, counts, amplitudes);
}
bool AnalogTestBoard::GetVersion(char *s, unsigned int n)
{
  return cTestboard->GetVersion(s, n);
}
