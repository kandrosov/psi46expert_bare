/*!
 * \file AnalogDecoding.cc
 * \brief Implementation of AnalogDecoding class.
 */

#include "psi/log.h"

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

AddressDecoding::AddressDecoding(TestRange *aTestRange, TBInterface *aTBInterface, bool debug)
{
    psi::LogDebug() << "[AddressDecoding] Initialization." << std::endl;

    testRange = aTestRange;
    tbInterface = aTBInterface;
    ReadTestParameters();
    gDecoder = RawPacketDecoder::Singleton();
    fdebug = debug;
}


void AddressDecoding::RocAction()
{
    map = GetMap("AddressDecoding");
    map->SetMaximum(1);
    map->SetMinimum(0);
    Test::RocAction();
    histograms->Add(map);
}


// void AddressDecoding::DoubleColumnAction()
// {
//   if (IncludesDoubleColumn())
//   {
//     roc->DoubleColumnADCData(doubleColumn->DoubleColumnNumber(), data, readoutStop);
//
//     for (int k = 0; k < 2*ROCNUMROWS; k++)
//     {
//       SetPixel(doubleColumn->GetPixel(k));
//       if (IncludesPixel())
//       {
//         AnalyseResult(k);
//       }
//     }
//   }
// }

void AddressDecoding::DoubleColumnAction()
{
    if (!fdebug) {
        if (IncludesDoubleColumn()) {
            roc->DoubleColumnADCData(doubleColumn->DoubleColumnNumber(), data, readoutStop);

            for (unsigned k = 0; k < 2 * psi::ROCNUMROWS; k++) {
                SetPixel(doubleColumn->GetPixel(k));
                if (IncludesPixel()) {
                    AnalyseResult(k);
                }
            }
        }
    } else if(true) {
        int twait = 1000;
        short data[psi::FIFOSIZE * 2];
        unsigned short nword;
        doubleColumn->EnableDoubleColumn();
        usleep(twait);
        Flush();
        usleep(twait);

        for (unsigned i = 0; i < psi::ROCNUMROWS * 2; i++) {

            SetPixel(doubleColumn->GetPixel(i));
            if (testRange->IncludesPixel(chipId, column, row)) {
                ArmPixel();
                ((TBAnalogInterface*)tbInterface)->ADCData(data, nword);
                DisarmPixel();
                Flush();
                if(nword < 25) {
                    psi::LogInfo() << "pixel " << column << "," << row << "  \033[31;49mNOT found\033[0m " << nword << std::endl;
                } else {
                    map->Fill(column, row);
                }
            }
        }
        doubleColumn->DisableDoubleColumn();
        usleep(twait);
        Flush();
    }// debug 1
    else if(false) {
        int twait = 1000;
        doubleColumn->EnableDoubleColumn();
        usleep(twait);
        Flush();
        usleep(twait);

        for (unsigned i = 0; i < psi::ROCNUMROWS * 2; i++) {
            SetPixel(doubleColumn->GetPixel(i));
            if (testRange->IncludesPixel(chipId, column, row)) {
                ArmPixel();
                int n = ((TBAnalogInterface*)tbInterface)->CountReadouts(1, 0);
                if(n == 0) {
                    psi::LogInfo() << "pixel " << column << "," << row << "  \033[31;49mNOT found\033[0m " <<  std::endl;
                } else {
                    map->Fill(column, row);
                }
                DisarmPixel();
                Flush();
            }
        }
        doubleColumn->DisableDoubleColumn();
        usleep(twait);
        Flush();
    }// debug 1
    else {
        int twait = 1000;
        int nReadouts, readoutStart[2 * psi::ROCNUMROWS];
        short data[psi::FIFOSIZE * 100];
        bool noError, pixelFound;
        doubleColumn->EnableDoubleColumn();
        usleep(twait);
        Flush();
        usleep(twait);
        int nTriggers = 1;
        DecodedReadoutModule decodedModuleReadout;
        const ConfigParameters& configParameters = ConfigParameters::Singleton();
        int nRocs = configParameters.NumberOfRocs();

        for (unsigned i = 0; i < psi::ROCNUMROWS * 2; i++) {
            SetPixel(doubleColumn->GetPixel(i));
            if (testRange->IncludesPixel(chipId, column, row)) {
                //PixelAction();
                //psi::LogInfo() << "AddressDecoding::DoubleColumnAction() i=" << i << " " << pixel->GetColumn() << " " << pixel->GetRow() << endl;
                ArmPixel();
                usleep(twait);
                Flush();
                usleep(twait);
                SendADCTrigs(nTriggers);
                usleep(twait);
                Flush();
                usleep(twait);
                //psi::LogInfo() << "calling GetADC " << nTriggers <<  " " << nReadouts << endl;
                noError = GetADC(data, psi::FIFOSIZE, count, nTriggers, readoutStart, nReadouts);
                //psi::LogInfo() << "back from  GetADC " << nTriggers <<  " " << nReadouts << " noerror= " << noError <<endl;
                if (!noError) {
                    psi::LogInfo() << "error reading pixel  column=" << column <<  " row=" <<  row <<  "nReadouts=" << nReadouts << " nTriggers= " << nTriggers << std::endl;
                    //char c;
                    //std::cin >> c;
                    DisarmPixel();
                    usleep(twait);
                    Flush();
                    usleep(twait);
                    continue;
                }
                if(!(nReadouts == 1)) {
                    psi::LogInfo() << "nReadouts=" << nReadouts << std::endl;
                }
                //psi::LogInfo() << "count = " << count << endl;

                for (int k = 0; k < nReadouts; k++) {
                    pixelFound = false;
                    int nDecodedPixelHitsModule = gDecoder->decode((int)count, &data[readoutStart[k]], decodedModuleReadout, nRocs);
                    if(!(nDecodedPixelHitsModule == 1)) {
                        psi::LogInfo() << "decoding  nhit=" << nDecodedPixelHitsModule << std::endl;
                    }
                    for(int iroc = 0; iroc < nRocs; iroc++) {
                        int nDecodedPixelHitsROC = decodedModuleReadout.roc[iroc].numPixelHits;
                        for (int ipixelhit = 0; ipixelhit < nDecodedPixelHitsROC; ipixelhit++) {
                            DecodedReadoutPixel decodedPixelHit = decodedModuleReadout.roc[iroc].pixelHit[ipixelhit];
                            if ((decodedPixelHit.rowROC == row) && (decodedPixelHit.columnROC == column)) {
                                pixelFound = true;
                            } else {
                                psi::LogInfo() << Form("unexpedted hit  column  %2d (%2d)  row  %2d (%2d) ", decodedPixelHit.columnROC, column, decodedPixelHit.rowROC, row) << std::endl;
                            }
                        }
                    }
                }
                if(pixelFound) {
                    map->Fill(column, row);
                    //psi::LogInfo() << "pixel " << column << ","<<row<<"  found" << endl;
                } else {
                    psi::LogInfo() << "pixel " << column << "," << row << "  \033[31;49mNOT found\033[0m " << std::endl;
                }
            }
            //char c;		cin << c;
            usleep(twait);
            DisarmPixel();
            usleep(twait);
            Flush();
            usleep(twait);

        }
        doubleColumn->DisableDoubleColumn();
        Flush();
    }// debug
}


void AddressDecoding::AnalyseResult(int pixel)
{
    unsigned readoutStart = 0, nDecodedPixels;
    DecodedReadoutModule decodedModuleReadout;
    if (pixel > 0) readoutStart = readoutStop[pixel - 1];

    const ConfigParameters& configParameters = ConfigParameters::Singleton();
    int nRocs = configParameters.NumberOfRocs();

    if (readoutStop[pixel] - readoutStart == ((TBAnalogInterface*)tbInterface)->GetEmptyReadoutLengthADC() + 6) {
        nDecodedPixels = gDecoder->decode( readoutStop[pixel] - readoutStart,
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
                       << column << ", " << row
                       << ") on ROC" << roc->GetChipId() << '.' << std::endl;

        if( (readoutStop[pixel] - readoutStart) ==
                dynamic_cast<TBAnalogInterface *>( tbInterface)->GetEmptyReadoutLengthADC() ) {
            psi::LogDebug() << "[AddressDecoding] Pixel seems to be dead."
                            << std::endl;
        }

        else if( (readoutStop[pixel] - readoutStart) !=
                 (dynamic_cast<TBAnalogInterface *>( tbInterface)->GetEmptyReadoutLengthADC() + 6) ) {
            psi::LogDebug() << "[AddressDecoding] Pixel has a wrong length ("
                            << readoutStop[pixel] - readoutStart
                            << ") of read-out signal. Expected length is "
                            << ( dynamic_cast<TBAnalogInterface *>( tbInterface)->GetEmptyReadoutLengthADC() + 6)
                            << '.' << std::endl;
        }
    } else if (nDecodedPixels == 0 || decodedModuleReadout.roc[roc->GetAoutChipPosition()].numPixelHits == 0) {
        psi::LogInfo() << "[AddressDecoding] Error: No address levels were found "
                       << "for Pixel( " << column << ", " << row
                       << ") on ROC" << roc->GetChipId() << '.' << std::endl;
    } else if (nDecodedPixels > 1) {
        psi::LogInfo() << "[AddressDecoding] Error: Too many address levels were "
                       << "found for Pixel( " << column << ", " << row
                       << ") on ROC" << roc->GetChipId() << '.' << std::endl;
    } else if (row != decodedModuleReadout.roc[roc->GetAoutChipPosition()].pixelHit[0].rowROC) {
        psi::LogInfo() << "[AddressDecoding] Error: wrong row "
                       << decodedModuleReadout.roc[roc->GetChipId()].pixelHit[0].rowROC
                       << " for Pixel( " << column << ", " << row
                       << ") on ROC" << roc->GetChipId() << '.' << std::endl;
    } else if (column != decodedModuleReadout.roc[roc->GetAoutChipPosition()].pixelHit[0].columnROC) {
        psi::LogInfo() << "[AddressDecoding] Error: wrong column "
                       << decodedModuleReadout.roc[roc->GetAoutChipPosition()].pixelHit[0].columnROC
                       << " for Pixel( " << column << ", " << row
                       << ") on ROC" << roc->GetChipId() << '.' << std::endl;
    } else map->Fill(column, row);
}
