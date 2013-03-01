/*!
 * \file AnalogDecoding.cc
 * \brief Implementation of AnalogDecoding class.
 *
 * \b Changelog
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using definitions from PsiCommon.h.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new ConfigParameters class definition.
 *      - Adaptation for the new TestParameters class definition.
 */

#include "interface/Log.h"

#include "AddressDecoding.h"
#include "TestRoc.h"
#include "BasePixel/PsiCommon.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/RawPacketDecoder.h"
#include "BasePixel/DecoderCalibration.h"

// bool AddressDecoding::fPrintDebug = true;
bool AddressDecoding::fPrintDebug = false;

using namespace RawPacketDecoderConstants;
using namespace DecoderCalibrationConstants;
using namespace DecodedReadoutConstants;

AddressDecoding::AddressDecoding(TestRange *aTestRange, TBInterface *aTBInterface,bool debug)
{
  psi::Log<psi::Debug>() << "[AddressDecoding] Initialization." << std::endl;

  testRange = aTestRange;
  tbInterface = aTBInterface;
  ReadTestParameters();
  gDecoder = RawPacketDecoder::Singleton();
	fdebug=debug;
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
  if (!fdebug)
  {
    if (IncludesDoubleColumn())
    {
      roc->DoubleColumnADCData(doubleColumn->DoubleColumnNumber(), data, readoutStop);
      
      for (int k = 0; k < 2*psi::ROCNUMROWS; k++)
      {
        SetPixel(doubleColumn->GetPixel(k));
        if (IncludesPixel()) 
        {
          AnalyseResult(k);
        }
      }
    }
  } else if(true) {
    int twait=1000;
    short data[psi::FIFOSIZE*2];
    unsigned short nword;
    doubleColumn->EnableDoubleColumn();usleep(twait);
    Flush(); usleep(twait);
    
    for (int i = 0; i < psi::ROCNUMROWS*2; i++)
      {

	SetPixel(doubleColumn->GetPixel(i));
	if (testRange->IncludesPixel(chipId, column, row)) {
	  ArmPixel();
	  ((TBAnalogInterface*)tbInterface)->ADCData(data, nword);
	  DisarmPixel();
	  Flush();
	  if(nword<25){
        std::cout << "pixel " << column << ","<<row<<"  \033[31;49mNOT found\033[0m " << nword << std::endl;
	  }else{
	    map->Fill(column, row);
	  }
	}	
      }
    doubleColumn->DisableDoubleColumn();usleep(twait);
    Flush();
  }// debug 1
  else if(false){
    int twait=1000;
   doubleColumn->EnableDoubleColumn();usleep(twait);
    Flush(); usleep(twait);
    
    for (int i = 0; i < psi::ROCNUMROWS*2; i++)
      {
	SetPixel(doubleColumn->GetPixel(i));
	if (testRange->IncludesPixel(chipId, column, row)) {
	  ArmPixel();
	  int n= ((TBAnalogInterface*)tbInterface)->CountReadouts(1,0);
	  if(n==0){
        std::cout << "pixel " << column << ","<<row<<"  \033[31;49mNOT found\033[0m " <<  std::endl;
	  }else{
	    map->Fill(column, row);
	  }
	  DisarmPixel();
	  Flush();
	}	
      }
    doubleColumn->DisableDoubleColumn();usleep(twait);
    Flush();
  }// debug 1
  else
    {
    int twait=1000;
    int nReadouts, readoutStart[2*psi::ROCNUMROWS];
    short data[psi::FIFOSIZE*100];
    bool noError, pixelFound;
    doubleColumn->EnableDoubleColumn();usleep(twait);
    Flush(); usleep(twait);
    int nTriggers=1;
    DecodedReadoutModule decodedModuleReadout;
    const ConfigParameters& configParameters = ConfigParameters::Singleton();
    int nRocs = configParameters.NumberOfRocs();
    
    for (int i = 0; i < psi::ROCNUMROWS*2; i++)
      {
	SetPixel(doubleColumn->GetPixel(i));
	if (testRange->IncludesPixel(chipId, column, row)) {
	  //PixelAction();
	  //cout << "AddressDecoding::DoubleColumnAction() i=" << i << " " << pixel->GetColumn() << " " << pixel->GetRow() << endl;
	  ArmPixel(); usleep(twait);
	  Flush(); usleep(twait);
	  SendADCTrigs(nTriggers); usleep(twait);
	  Flush(); usleep(twait);
	  //cout << "calling GetADC " << nTriggers <<  " " << nReadouts << endl;
      noError = GetADC(data, psi::FIFOSIZE, count, nTriggers, readoutStart, nReadouts);
	  //cout << "back from  GetADC " << nTriggers <<  " " << nReadouts << " noerror= " << noError <<endl;
	  if (!noError){
        std::cout << "error reading pixel  column=" << column <<  " row=" <<  row <<  "nReadouts=" << nReadouts << " nTriggers= " << nTriggers << std::endl;
	    char c;
        std::cin >> c;
	    DisarmPixel();	usleep(twait);
	    Flush();	usleep(twait);
	    continue;
	  }
      if(!(nReadouts == 1)){ std::cout << "nReadouts=" << nReadouts << std::endl;	}
	  //cout << "count = " << count << endl;
	  
	  for (int k = 0; k < nReadouts; k++) {
	    pixelFound = false;
	    int nDecodedPixelHitsModule = gDecoder->decode((int)count, &data[readoutStart[k]], decodedModuleReadout, nRocs);
	    if(!(nDecodedPixelHitsModule==1)){
          std::cout << "decoding  nhit=" << nDecodedPixelHitsModule << std::endl;
	    }
	    for(int iroc=0; iroc<nRocs; iroc++){
	      int nDecodedPixelHitsROC = decodedModuleReadout.roc[iroc].numPixelHits;
	      for (int ipixelhit = 0; ipixelhit < nDecodedPixelHitsROC; ipixelhit++)
		{
		  DecodedReadoutPixel decodedPixelHit = decodedModuleReadout.roc[iroc].pixelHit[ipixelhit];
		  if ((decodedPixelHit.rowROC == row) && (decodedPixelHit.columnROC == column))
		    {
		      pixelFound = true;
		    }else{
              std::cout << Form("unexpedted hit  column  %2d (%2d)  row  %2d (%2d) ",decodedPixelHit.columnROC,column,decodedPixelHit.rowROC,row) << std::endl;
		    }
		}
	    }
	  }
	  if(pixelFound){
	    map->Fill(column, row);
	    //cout << "pixel " << column << ","<<row<<"  found" << endl;
	  }else{
        std::cout << "pixel " << column << ","<<row<<"  \033[31;49mNOT found\033[0m " << std::endl;
	  }
	}
	//char c;		cin << c;
	usleep(twait);
	DisarmPixel();	usleep(twait);
	Flush();	usleep(twait);
	
      }
    doubleColumn->DisableDoubleColumn();
    Flush();
  }// debug
}


void AddressDecoding::AnalyseResult(int pixel)
{
  int readoutStart = 0, nDecodedPixels;
  DecodedReadoutModule decodedModuleReadout;
  if (pixel > 0) readoutStart = readoutStop[pixel-1];
  
//  for (int n = readoutStart; n<readoutStop[pixel]; n++)
//  {
//    Log::Current()->printf(" %4i", data[n]);
//  }
//  Log::Current()->printf("\n");
//  printf("start %i stop %i\n", readoutStart, readoutStop[pixel]);

  const ConfigParameters& configParameters = ConfigParameters::Singleton();
  int nRocs = configParameters.NumberOfRocs();

  if (readoutStop[pixel] - readoutStart == ((TBAnalogInterface*)tbInterface)->GetEmptyReadoutLengthADC() + 6) 
  {
    nDecodedPixels = gDecoder->decode( readoutStop[pixel] - readoutStart,
                                       &data[readoutStart], 
                                       decodedModuleReadout, 
                                       nRocs);

  }
  else 
  {
    if ( fPrintDebug )
    {
      std::cout << "ADC values = { ";
      for ( int ivalue = readoutStart; ivalue < readoutStop[pixel]; ivalue++ )
      {
        std::cout << data[ivalue] << " ";
      }
      std::cout << "}" << std::endl;
    }

    nDecodedPixels = -1;
  }

  if (nDecodedPixels < 0) 
  {
    psi::Log<psi::Info>() << "[AddressDecoding] Error: Decoding Error for Pixel( "
                   << column << ", " << row 
                   << ") on ROC" << roc->GetChipId() << '.' << std::endl;

    if( (readoutStop[pixel] - readoutStart) == 
        dynamic_cast<TBAnalogInterface *>( tbInterface)->GetEmptyReadoutLengthADC() )
    {
      psi::Log<psi::Debug>() << "[AddressDecoding] Pixel seems to be dead."
                      << std::endl;
    }

    else if( (readoutStop[pixel] - readoutStart) != 
             (dynamic_cast<TBAnalogInterface *>( tbInterface)->GetEmptyReadoutLengthADC() + 6) )
    {
      psi::Log<psi::Debug>() << "[AddressDecoding] Pixel has a wrong length ("
                      << readoutStop[pixel] - readoutStart 
                      << ") of read-out signal. Expected length is "
                      << ( dynamic_cast<TBAnalogInterface *>( tbInterface)->GetEmptyReadoutLengthADC() + 6)
                      << '.' << std::endl;
    }
  }
  else if (nDecodedPixels == 0 || decodedModuleReadout.roc[roc->GetAoutChipPosition()].numPixelHits == 0) 
  {
    psi::Log<psi::Info>() << "[AddressDecoding] Error: No address levels were found "
                   << "for Pixel( " << column << ", " << row 
                   << ") on ROC" << roc->GetChipId() << '.' << std::endl;
  }
  else if (nDecodedPixels > 1) 
  {
//    for (int n=0; n<count; n++)
//    {
//      Log::Current()->printf(" %4i", data[n]);
//    }
//    Log::Current()->printf("\n");
    psi::Log<psi::Info>() << "[AddressDecoding] Error: Too many address levels were "
                   << "found for Pixel( " << column << ", " << row 
                   << ") on ROC" << roc->GetChipId() << '.' << std::endl;
  }
  else if (row != decodedModuleReadout.roc[roc->GetAoutChipPosition()].pixelHit[0].rowROC)
  {
/*
    for (int n=readoutStart[readout]; n<readoutStop; n++)
    {
      Log::Current()->printf(" %4i", data[n]);
    }
    Log::Current()->printf("\n");
  */
    psi::Log<psi::Info>() << "[AddressDecoding] Error: wrong row "
                   << decodedModuleReadout.roc[roc->GetChipId()].pixelHit[0].rowROC
                   << " for Pixel( " << column << ", " << row 
                   << ") on ROC" << roc->GetChipId() << '.' << std::endl;
  }
  else if (column != decodedModuleReadout.roc[roc->GetAoutChipPosition()].pixelHit[0].columnROC)
  {
//    for (int n=0; n<count; n++)
//    {
//      Log::Current()->printf(" %4i", data[n]);
//    }
//    Log::Current()->printf("\n");
    psi::Log<psi::Info>() << "[AddressDecoding] Error: wrong column "
                   << decodedModuleReadout.roc[roc->GetAoutChipPosition()].pixelHit[0].columnROC
                   << " for Pixel( " << column << ", " << row 
                   << ") on ROC" << roc->GetChipId() << '.' << std::endl;
  }
  else map->Fill(column, row);
}
