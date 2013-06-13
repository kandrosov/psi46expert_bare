/*!
 * \file AnalogDecoding.cc
 * \brief Implementation of AnalogDecoding class.
 */

#include "psi/log.h"
#include "psi/date_time.h"

#include "AddressDecoding.h"
#include "psi46expert/TestRoc.h"
#include "BasePixel/constants.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/RawPacketDecoder.h"
#include "BasePixel/DecoderCalibration.h"

// bool AddressDecoding::fPrintDebug = true;
bool AddressDecoding::fPrintDebug = false;

using namespace RawPacketDecoderConstants;
using namespace DecoderCalibrationConstants;
using namespace DecodedReadoutConstants;

AddressDecoding::AddressDecoding(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface, bool debug,
                                 unsigned _maxNumberOfTry)
    : Test("AddressDecoding", testRange), tbInterface(aTBInterface), fdebug(debug), maxNumberOfTry(_maxNumberOfTry) {}

void AddressDecoding::RocAction(TestRoc& roc)
{
    map = CreateMap("AddressDecoding", roc.GetChipId());
    map->SetMaximum(1);
    map->SetMinimum(0);

    tryMap = CreateMap("AddressDecodingTry", roc.GetChipId());
    tryMap->SetMaximum(maxNumberOfTry + 1);
    tryMap->SetMinimum(0);

    Test::RocAction(roc);
    histograms->Add(map);
    histograms->Add(tryMap);
}

void AddressDecoding::DoubleColumnAction(TestDoubleColumn& doubleColumn)
{
    if (!fdebug) {
        if (doubleColumn.IsIncluded(testRange)) {
            doubleColumn.ADCData(data, readoutStop);

            for (unsigned k = 0; k < 2 * psi::ROCNUMROWS; k++) {
                TestPixel& pixel = doubleColumn.GetPixel(k);
                if (pixel.IsIncluded(testRange))
                    AnalyseResult(k, pixel);
            }
        }
    } else {
        psi::Time twait = 1000 * psi::micro * psi::seconds;
        short data[psi::FIFOSIZE * 2];
        unsigned short nword;
        doubleColumn.EnableDoubleColumn();
        psi::Sleep(twait);
        tbInterface->Flush();
        psi::Sleep(twait);

        for (unsigned i = 0; i < psi::ROCNUMROWS * 2; i++) {
            TestPixel& pixel = doubleColumn.GetPixel(i);
            if (pixel.IsIncluded(testRange)) {
                unsigned tryNumber = 0;
                for(; tryNumber < maxNumberOfTry; ++tryNumber) {
                    pixel.ArmPixel();
                    tbInterface->ADCData(data, nword);
                    pixel.DisarmPixel();
                    tbInterface->Flush();
                    if(AnalyseResultDebug(pixel, data, nword))
                        break;
                }
                if ( tryNumber == maxNumberOfTry ) {
                    psi::LogInfo() << "Pixel seems to be dead ( "
                                           << pixel.GetColumn() << ", " << pixel.GetRow()
                                           << ") on ROC" << pixel.GetRoc().GetChipId() << '.' << std::endl;
                }

                for(unsigned n = 0; n <= tryNumber; ++n)
                    tryMap->Fill(pixel.GetColumn(), pixel.GetRow());
            }
        }
        doubleColumn.DisableDoubleColumn();
        psi::Sleep(twait);
        tbInterface->Flush();
    }
}

void AddressDecoding::AnalyseResult(int pixel, TestPixel& testPixel)
{
    unsigned readoutStart = 0, nDecodedPixels;
    DecodedReadoutModule decodedModuleReadout;
    if (pixel > 0) readoutStart = readoutStop[pixel - 1];

    const ConfigParameters& configParameters = ConfigParameters::Singleton();
    int nRocs = configParameters.NumberOfRocs();

    if (readoutStop[pixel] - readoutStart == tbInterface->GetEmptyReadoutLengthADC() + 6) {
        nDecodedPixels = RawPacketDecoder::Singleton()->decode( readoutStop[pixel] - readoutStart,
                                           &data[readoutStart],
                                           decodedModuleReadout,
                                           nRocs);

    } else {
        if ( fPrintDebug ) {
            psi::LogInfo() << "ADC values = { ";
            for ( unsigned ivalue = readoutStart; ivalue < readoutStop[pixel]; ivalue++ ) {
                psi::LogInfo() << data[ivalue] << " ";
            }
            psi::LogInfo() << "}" << std::endl;
        }

        nDecodedPixels = -1;
    }

    if (nDecodedPixels < 0) {
        psi::LogInfo() << "[AddressDecoding] Error: Decoding Error for Pixel( "
                       << testPixel.GetColumn() << ", " << testPixel.GetRow()
                       << ") on ROC" << testPixel.GetRoc().GetChipId() << '.' << std::endl;

        if( (readoutStop[pixel] - readoutStart) == tbInterface->GetEmptyReadoutLengthADC() ) {
            psi::LogDebug() << "[AddressDecoding] Pixel seems to be dead."
                            << std::endl;
        }

        else if( (readoutStop[pixel] - readoutStart) != (tbInterface->GetEmptyReadoutLengthADC() + 6) ) {
            psi::LogDebug() << "[AddressDecoding] Pixel has a wrong length ("
                            << readoutStop[pixel] - readoutStart
                            << ") of read-out signal. Expected length is "
                            << (tbInterface->GetEmptyReadoutLengthADC() + 6)
                            << '.' << std::endl;
        }
    } else if (nDecodedPixels == 0 ||
               decodedModuleReadout.roc[testPixel.GetRoc().GetAoutChipPosition()].numPixelHits == 0) {
        psi::LogInfo() << "[AddressDecoding] Error: No address levels were found "
                       << "for Pixel( " << testPixel.GetColumn() << ", " << testPixel.GetRow()
                       << ") on ROC" << testPixel.GetRoc().GetChipId() << '.' << std::endl;
    } else if (nDecodedPixels > 1) {
        psi::LogInfo() << "[AddressDecoding] Error: Too many address levels were "
                       << "found for Pixel( " << testPixel.GetColumn() << ", " << testPixel.GetRow()
                       << ") on ROC" << testPixel.GetRoc().GetChipId() << '.' << std::endl;
    } else if (testPixel.GetRow() != decodedModuleReadout.roc[testPixel.GetRoc().GetAoutChipPosition()].pixelHit[0].rowROC) {
        psi::LogInfo() << "[AddressDecoding] Error: wrong row "
                       << decodedModuleReadout.roc[testPixel.GetRoc().GetChipId()].pixelHit[0].rowROC
                       << " for Pixel( " << testPixel.GetColumn() << ", " << testPixel.GetRow()
                       << ") on ROC" << testPixel.GetRoc().GetChipId() << '.' << std::endl;
    } else if (testPixel.GetColumn() != decodedModuleReadout.roc[testPixel.GetRoc().GetAoutChipPosition()].pixelHit[0].columnROC) {
        psi::LogInfo() << "[AddressDecoding] Error: wrong column "
                       << decodedModuleReadout.roc[testPixel.GetRoc().GetAoutChipPosition()].pixelHit[0].columnROC
                       << " for Pixel( " << testPixel.GetColumn() << ", " << testPixel.GetRow()
                       << ") on ROC" << testPixel.GetRoc().GetChipId() << '.' << std::endl;
    } else map->Fill(testPixel.GetColumn(), testPixel.GetRow());
}

bool AddressDecoding::AnalyseResultDebug(TestPixel& testPixel, short *data, unsigned nword)
{
    int nDecodedPixels;
    DecodedReadoutModule decodedModuleReadout;

    const ConfigParameters& configParameters = ConfigParameters::Singleton();
    const unsigned nRocs = configParameters.NumberOfRocs();

    if (nword == tbInterface->GetEmptyReadoutLengthADC() + 6) {
        nDecodedPixels = RawPacketDecoder::Singleton()->decode( nword, data, decodedModuleReadout, nRocs);

    } else {
        if(nword == tbInterface->GetEmptyReadoutLengthADC())
            return false;
        nDecodedPixels = -1;
    }

    if (nDecodedPixels < 0) {
        psi::LogInfo() << "[AddressDecoding] Error: Decoding Error for Pixel( "
                       << testPixel.GetColumn() << ", " << testPixel.GetRow()
                       << ") on ROC" << testPixel.GetRoc().GetChipId() << '.' << std::endl;

        if( nword == tbInterface->GetEmptyReadoutLengthADC() ) {
            psi::LogDebug() << "[AddressDecoding] Pixel seems to be dead."
                            << std::endl;
        }

        else if( nword != (tbInterface->GetEmptyReadoutLengthADC() + 6) ) {
            psi::LogDebug() << "[AddressDecoding] Pixel has a wrong length ("
                            << nword
                            << ") of read-out signal. Expected length is "
                            << (tbInterface->GetEmptyReadoutLengthADC() + 6)
                            << '.' << std::endl;
        }
    } else if (nDecodedPixels == 0 ||
               decodedModuleReadout.roc[testPixel.GetRoc().GetAoutChipPosition()].numPixelHits == 0) {
        psi::LogInfo() << "[AddressDecoding] Error: No address levels were found "
                       << "for Pixel( " << testPixel.GetColumn() << ", " << testPixel.GetRow()
                       << ") on ROC" << testPixel.GetRoc().GetChipId() << '.' << std::endl;
    } else if (nDecodedPixels > 1) {
        psi::LogInfo() << "[AddressDecoding] Error: Too many address levels were "
                       << "found for Pixel( " << testPixel.GetColumn() << ", " << testPixel.GetRow()
                       << ") on ROC" << testPixel.GetRoc().GetChipId() << '.' << std::endl;
    } else if (testPixel.GetRow() != decodedModuleReadout.roc[testPixel.GetRoc().GetAoutChipPosition()].pixelHit[0].rowROC) {
        psi::LogInfo() << "[AddressDecoding] Error: wrong row "
                       << decodedModuleReadout.roc[testPixel.GetRoc().GetChipId()].pixelHit[0].rowROC
                       << " for Pixel( " << testPixel.GetColumn() << ", " << testPixel.GetRow()
                       << ") on ROC" << testPixel.GetRoc().GetChipId() << '.' << std::endl;
    } else if (testPixel.GetColumn() != decodedModuleReadout.roc[testPixel.GetRoc().GetAoutChipPosition()].pixelHit[0].columnROC) {
        psi::LogInfo() << "[AddressDecoding] Error: wrong column "
                       << decodedModuleReadout.roc[testPixel.GetRoc().GetAoutChipPosition()].pixelHit[0].columnROC
                       << " for Pixel( " << testPixel.GetColumn() << ", " << testPixel.GetRow()
                       << ") on ROC" << testPixel.GetRoc().GetChipId() << '.' << std::endl;
    } else map->Fill(testPixel.GetColumn(), testPixel.GetRow());
    return true;
}
