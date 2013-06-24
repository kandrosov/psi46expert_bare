/*!
 * \file AddressDecoding.h
 * \brief Definition of AddressDecoding class.
 */

#pragma once

#include "BasePixel/constants.h"
#include "BasePixel/Test.h"

class RawPacketDecoder;

/*!
 * \brief Pixel alive test, analog testboard version.
 */
class AddressDecoding : public Test {

public:
    AddressDecoding(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface, bool debug = false,
                    unsigned _maxNumberOfTry = 1);

    virtual void RocAction(TestRoc& roc);
    virtual void DoubleColumnAction(TestDoubleColumn& doubleColumn);


private:
    void AnalyseResult(int pixel, TestPixel& testPixel);
    bool AnalyseResultDebug(TestPixel& testPixel, short *data, unsigned nword);

    boost::shared_ptr<TBAnalogInterface> tbInterface;
    TH2D *map, *firstTryMap;
    unsigned readoutStop[2 * psi::ROCNUMROWS];
    short data[20000];

    unsigned short count;

    static bool fPrintDebug;
    bool fdebug;
    unsigned maxNumberOfTry;
};
