/*!
 * \file DecoderCalibration.cc
 * \brief Implementation of DecoderCalibration class.
 */

#include <iomanip>
#include <fstream>

#include "DecoderCalibration.h"

#include "psi/exception.h"
#include "psi/log.h"
#include "RawPacketDecoder.h"

using namespace DecoderCalibrationConstants;
using namespace RawPacketDecoderConstants;

bool DecoderCalibrationModule::fPrintDebug   = false;
//bool DecoderCalibrationModule::fPrintDebug   = true;
bool DecoderCalibrationModule::fPrintWarning = true;
bool DecoderCalibrationModule::fPrintError   = true;

//-------------------------------------------------------------------------------
DecoderCalibrationModule::DecoderCalibrationModule()
{
    fCalibrationTBM.SetUltraBlackLevel(-300);
    fCalibrationTBM.SetBlackLevel(300);

    fPedestalADC = 0;
}

DecoderCalibrationModule::DecoderCalibrationModule(ADCword ultraBlack, ADCword black,
        ADCword levelROC_Address0, ADCword levelROC_Address1, ADCword levelROC_Address2, ADCword levelROC_Address3,
        ADCword levelROC_Address4, ADCword levelROC_Address5, ADCword levelROC_Address6,
        ADCword levelTBM_Address0, ADCword levelTBM_Address1, ADCword levelTBM_Address2,
        ADCword levelTBM_Address3, ADCword levelTBM_Address4)
{
    SetCalibration(ultraBlack, black,
                   levelROC_Address0, levelROC_Address1, levelROC_Address2, levelROC_Address3,
                   levelROC_Address4, levelROC_Address5, levelROC_Address6,
                   levelTBM_Address0, levelTBM_Address1, levelTBM_Address2,
                   levelTBM_Address3, levelTBM_Address4);

    fPedestalADC = 0;
}

DecoderCalibrationModule::DecoderCalibrationModule(ADCword levelsTBM[], ADCword levelsROC[][NUM_LEVELSROC + 1], int numROCs)
{
    SetCalibration(levelsTBM, levelsROC, numROCs);

    fPedestalADC = 0;
}

DecoderCalibrationModule::DecoderCalibrationModule(const char* fileName, int fileType, int mode, int numROCs)
{
    ReadCalibrationFile(fileName, fileType, mode, numROCs);

    fPedestalADC = 0;
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
void DecoderCalibrationModule::SetCalibration(ADCword ultraBlack, ADCword black,
        ADCword levelROC_Address0, ADCword levelROC_Address1, ADCword levelROC_Address2, ADCword levelROC_Address3,
        ADCword levelROC_Address4, ADCword levelROC_Address5, ADCword levelROC_Address6,
        ADCword levelTBM_Status0, ADCword levelTBM_Status1, ADCword levelTBM_Status2,
        ADCword levelTBM_Status3, ADCword levelTBM_Status4)

{
    if ( fPrintWarning ) psi::LogError() << "Warning in <DecoderCalibrationModule::SetCalibration>: this function is deprecated, please use readCalibration instead !" << std::endl;

    fCalibrationTBM.SetUltraBlackLevel(ultraBlack);
    fCalibrationTBM.SetBlackLevel(black);
    fCalibrationTBM.SetStatusLevel(0, levelTBM_Status0);
    fCalibrationTBM.SetStatusLevel(1, levelTBM_Status1);
    fCalibrationTBM.SetStatusLevel(2, levelTBM_Status2);
    fCalibrationTBM.SetStatusLevel(3, levelTBM_Status3);
    fCalibrationTBM.SetStatusLevel(4, levelTBM_Status4);

    for ( int iroc = 0; iroc < MAX_ROCS; iroc++ ) {
        fCalibrationROC[iroc].SetUltraBlackLevel(ultraBlack);
        fCalibrationROC[iroc].SetBlackLevel(black);

        fCalibrationROC[iroc].SetAddressLevel(0, levelROC_Address0);
        fCalibrationROC[iroc].SetAddressLevel(1, levelROC_Address1);
        fCalibrationROC[iroc].SetAddressLevel(2, levelROC_Address2);
        fCalibrationROC[iroc].SetAddressLevel(3, levelROC_Address3);
        fCalibrationROC[iroc].SetAddressLevel(4, levelROC_Address4);
        fCalibrationROC[iroc].SetAddressLevel(5, levelROC_Address5);
        fCalibrationROC[iroc].SetAddressLevel(6, levelROC_Address6);

        if ( fPrintDebug ) {
            if ( iroc == 0 ) {
                psi::LogInfo() << "defining adress levels = { ";
                for ( int ilevel = 0; ilevel < (NUM_LEVELSROC + 1); ilevel++ ) {
                    psi::LogInfo() << fCalibrationROC[iroc].GetAddressLevel(ilevel) << " ";
                }
                psi::LogInfo() << "}" << std::endl;
            }
        }
    }

    fNumROCs = MAX_ROCS;

    std::ostringstream ss;
    Print(ss);
    psi::LogInfo() << ss.str();
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
void DecoderCalibrationModule::SetCalibration(ADCword levelsTBM[], ADCword levelsROC[][NUM_LEVELSROC + 1], int numROCs)
{
    fCalibrationTBM.SetUltraBlackLevel(levelsTBM[0]);
    fCalibrationTBM.SetBlackLevel(300);
    for ( int ilevel = 0; ilevel < (NUM_LEVELSTBM + 1); ilevel++ ) {
        fCalibrationTBM.SetStatusLevel(ilevel, levelsTBM[ilevel]);
    }

    for ( int iroc = 0; iroc < numROCs; iroc++ ) {
        fCalibrationROC[iroc].SetUltraBlackLevel(levelsROC[iroc][0]);
        fCalibrationROC[iroc].SetBlackLevel(300);
        for ( int ilevel = 0; ilevel < (NUM_LEVELSROC + 1); ilevel++ ) {
            fCalibrationROC[iroc].SetAddressLevel(ilevel, levelsROC[iroc][ilevel]);
        }
    }

    fNumROCs = numROCs;

    std::ostringstream ss;
    Print(ss);
    psi::LogInfo() << ss.str();
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
int DecoderCalibrationModule::ReadCalibrationFile(const char* fileName, int fileType, int mode, int numROCs)
{
    switch ( fileType ) {
    case 1:
        return ReadCalibrationFile1(fileName, mode, numROCs);
    case 2:
        return ReadCalibrationFile2(fileName, mode, numROCs);
    case 3:
        return ReadCalibrationFile3(fileName, mode, numROCs);
    default:
        psi::LogError() << "Error in <DecoderCalibration::ReadCalibrationFile>: file type " << fileType << " not defined !" << std::endl;
        return -1;
    }
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
int DecoderCalibrationModule::ReadCalibrationFile1(const char* fileName, int mode, int numROCs)
/*
  Read the UltraBlack, Black and address levels of ROCs
  and the UltraBlack, Black and bit levels of the TBM
  from a calibration file

  Return value is 0 if the reading of the calibration file has been succesfull

  Error codes: 1 file not found
               2 invalid file format
*/
{
    int status = 0;
    FILE* calibrateFile = 0;

//--- open file
    if ( mode < 10 ) { // open file for module
        calibrateFile = fopen("levels-module.dat", "r");
        if ( fPrintDebug ) psi::LogInfo() << "Read in calibration from file: levels-module.dat" << std::endl;
    } else { // open file for ROC
        calibrateFile = fopen("levels-roc.dat", "r");
        if ( fPrintDebug ) psi::LogInfo() << "Read in calibration from file: levels-roc.cal" << std::endl;
    }

    if ( calibrateFile == 0 ) {
        if ( fPrintError ) psi::LogError() << " Error in <DecodeRawPacket::readCalibration>: cannot open the calibration file !" << std::endl;
        return 1;
    }

//--- read TBM ultra black, black and bit levels
//    (4 levels --> 5 limits)
    ADCword level = 0;
    status = fscanf(calibrateFile, "%hd", &level); // read UltraBlack
    if ( status == EOF ) {
        if ( fPrintError ) psi::LogError() << "Error in <DecodeRawPacket::readCalibration>: invalid format of calibration file !" << std::endl;
        return 2;
    }
    fCalibrationTBM.SetUltraBlackLevel(level);


    //status = fscanf(calibrateFile, "%d", &level); // skip reading TBM black level, keep it constant
    //if ( status == EOF ){
    //  if ( fPrintError ) cerr << "Error in <DecodeRawPacket::readCalibration>: invalid format of calibration file !" <<endl;
    //  return 2;
    //}
    //fCalibrationTBM.SetBlackLevel(level);

    for ( int ilevel = 0; ilevel < (NUM_LEVELSTBM + 1); ilevel++ ) {
        status = fscanf(calibrateFile, "%hd", &level);
        if ( status == EOF ) {
            if ( fPrintError ) psi::LogError() << "Error in <DecodeRawPacket::readCalibration>: invalid format of calibration file !" << std::endl;
            return 2;
        }
        fCalibrationTBM.SetStatusLevel(ilevel, level);
    }

    if ( fPrintDebug ) {
        psi::LogInfo() << " TBM bit levels = { "
                       << fCalibrationTBM.GetUltraBlackLevel() << " " << fCalibrationTBM.GetBlackLevel();
        for ( int ilevel = 0; ilevel < (NUM_LEVELSTBM + 1); ilevel++ ) {
            psi::LogInfo() << fCalibrationTBM.GetStatusLevel(ilevel) << " ";
        }
        psi::LogInfo() << "}" << std::endl;
    }

//--- read ROC ultra black, black and address levels
    for ( int iroc = 0; iroc < numROCs; iroc++ ) {
        status = fscanf(calibrateFile, "%hd", &level); // read ultraBlack for ROC
        if ( status == EOF ) {
            if ( fPrintError ) psi::LogError() << "Error in <DecodeRawPacket::readCalibration>: invalid format of calibration file !" << std::endl;
            return 2;
        }
        fCalibrationROC[iroc].SetUltraBlackLevel(level);

        //status = fscanf(calibrateFile, "%d", &fLevelROC_Black[iroc]); // skip reading Black for ROC, keep it constant
        //if ( status == EOF ){
        //  if ( fPrintError ) cerr << "Error in <DecodeRawPacket::readCalibration>: invalid format of calibration file !" <<endl;
        //  return 2;
        //}
        //fCalibrationROC[iroc].SetBlackLevel(level);

        for ( int ilevel = 0; ilevel < (NUM_LEVELSROC + 1); ilevel++ ) {
            status = fscanf(calibrateFile, "%hd", &level);
            if ( status == EOF ) {
                if ( fPrintError) psi::LogError() << "Error in <DecodeRawPacket::readCalibration>: invalid format of calibration file !" << std::endl;
                return 2;
            }
            fCalibrationROC[iroc].SetAddressLevel(ilevel, level);
        }

        if ( fPrintDebug ) {
            psi::LogInfo() << " ROC (" << iroc << ") address levels = { "
                           << fCalibrationROC[iroc].GetUltraBlackLevel() << " " << fCalibrationROC[iroc].GetBlackLevel();
            for ( int ilevel = 0; ilevel < (NUM_LEVELSROC + 1); ilevel++ ) {
                psi::LogInfo() << fCalibrationROC[iroc].GetAddressLevel(ilevel) << " ";
            }
            psi::LogInfo() << "}" << std::endl;
        }
    }

//--- close the calibration file.
    fclose(calibrateFile);

    if ( fPrintDebug ) {
        std::ostringstream ss;
        Print(ss);
        psi::LogInfo() << ss.str();
    }

    fNumROCs = numROCs;

//--- return success
    return 0;
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
int DecoderCalibrationModule::ReadCalibrationFile2(const char* fileName, int mode, int numROCs)
/*
  Read the UltraBlack, Black and address levels of ROCs
  and the UltraBlack, Black and bit levels of the TBM
  from a calibration file

  The order from the lowest level is :
   <= UltraBlack              --> it is an ultra black
   >  UltraBlack and <= Black --> it is black
   > table[0] and <= table[1] --> it is 0 (table[0] and black can be the same)
   > table[1] and <= table[2] --> it is 1
   > table[2] and <= table[3] --> it is 2
   > table[3] and <= table[4] --> it is 3
   > table[4] and <= table[5] --> it is 4
   > table[5] and <= table[6] --> it is 5
   > table[6]                 --> invalid (can be infinte, e.g. 99999)

  Return value is 0 if the reading of the calibration file has been succesfull

  Error codes: 1 file not found
               2 invalid file format
*/
{
    int status = 0;
    FILE* calibrateFile = 0;

//--- open file
    if ( mode < 10 ) { // open file for module
        calibrateFile = fopen("module.cal", "r");
        if ( fPrintDebug ) psi::LogInfo() << "Read in calibration from file: module.cal" << std::endl;
    } else { // open file for ROC
        calibrateFile = fopen("singleROC.cal", "r");
        if ( fPrintDebug ) psi::LogInfo() << "Read in calibration from file: singleROC.cal" << std::endl;
    }

    if ( calibrateFile == 0 ) {
        if ( fPrintError ) psi::LogError() << " Error in <DecodeRawPacket::readCalibration2>: cannot open the calibration file !" << std::endl;
        return 1;
    }

    ADCword level;
    status = fscanf(calibrateFile, "%hd", &level); // read UltraBlack
    if ( status == EOF ) {
        if ( fPrintError ) psi::LogError() << "Error in <DecodeRawPacket::readCalibration2>: invalid format of calibration file !" << std::endl;
        return 2;
    }
    fCalibrationTBM.SetUltraBlackLevel(level);

    status = fscanf(calibrateFile, "%hd", &level); // read Black
    if ( status == EOF ) {
        if ( fPrintError ) psi::LogError() << "Error in <DecodeRawPacket::readCalibration2>: invalid format of calibration file !" << std::endl;
        return 2;
    }
    fCalibrationTBM.SetBlackLevel(level);

//--- read levels of all ROCs
    bool end = false;
    for ( int iroc = 0; iroc < numROCs; iroc++ ) {
//--- set UltraBlack and Black levels of ROCs to the TBM values
        fCalibrationROC[iroc].SetUltraBlackLevel(fCalibrationTBM.GetUltraBlackLevel());
        fCalibrationROC[iroc].SetBlackLevel(fCalibrationTBM.GetBlackLevel());

        if ( !end ) {
            for ( int ilevel = 0; ilevel < (NUM_LEVELSROC + 1); ilevel++ ) {
                status = fscanf(calibrateFile, "%hd", &level);
                if ( status == EOF || level == -1 ) {
                    if ( iroc == 0 ) {
                        if ( fPrintError ) psi::LogError() << "Error in <DecodeRawPacket::readCalibration2>: invalid format of calibration file !" << std::endl;
                        return 2;
                    } else {
                        if ( fPrintDebug ) psi::LogInfo() << "ROC address level information not complete, use information of first ROC for all ROCs" << std::endl;
                        for ( int jroc = 1; jroc < numROCs; jroc++ ) {
                            for ( int jlevel = 0; jlevel < (NUM_LEVELSROC + 1); jlevel++ ) {
                                fCalibrationROC[jroc].SetAddressLevel(jlevel, fCalibrationROC[0].GetAddressLevel(jlevel));
                            }
                            end = true;
                            break;
                        }
                    }
                } else {
                    fCalibrationROC[iroc].SetAddressLevel(ilevel, level);
                }
            }
        }

        if ( fPrintDebug ) {
            psi::LogInfo() << " ROC (" << iroc << ") address levels = { "
                           << fCalibrationROC[iroc].GetUltraBlackLevel() << " " << fCalibrationROC[iroc].GetBlackLevel();
            for ( int ilevel = 0; ilevel < (NUM_LEVELSROC + 1); ilevel++ ) {
                psi::LogInfo() << fCalibrationROC[iroc].GetAddressLevel(ilevel) << " ";
            }
            psi::LogInfo() << "}" << std::endl;
        }
    }

//--- read TBM bit levels
//    (4 levels --> 5 limits)
    for ( int ilevel = 0; ilevel < (NUM_LEVELSTBM + 1); ilevel++ ) {
        status = fscanf(calibrateFile, "%hd", &level);
        if ( status == EOF || fCalibrationTBM.GetStatusLevel(ilevel) == -1 ) {
            if ( fPrintDebug ) psi::LogInfo() << "no bit levels for TBM specified, use default" << std::endl;
            break;
        }
        fCalibrationTBM.SetStatusLevel(ilevel, level);
    }

    if ( fPrintDebug ) {
        psi::LogInfo() << " TBM bit levels = { "
                       << fCalibrationTBM.GetUltraBlackLevel() << " " << fCalibrationTBM.GetBlackLevel();
        for ( int ilevel = 0; ilevel < (NUM_LEVELSTBM + 1); ilevel++ ) {
            psi::LogInfo() << fCalibrationTBM.GetStatusLevel(ilevel) << " ";
        }
        psi::LogInfo() << "}" << std::endl;
    }

//--- close the calibration file.
    fclose(calibrateFile);

    if ( fPrintDebug ) {
        std::ostringstream ss;
        Print(ss);
        psi::LogInfo() << ss.str();
    }

    fNumROCs = numROCs;

//--- return success
    return 0;
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
int DecoderCalibrationModule::ReadCalibrationFile3(const char* fileName, int mode, int numROCs)
/*
  Read the UltraBlack, Black and address levels of ROCs
  and the UltraBlack, Black and bit levels of the TBM
  from a calibration file

  The file format is that defined by DecodeRawPacket::Print

  Return value is 0 if the reading of the calibration file has been succesfull

  Error codes: 1 file not found
               2 invalid file format
*/
{
    std::ifstream file(fileName);
    if ( file.bad() ) {
        if ( fPrintError ) psi::LogError() << " Error in <DecodeRawPacket::readCalibration3>: cannot open the calibration file " << fileName << " !" << std::endl;
        return 1;
    }

//--- skip reading labels and separating lines
    char dummyString[100];
    for ( int iskip = 0; iskip < NUM_LEVELSTBM + 8; iskip++ ) {
        file >> dummyString;
        if ( fPrintDebug ) psi::LogInfo() << "READ (dummyString): " << dummyString << std::endl;
    }

//--- skip reading first "-2000" number
//    (not needed for address decoding)
    int dummyNumber;
    file >> dummyNumber;
    if ( fPrintDebug ) psi::LogInfo() << "READ (dummyNumber): " << dummyNumber << std::endl;

//--- read TBM UltraBlack and address levels
//    (skip reading TBM black level, keep it constant)
    ADCword level;
    file >> level;
    if ( fPrintDebug ) psi::LogInfo() << "READ (TBM UB): " << level << std::endl;
    fCalibrationTBM.SetUltraBlackLevel(level);
    file >> level;
    if ( fPrintDebug ) psi::LogInfo() << "READ (TBM B): " << level << std::endl;
    fCalibrationTBM.SetBlackLevel(level);

    if ( file.eof() || file.bad() ) {
        if ( fPrintError ) psi::LogError() << "Error in <DecodeRawPacket::readCalibration3>: invalid format of calibration file !" << std::endl;
        return 2;
    }

    for ( int ilevel = 0; ilevel < (NUM_LEVELSTBM + 1); ilevel++ ) {
        file >> level;
        if ( fPrintDebug ) psi::LogInfo() << "READ (TBM Lev" << ilevel << "): " << level << std::endl;
        fCalibrationTBM.SetStatusLevel(ilevel, level);

        if ( file.eof() || file.bad() ) {
            if ( fPrintError ) psi::LogError() << "Error in <DecodeRawPacket::readCalibration3>: invalid format of calibration file !" << std::endl;
            return 2;
        }
    }

//--- skip reading labels and separating lines
    for ( int iskip = 0; iskip < NUM_LEVELSROC + 3; iskip++ ) {
        file >> dummyString;
        if ( fPrintDebug ) psi::LogInfo() << "READ (dummyString): " << dummyString << std::endl;
    }

//--- read UltraBlack and address levels for each ROC
//    (skip reading black levels, keep them constant)
    for ( int iroc = 0; iroc < numROCs; iroc++ ) {
        file >> dummyString; // skip reading ROC label
        if ( fPrintDebug ) psi::LogInfo() << "READ (dummyString): " << dummyString << std::endl;

        file >> dummyNumber;
        if ( fPrintDebug ) psi::LogInfo() << "READ (dummyNumber): " << dummyNumber << std::endl;

        file >> level;
        if ( fPrintDebug ) psi::LogInfo() << "READ (ROC" << iroc << " UB): " << level << std::endl;
        fCalibrationROC[iroc].SetUltraBlackLevel(level);
        file >> level;
        if ( fPrintDebug ) psi::LogInfo() << "READ (ROC" << iroc << " B): " << level << std::endl;
        fCalibrationROC[iroc].SetBlackLevel(level);

        if ( file.eof() || file.bad() ) {
            if ( fPrintError ) psi::LogError() << "Error in <DecodeRawPacket::readCalibration3>: invalid format of calibration file !" << std::endl;
            return 2;
        }

        for ( int ilevel = 0; ilevel < (NUM_LEVELSROC + 1); ilevel++ ) {
            file >> level;
            if ( fPrintDebug ) psi::LogInfo() << "READ (ROC" << iroc << " Lev" << ilevel << "): " << level << std::endl;
            fCalibrationROC[iroc].SetAddressLevel(ilevel, level);

            if ( file.eof() || file.bad() ) {
                if ( fPrintError ) psi::LogError() << "Error in <DecodeRawPacket::readCalibration3>: invalid format of calibration file !" << std::endl;
                return 2;
            }
        }
    }

//--- skip reading last separating lines
    file >> dummyString;
    if ( fPrintDebug ) psi::LogInfo() << "READ (dummyString): " << dummyString << std::endl;

    if ( file.eof() || file.bad() ) {
        if ( fPrintError ) psi::LogError() << "Error in <DecodeRawPacket::readCalibration3>: invalid format of calibration file !" << std::endl;
        return 2;
    }

    if ( fPrintDebug ) {
        std::ostringstream ss;
        Print(ss);
        psi::LogInfo() << ss.str();
    }

    fNumROCs = numROCs;

//--- return success
    return 0;
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
const struct DecoderCalibrationROC& DecoderCalibrationModule::GetCalibrationROC(int rocId) const {
    if ( rocId >= 0 && rocId < fNumROCs )
        return fCalibrationROC[rocId];
    THROW_PSI_EXCEPTION("No Calibration defined for ROC " << rocId << "!");
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
void DecoderCalibrationModule::Print(std::ostream &outputStream) const
{
    outputStream << "Module Address Level Table:" << std::endl;
    outputStream << "====================================================================================================" << std::endl;
    outputStream << "         " << std::setw(9) << "UB" << std::setw(9) << "B" << std::endl;
    outputStream << "                                      ";
    for ( int ilevel = 0; ilevel < NUM_LEVELSTBM; ilevel++ ) {
        std::stringstream ss;
        ss << "Lev" << ilevel;
        outputStream << std::setw(9) << ss.str();
    }
    outputStream << std::endl;
    outputStream << std::setw(5) << "TBM" << std::setw(9) << -2000 << std::setw(9) << fCalibrationTBM.GetUltraBlackLevel() << std::setw(9) << fCalibrationTBM.GetBlackLevel() << std::endl;
    outputStream << "                                 ";
    for ( int ilevel = 0; ilevel < (NUM_LEVELSTBM + 1); ilevel++ ) {
        outputStream << std::setw(9) << fCalibrationTBM.GetStatusLevel(ilevel);
    }
    outputStream << std::endl;
    outputStream << "----------------------------------------------------------------------------------------------------" << std::endl;
    outputStream << "         " << std::setw(9) << "UB" << std::setw(9) << "B" << std::endl;
    outputStream << "                                      ";
    for ( int ilevel = 0; ilevel < NUM_LEVELSROC; ilevel++ ) {
        std::stringstream ss;
        ss << "Lev" << ilevel;
        outputStream << std::setw(9) << ss.str();
    }
    outputStream << std::endl;
    for ( int iroc = 0; iroc < fNumROCs; iroc++ ) {
        std::stringstream ss;
        ss << "ROC" << iroc;
        outputStream << std::setw(5) << ss.str() << std::setw(9) << -2000 << std::setw(9) << fCalibrationROC[iroc].GetUltraBlackLevel() << std::setw(9) << fCalibrationROC[iroc].GetBlackLevel() << std::endl;
        outputStream << "                                 ";
        for ( int ilevel = 0; ilevel < (NUM_LEVELSROC + 1); ilevel++ ) {
            outputStream << std::setw(9) << fCalibrationROC[iroc].GetAddressLevel(ilevel);
        }
        outputStream << std::endl;
    }
    outputStream << "====================================================================================================" << std::endl;
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
DecoderCalibrationTBM::DecoderCalibrationTBM()
{
    fUltraBlackLevel =   0;
    fBlackLevel      = 300;

    for ( int ilevel = 0; ilevel < (NUM_LEVELSTBM + 1); ilevel++ ) {
        fStatusLevel[ilevel] = 2000;
    }
}

DecoderCalibrationTBM::DecoderCalibrationTBM(ADCword levels[])
{
    fUltraBlackLevel = levels[0];
    fBlackLevel      = 300;

    for ( int ilevel = 0; ilevel < (NUM_LEVELSTBM + 1); ilevel++ ) {
        fStatusLevel[ilevel] = levels[ilevel + 1];
    }
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
void DecoderCalibrationTBM::SetStatusLevel(int levelIndex, ADCword level)
{
    if ( levelIndex >= 0 && levelIndex <= NUM_LEVELSTBM ) {
        fStatusLevel[levelIndex] = level;
    } else {
        psi::LogError() << "Error in <DecoderCalibrationTBM::SetStatusLevel>: index " << levelIndex << " out of range !" << std::endl;
    }
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
ADCword DecoderCalibrationTBM::GetStatusLevel(int levelIndex) const
{
    if ( levelIndex >= 0 && levelIndex <= NUM_LEVELSTBM ) {
        return fStatusLevel[levelIndex];
    } else {
        psi::LogError() << "Error in <DecoderCalibrationTBM::GetStatusLevel>: index " << levelIndex << " out of range !" << std::endl;
        return 0;
    }
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
DecoderCalibrationROC::DecoderCalibrationROC()
{
    fUltraBlackLevel =   0;
    fBlackLevel      = 300;

    for ( int ilevel = 0; ilevel < (NUM_LEVELSROC + 1); ilevel++ ) {
        fAddressLevel[ilevel] = 2000;
    }
}

DecoderCalibrationROC::DecoderCalibrationROC(ADCword levels[])
{
    fUltraBlackLevel = levels[0];
    fBlackLevel      = 300;

    for ( int ilevel = 0; ilevel < (NUM_LEVELSROC + 1); ilevel++ ) {
        fAddressLevel[ilevel] = levels[ilevel + 1];
    }
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
void DecoderCalibrationROC::SetAddressLevel(int levelIndex, ADCword level)
{
    if ( levelIndex >= 0 && levelIndex <= NUM_LEVELSROC ) {
        fAddressLevel[levelIndex] = level;
    } else {
        psi::LogError() << "Error in <DecoderCalibrationROC::SetAddressLevel>: index " << levelIndex << " out of range !" << std::endl;
    }
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
ADCword DecoderCalibrationROC::GetAddressLevel(int levelIndex) const
{
    if ( levelIndex >= 0 && levelIndex <= NUM_LEVELSROC ) {
        return fAddressLevel[levelIndex];
    } else {
        psi::LogError() << "Error in <DecoderCalibrationROC::GetAddressLevel>: index " << levelIndex << " out of range !" << std::endl;
        return 0;
    }
}
//-------------------------------------------------------------------------------
