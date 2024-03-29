/*!
 * \file RawPacketDecoder.h
 * \brief Definition of RawPacketDecoder class.
 */

#pragma once

#include <boost/shared_ptr.hpp>

#include "DecodedReadout.h"

class DecoderCalibrationModule;
struct DecodedReadoutModule;

namespace RawPacketDecoderConstants {
const int MAX_ROCS = 24;
}

/*!
 * \brief A class to decode a pixel data packet (TBM+ROCs).
 */
class RawPacketDecoder {
public:
    static RawPacketDecoder* Singleton();

    void SetCalibration(boost::shared_ptr<DecoderCalibrationModule> calibration) {
        fCalibration = calibration;
    }

    int decode(int dataLength, ADCword dataBuffer[], DecodedReadoutModule& module, int numROCs);

    int findTBMheader(int indexStart, int dataLength, ADCword dataBuffer[]) const;
    int findTBMtrailer(int indexStart, int dataLength, ADCword dataBuffer[]) const;
    int findROCheader(int rocId, int indexStart, int dataLength, ADCword dataBuffer[]) const;

    bool isBlackTBM(ADCword adcValue) const;
    bool isUltraBlackTBM(ADCword adcValue) const;
    bool isBlackROC(int rocId, ADCword adcValue) const;
    bool isUltraBlackROC(int rocId, ADCword adcValue) const;

protected:
    RawPacketDecoder();
    ~RawPacketDecoder(void);

    void Initialize();

    int decodeROCaddressLevel(int rocId, ADCword adcValue) const;
    int decodeTBMstatusLevel(ADCword adcValue) const;
    int decodeTBMheader(int indexStart, int dataLength, ADCword dataBuffer[], DecodedReadoutModule& module);
    int decodeROCsequence(int rocId, int indexStart, int indexStop, ADCword dataBuffer[], DecodedReadoutModule& module,
                          int numROCs);
    int decodeROCaddress(int rocId, ADCword rawADC[], unsigned& columnROC, unsigned& rowROC, unsigned& rawColumn,
                         unsigned& rawPixel) const;
    int decodeTBMtrailer(int indexStart, int dataLength, ADCword dataBuffer[], DecodedReadoutModule& module);
    int transformROCaddress2ModuleAddress(int columnROC, int rowROC, int rocId, int& columnModule,
                                          int& rowModule) const;

private:
    static RawPacketDecoder* fInstance;

    static const int fNumClocksTBMheader  = 8; // number of clock cycles for a TBM header
    static const int fNumClocksTBMtrailer = 8; // number of clock cycles for a TBM trailer
    static const int fNumClocksROCheader  = 3; // number of clock cycles for a ROC header
    static const int fNumClocksPixelHit   = 6; // number of clock cycles for each pixel hit

    static bool fPrintDebug;
    static bool fPrintWarning;
    static bool fPrintError;

    boost::shared_ptr<DecoderCalibrationModule> fCalibration;
};
