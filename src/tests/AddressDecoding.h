/*!
 * \file AddressDecoding.h
 * \brief Definition of AddressDecoding class.
 *
 * \b Changelog
 * 09-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Corrected questionable language constructions, which was found using -Wall g++ option.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#pragma once

#include "BasePixel/Test.h"

class RawPacketDecoder;

/*!
 * \brief Pixel alive test, analog testboard version.
 */
class AddressDecoding : public Test {

public:
    AddressDecoding(TestRange *testRange, TBInterface *aTBInterface, bool debug = false);

    virtual void RocAction();
    virtual void DoubleColumnAction();
    void AnalyseResult(int pixel);

protected:

    TH2D *map;

    unsigned readoutStop[2 * psi::ROCNUMROWS];
    short data[20000];

    RawPacketDecoder *gDecoder;
    unsigned short count;

    static bool fPrintDebug;
    bool fdebug;
};
