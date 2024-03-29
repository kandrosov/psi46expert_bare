/*!
 * \file DecodedReadout.h
 * \brief Definition of classes related to readout decoding.
 *
 * Structures to store the data decoded from a raw pixel data packet
 * (Modules, [Plaquettes, to be implemented ?], TBMs and ROCs)
 *
 * \author Christian Veelken (UC Davis), 06/01/06
 */

#pragma once


//--- these typedefs allow the DecodeRawPacket class
//    to be used in different contexts:
//   --> short for use with PSI46 testboard
//       long for use with spy-data of the Front-End-Driver of the final CMS Pixel system
typedef short ADCword;
//typedef long  ADCword;

namespace DecodedReadoutConstants {
const int NUM_ROCSMODULE =   16; // number of ROCs on a module
const int MAX_PIXELSROC  = 1000; // maximum number of pixel hits in one and the same ROC
}

struct DecodedReadoutTBM {
    int  tbmEventCounter;
    bool tbmErrorStatus[8];

    ADCword rawTBMheader[4];
    ADCword rawTBMtrailer[4];
};

struct DecodedReadoutPixel {
    unsigned rocId;

    unsigned columnROC;
    unsigned rowROC;

    int analogPulseHeight;

    unsigned columnModule;
    unsigned rowModule;

    ADCword rawADC[6];
};

struct DecodedReadoutROC {
    int lastDac;

    struct DecodedReadoutPixel pixelHit[DecodedReadoutConstants::MAX_PIXELSROC];
    int numPixelHits;
};

struct DecodedReadoutModule {
    struct DecodedReadoutTBM tbm;
    struct DecodedReadoutROC roc[DecodedReadoutConstants::NUM_ROCSMODULE];
};
