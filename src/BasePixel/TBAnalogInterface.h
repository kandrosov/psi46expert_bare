/*!
 * \file TBAnalogInterface.h
 * \brief Definition of TBAnalogInterface class.
 *
 * \b Changelog
 * 28-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - All functionality extracted from TBAnalogInterface class to AnalogTestBoard class. TBAnalogInterface is now
 *        abstract.
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using definitions from PsiCommon.h.
 * 15-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using boost::units::quantity to represent physical values.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new ConfigParameters class definition.
 */

#pragma once

#include "BasePixel/TBInterface.h"
#include "BasePixel/ConfigParameters.h"
#include "BasePixel/psi46_tb.h"
#include "psi/units.h"

/*!
 * Abstract class that defines interface to program the analog testboard.
 */
class TBAnalogInterface: public TBInterface
{
public:

    // == General functions ================================================
    
    virtual void SetReg(int reg, int value) = 0;
    virtual void Clear() = 0;
    virtual int CountReadouts(int count, int chipId) = 0;
    virtual void SingleCal() = 0;
    virtual void SendCal(int nTrig) = 0;
    virtual void SetEmptyReadoutLength(int length) = 0;
    virtual int GetEmptyReadoutLength() = 0;
    virtual void SetEmptyReadoutLengthADC(int length) = 0;
    virtual int GetEmptyReadoutLengthADC() = 0;
    virtual void SetEmptyReadoutLengthADCDual(int length) = 0;
    virtual int GetEmptyReadoutLengthADCDual() = 0;
    virtual void SetEnableAll(int value) = 0;
    virtual unsigned short GetModRoCnt(unsigned short index) = 0;

    // == Analog functions =================================================
    
    virtual void SetClock(int mhz) = 0;
    virtual void DataCtrl(bool clear, bool trigger, bool cont = false) = 0;
    virtual void DataEnable(bool on) = 0;
    virtual bool DataRead(short buffer[], unsigned short buffersize, unsigned short &wordsread) = 0;
    virtual void SetDelay(int signal, int ns) = 0;
    virtual void SetClockStretch(unsigned char src, unsigned short delay, unsigned short width) = 0;
    virtual bool SendRoCnt() = 0;
    virtual int  RecvRoCnt() = 0;
    virtual void DataTriggerLevel(int level) = 0;
    virtual void SetTriggerMode(unsigned short mode) = 0;
    virtual void SetTBMChannel(int channel) = 0;
    virtual int GetTBMChannel() = 0;
    virtual bool TBMPresent() = 0;

    virtual void ADCRead(short buffer[], unsigned short &wordsread, short nTrig = 1) = 0;
    virtual bool ADCData(short buffer[], unsigned short &wordsread) = 0;
    virtual unsigned short ADC(int nbsize) = 0;
    virtual unsigned short ADC() = 0;
    virtual void SendADCTrigs(int nTrig) = 0;
    virtual void SendADCTrigsNoReset(int nTrig) = 0;
    virtual bool GetADC(short buffer[], unsigned short buffersize, unsigned short &wordsread, int nTrig, int startBuffer[], int &nReadouts) = 0;
    virtual int LastDAC(int nTrig, int chipId) = 0;
    
    virtual void SetVA(psi::ElectricPotential V) = 0;   // set VA voltage in V
    virtual void SetIA(psi::ElectricCurrent A) = 0;   // set VA current limit in A
    virtual void SetVD(psi::ElectricPotential V) = 0;   // set VD voltage in V
    virtual void SetID(psi::ElectricCurrent A) = 0;   // set VD current limit in A

    virtual psi::ElectricPotential GetVA() = 0; // get VA voltage in V
    virtual psi::ElectricCurrent GetIA() = 0; // get VA current in A
    virtual psi::ElectricPotential GetVD() = 0; // get VD voltage in V
    virtual psi::ElectricCurrent GetID() = 0; // get VD current in A

    virtual void HVon() = 0;    // switch HV relais on
    virtual void HVoff() = 0; // switch HV relais off

    virtual void ResetOn() = 0; // switch RESET-line to reset state (low)
    virtual void ResetOff() = 0;  // switch RESET-line to not reset state (high)
    virtual int CountADCReadouts(int count) = 0;

    virtual bool Mem_ReadOut(FILE *file, unsigned int addr, unsigned int size) = 0;
    
    virtual void SetReg41() = 0;

    virtual void StartDataTaking() = 0;
    virtual void StopDataTaking() = 0;
    
    // == TBM functions ======================================================

    virtual void ModAddr(int hub) = 0;
    virtual void TbmAddr(int hub, int port) = 0;
    virtual bool DataTriggerLevelScan() = 0;
    virtual int TbmWrite(const int hubAddr, const int addr, const int value) = 0;
    virtual bool GetTBMReg(int reg, int &value) = 0;

    // == ROC functions ======================================================

    virtual void SetChip(int chipId, int hubId, int portId, int aoutChipPosition) = 0;
    virtual void RocClrCal() = 0;
    virtual void RocSetDAC(int reg, int value) = 0;
    virtual void RocPixTrim(int col, int row, int value) = 0;
    virtual void RocPixMask(int col, int row) = 0;
    virtual void RocPixCal(int col, int row, int sensorcal) = 0;
    virtual void RocColEnable(int col, int on) = 0;

    virtual CTestboard *getCTestboard() = 0;
    
    virtual int AoutLevel(int position, int nTriggers) = 0;
    virtual int AoutLevelChip(int position, int nTriggers, int trims[], int res[]) = 0;
    virtual int AoutLevelPartOfChip(int position, int nTriggers, int trims[], int res[], bool pxlFlags[]) = 0;
    virtual int ChipEfficiency(int nTriggers, int trim[], double res[]) = 0;
    virtual int MaskTest(short nTriggers, short res[]) = 0;
    virtual void DoubleColumnADCData(int doubleColumn, short data[], int readoutStop[]) = 0;
    virtual int ChipThreshold(int start, int step, int thrLevel, int nTrig, int dacReg, int xtalk, int cals, int trim[], int res[]) = 0;
    virtual int PixelThreshold(int col, int row, int start, int step, int thrLevel, int nTrig, int dacReg, int xtalk, int cals, int trim) = 0;
    virtual int SCurve(int nTrig, int dacReg, int threshold, int res[]) = 0;
    virtual int SCurveColumn(int column, int nTrig, int dacReg, int thr[], int trims[], int chipId[], int res[]) = 0;
    virtual void DacDac(int dac1, int dacRange1, int dac2, int dacRange2, int nTrig, int result[]) = 0;
    virtual void PHDac(int dac, int dacRange, int nTrig, int position, short result[]) = 0;
    virtual void AddressLevels(int position, int result[]) = 0;
    virtual void TBMAddressLevels(int result[]) = 0;
    virtual void TrimAboveNoise(short nTrigs, short thr, short mode, short result[]) = 0;
    
  // --------------------------------------------------------

    virtual void ProbeSelect(unsigned char port, unsigned char signal) = 0;


    virtual int demo(short x) = 0;



    virtual void ScanAdac(unsigned short chip, unsigned char dac, unsigned char min, unsigned char max, char step,unsigned char rep, unsigned int usDelay, unsigned char res[]) = 0;
    
    
    virtual void CdVc(unsigned short chip, unsigned char wbcmin, unsigned char wbcmax, unsigned char vcalstep, unsigned char cdinit, unsigned short &lres, unsigned short res[]) = 0;
    
    virtual char CountAllReadouts(int nTrig, int counts[], int amplitudes[]) = 0;
    virtual bool GetVersion(char *s, unsigned int n) = 0;

};
