/*!
 * \file AnalogTestBoard.h
 * \brief Definition of AnalogTestBoard class.
 *
 * \b Changelog
 * 28-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - All functionality extracted from TBAnalogInterface class to AnalogTestBoard class. TBAnalogInterface is now
 *        abstract.
 */

#pragma once

#include "TBAnalogInterface.h"

/*!
 * This class provides the functionality to program the analog testboard via USB
 * This class is mainly a dummy class which forwards the commands to Beat's
 * testboard class CTestboard. A USB read buffer was added, because problems
 * occured under linux, when more than about 32K signals were sent without
 * reading back.
 */
class AnalogTestBoard : public TBAnalogInterface
{
public:
    AnalogTestBoard();
    virtual ~AnalogTestBoard();

    // == General functions ================================================

    virtual void Execute(SysCommand &command);
    virtual void Pon();
    virtual void Poff();
    virtual void Set(int reg, int value);
    virtual void SetReg(int reg, int value);
    virtual void Single(int mask);
    virtual void Intern(int mask);
    virtual void Extern(int mask);
    virtual int GetRoCnt();
    virtual void Initialize();
    virtual int Startup(int port);
    virtual void Cleanup();
    virtual void Clear();
    virtual int Present();
    virtual void I2cAddr(int id);
    virtual int IsPresent() {return fIsPresent;}
    virtual bool IsAnalogTB() {return true;}
    virtual int CountReadouts(int count, int chipId);
    virtual void SingleCal();
    virtual void SendCal(int nTrig);
    virtual void SetEmptyReadoutLength(int length);
    virtual int GetEmptyReadoutLength();
    virtual void SetEmptyReadoutLengthADC(int length);
    virtual int GetEmptyReadoutLengthADC();
    virtual void SetEmptyReadoutLengthADCDual(int length);
    virtual int GetEmptyReadoutLengthADCDual();
    virtual void SetEnableAll(int value);
    virtual unsigned short GetModRoCnt(unsigned short index);

    // == Analog functions =================================================

    virtual void SetClock(int mhz);
    virtual void DataCtrl(bool clear, bool trigger, bool cont = false);
    virtual void DataEnable(bool on);
    virtual bool DataRead(short buffer[], unsigned short buffersize, unsigned short &wordsread);
    virtual void SetDelay(int signal, int ns);
    virtual void SetClockStretch(unsigned char src, unsigned short delay, unsigned short width);
    virtual void CDelay(unsigned int us);
    virtual bool SendRoCnt();
    virtual int  RecvRoCnt();
    virtual void Flush();
    virtual void DataTriggerLevel(int level);
    virtual void SetTriggerMode(unsigned short mode);
    virtual void SetTBMChannel(int channel);
    virtual int GetTBMChannel();
    virtual bool TBMPresent() { return cTestboard->TBMPresent(); }

    virtual void ADCRead(short buffer[], unsigned short &wordsread, short nTrig = 1);
    virtual bool ADCData(short buffer[], unsigned short &wordsread);
    virtual unsigned short ADC(int nbsize);
    virtual unsigned short ADC();
    virtual void SendADCTrigs(int nTrig);
    virtual void SendADCTrigsNoReset(int nTrig);
    virtual bool GetADC(short buffer[], unsigned short buffersize, unsigned short &wordsread, int nTrig, int startBuffer[], int &nReadouts);
    virtual int LastDAC(int nTrig, int chipId);

    virtual void SetVA(psi::ElectricPotential V);   // set VA voltage in V
    virtual void SetIA(psi::ElectricCurrent A);   // set VA current limit in A
    virtual void SetVD(psi::ElectricPotential V);   // set VD voltage in V
    virtual void SetID(psi::ElectricCurrent A);   // set VD current limit in A

    virtual psi::ElectricPotential GetVA(); // get VA voltage in V
    virtual psi::ElectricCurrent GetIA(); // get VA current in A
    virtual psi::ElectricPotential GetVD(); // get VD voltage in V
    virtual psi::ElectricCurrent GetID(); // get VD current in A

    virtual void HVon();    // switch HV relais on
    virtual void HVoff(); // switch HV relais off

    virtual void ResetOn(); // switch RESET-line to reset state (low)
    virtual void ResetOff();  // switch RESET-line to not reset state (high)
    virtual int CountADCReadouts(int count);

    virtual bool Mem_ReadOut(FILE *file, unsigned int addr, unsigned int size);

    virtual void SetReg41();

    virtual void StartDataTaking();
    virtual void StopDataTaking();

    // == TBM functions ======================================================

    virtual void Tbmenable(int on);
    virtual void ModAddr(int hub);
    virtual void TbmAddr(int hub, int port);
    virtual bool DataTriggerLevelScan();
    virtual int TbmWrite(const int hubAddr, const int addr, const int value);
    virtual int Tbm1write(const int hubAddr, const int registerAddress, const int value);
    virtual int Tbm2write(const int hubAddr, const int registerAddress, const int value);
    virtual bool GetTBMReg(int reg, int &value);

    // == ROC functions ======================================================

    virtual void SetChip(int chipId, int hubId, int portId, int aoutChipPosition);
    virtual void RocClrCal();
    virtual void RocSetDAC(int reg, int value);
    virtual void RocPixTrim(int col, int row, int value);
    virtual void RocPixMask(int col, int row);
    virtual void RocPixCal(int col, int row, int sensorcal);
    virtual void RocColEnable(int col, int on);

    virtual CTestboard *getCTestboard() {return cTestboard;}

    virtual int AoutLevel(int position, int nTriggers);
    virtual int AoutLevelChip(int position, int nTriggers, int trims[], int res[]);
    virtual int AoutLevelPartOfChip(int position, int nTriggers, int trims[], int res[], bool pxlFlags[]);
    virtual int ChipEfficiency(int nTriggers, int trim[], double res[]);
    virtual int MaskTest(short nTriggers, short res[]);
    virtual void DoubleColumnADCData(int doubleColumn, short data[], int readoutStop[]);
    virtual int ChipThreshold(int start, int step, int thrLevel, int nTrig, int dacReg, int xtalk, int cals, int trim[], int res[]);
    virtual int PixelThreshold(int col, int row, int start, int step, int thrLevel, int nTrig, int dacReg, int xtalk, int cals, int trim);
    virtual int SCurve(int nTrig, int dacReg, int threshold, int res[]);
    virtual int SCurveColumn(int column, int nTrig, int dacReg, int thr[], int trims[], int chipId[], int res[]);
    virtual void DacDac(int dac1, int dacRange1, int dac2, int dacRange2, int nTrig, int result[]);
    virtual void PHDac(int dac, int dacRange, int nTrig, int position, short result[]);
    virtual void AddressLevels(int position, int result[]);
    virtual void TBMAddressLevels(int result[]);
    virtual void TrimAboveNoise(short nTrigs, short thr, short mode, short result[]);

  // --------------------------------------------------------

    virtual void ProbeSelect(unsigned char port, unsigned char signal);


    virtual int demo(short x);



    virtual void ScanAdac(unsigned short chip, unsigned char dac, unsigned char min, unsigned char max, char step,unsigned char rep, unsigned int usDelay, unsigned char res[]);


    virtual void CdVc(unsigned short chip, unsigned char wbcmin, unsigned char wbcmax, unsigned char vcalstep, unsigned char cdinit, unsigned short &lres, unsigned short res[]);

    virtual char CountAllReadouts(int nTrig, int counts[], int amplitudes[]);
    virtual bool GetVersion(char *s, unsigned int n);

private:
    CTestboard *cTestboard;

    int TBMChannel;
    int emptyReadoutLength, emptyReadoutLengthADC, emptyReadoutLengthADCDual;
    bool tbmenable;

    // == data buffer ==============================================================
    static const int bufferSize = 2500000;
    int dataBuffer[bufferSize];
    int signalCounter, readPosition, writePosition;
    void ReadBackData();

    int triggerSource;  // 0 = local, 1 = extern
};
