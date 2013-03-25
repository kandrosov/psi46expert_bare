/*!
 * \file FakeTestBoard.h
 * \brief Definition of FakeTestBoard class.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 09-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Corrected questionable language constructions, which was found using -Wall g++ option.
 * 01-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Class SysCommand removed.
 * 28-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#pragma once

#include "TBAnalogInterface.h"

/*!
 * \brief Provides fake TBInterface implementation for the debug purposes.
 */
class FakeTestBoard : public TBAnalogInterface {
public:
    FakeTestBoard() {}

    virtual void Pon() {}
    virtual void Poff() {}
    virtual void Set( int reg, int value) {}
    virtual void Single( int mask) {}
    virtual void Intern( int mask) {}
    virtual void Extern( int mask) {}
    virtual int GetRoCnt() {
        return 0;
    }
    virtual void Initialize() {}
    virtual int Startup( int port) {
        return 0;
    }
    virtual void Cleanup() {}
    virtual int Present() {
        return 1;
    }
    virtual void I2cAddr( int id) {}
    virtual int IsPresent() {
        return 1;
    }
    virtual void Flush() {}
    virtual void CDelay( unsigned int n) {}
    virtual bool IsAnalogTB() {
        return true;
    }

    virtual void Tbmenable( int on) {}
    virtual int Tbm1write( const int hubAddr, const int registerAddress, const int value) {
        return 0;
    }
    virtual int Tbm2write( const int hubAddr, const int registerAddress, const int value) {
        return 0;
    }

    virtual void SetReg(int reg, int value) {}
    virtual void Clear() {}
    virtual int CountReadouts(int count, int chipId) {
        return 0;
    }
    virtual void SingleCal() {}
    virtual void SendCal(int nTrig) {}
    virtual void SetEmptyReadoutLength(int length) {}
    virtual int GetEmptyReadoutLength() {
        return 0;
    }
    virtual void SetEmptyReadoutLengthADC(int length) {}
    virtual unsigned GetEmptyReadoutLengthADC() {
        return 0;
    }
    virtual void SetEmptyReadoutLengthADCDual(int length) {}
    virtual int GetEmptyReadoutLengthADCDual() {
        return 0;
    }
    virtual void SetEnableAll(int value) {}
    virtual unsigned short GetModRoCnt(unsigned short index) {
        return 0;
    }

    // == Analog functions =================================================

    virtual void SetClock(int mhz) {}
    virtual void DataCtrl(bool clear, bool trigger, bool cont = false) {}
    virtual void DataEnable(bool on) {}
    virtual bool DataRead(short buffer[], unsigned short buffersize, unsigned short &wordsread) {
        return false;
    }
    virtual void SetDelay(int signal, int ns) {}
    virtual void SetClockStretch(unsigned char src, unsigned short delay, unsigned short width) {}
    virtual bool SendRoCnt() {
        return false;
    }
    virtual int  RecvRoCnt() {
        return 0;
    }
    virtual void DataTriggerLevel(int level) {}
    virtual void SetTriggerMode(unsigned short mode) {}
    virtual void SetTBMChannel(int channel) {}
    virtual int GetTBMChannel() {
        return 0;
    }
    virtual bool TBMPresent() {
        return false;
    }

    virtual void ADCRead(short buffer[], unsigned short &wordsread, short nTrig = 1) {}
    virtual bool ADCData(short buffer[], unsigned short &wordsread) {
        return false;
    }
    virtual unsigned short ADC(int nbsize) {
        return 0;
    }
    virtual unsigned short ADC() {
        return 0;
    }
    virtual void SendADCTrigs(int nTrig) {}
    virtual void SendADCTrigsNoReset(int nTrig) {}
    virtual bool GetADC(short buffer[], unsigned short buffersize, unsigned short &wordsread, int nTrig,
                        int startBuffer[], int &nReadouts) {
        return false;
    }
    virtual int LastDAC(int nTrig, int chipId) {
        return 0;
    }

    virtual void SetVA(psi::ElectricPotential V) {}
    virtual void SetIA(psi::ElectricCurrent A) {}
    virtual void SetVD(psi::ElectricPotential V) {}
    virtual void SetID(psi::ElectricCurrent A) {}

    virtual psi::ElectricPotential GetVA() {
        return 0.0 * psi::volts;
    }
    virtual psi::ElectricCurrent GetIA() {
        return 0.02 * psi::amperes;
    }
    virtual psi::ElectricPotential GetVD() {
        return 0.0 * psi::volts;
    }
    virtual psi::ElectricCurrent GetID() {
        return 0.02 * psi::amperes;
    }

    virtual void HVon() {}
    virtual void HVoff() {}

    virtual void ResetOn() {}
    virtual void ResetOff() {}
    virtual int CountADCReadouts(int count) {
        return 0;
    }

    virtual void SetReg41() {}

    virtual void StartDataTaking() {}
    virtual void StopDataTaking() {}

    virtual void ModAddr(int hub) {}
    virtual void TbmAddr(int hub, int port) {}
    virtual bool DataTriggerLevelScan() {
        return false;
    }
    virtual int TbmWrite(const int hubAddr, const int addr, const int value) {
        return 0;
    }
    virtual bool GetTBMReg(int reg, int &value) {
        return false;
    }

    virtual void SetChip(int chipId, int hubId, int portId, int aoutChipPosition) {}
    virtual void RocClrCal() {}
    virtual void RocSetDAC(int reg, int value) {}
    virtual void RocPixTrim(int col, int row, int value) {}
    virtual void RocPixMask(int col, int row) {}
    virtual void RocPixCal(int col, int row, int sensorcal) {}
    virtual void RocColEnable(int col, int on) {}

    virtual CTestboard *getCTestboard() {
        return 0;
    }

    virtual int AoutLevel(int position, int nTriggers) {
        return 0;
    }
    virtual int AoutLevelChip(int position, int nTriggers, int trims[], int res[]) {
        return 0;
    }
    virtual int AoutLevelPartOfChip(int position, int nTriggers, int trims[], int res[],
                                    bool pxlFlags[]) {
        return 0;
    }
    virtual int ChipEfficiency(int nTriggers, int trim[], double res[]) {
        return 0;
    }
    virtual int MaskTest(short nTriggers, short res[]) {
        return 0;
    }
    virtual void DoubleColumnADCData(int doubleColumn, short data[], unsigned readoutStop[]) {}
    virtual int ChipThreshold(int start, int step, int thrLevel, int nTrig, int dacReg, int xtalk, int cals,
                              int trim[], int res[]) {
        return 0;
    }
    virtual int PixelThreshold(int col, int row, int start, int step, int thrLevel, int nTrig, int dacReg, int xtalk,
                               int cals, int trim) {
        return 0;
    }
    virtual int SCurve(int nTrig, int dacReg, int threshold, int res[]) {
        return 0;
    }
    virtual int SCurveColumn(int column, int nTrig, int dacReg, int thr[], int trims[], int chipId[],
                             int res[]) {
        return 0;
    }
    virtual void DacDac(int dac1, int dacRange1, int dac2, int dacRange2, int nTrig, int result[]) {}
    virtual void PHDac(int dac, int dacRange, int nTrig, int position, short result[]) {}
    virtual void AddressLevels(int position, int result[]) {}
    virtual void TBMAddressLevels(int result[]) {}
    virtual void TrimAboveNoise(short nTrigs, short thr, short mode, short result[]) {}

    virtual void ProbeSelect(unsigned char port, unsigned char signal) {}


    virtual int demo(short x) {
        return 0;
    }

    virtual void ScanAdac(unsigned short chip, unsigned char dac, unsigned char min, unsigned char max, char step,
                          unsigned char rep, unsigned int usDelay, unsigned char res[]) {}


    virtual void CdVc(unsigned short chip, unsigned char wbcmin, unsigned char wbcmax, unsigned char vcalstep,
                      unsigned char cdinit, unsigned short &lres, unsigned short res[]) {}

    virtual char CountAllReadouts(int nTrig, int counts[], int amplitudes[]) {
        return 0;
    }
    virtual bool GetVersion(char *s, unsigned int n) {
        return false;
    }
};
