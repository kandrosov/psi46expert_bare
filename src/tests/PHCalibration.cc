/*!
 * \file PHCalibration.cc
 * \brief Implementation of PHCalibration class.
 */

#include <TSystem.h>
#include <TRandom.h>
#include <TMath.h>

#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/ConfigParameters.h"
#include "psi/log.h"
#include "psi46expert/TestRoc.h"
#include "DacDependency.h"
#include "PHCalibration.h"
#include "BasePixel/TestParameters.h"

PHCalibration::PHCalibration(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface)
    : Test("PHCalibration", testRange), tbInterface(aTBInterface)
{
    const TestParameters& testParameters = TestParameters::Singleton();
    nTrig = testParameters.PHCalibrationNTrig();
// 	memoryCorrection = (*testParameters).PHMemoryCorrection / 100;
    mode = testParameters.PHCalibrationMode();
    numPixels = testParameters.PHCalibrationNPixels();
    calDelVthrComp = testParameters.PHCalibrationCalDelVthrComp();
    Initialize();
}


void PHCalibration::Initialize()
{
    if (mode == 0) {
        vcalSteps = 10;
        vcal[0] = 50;
        vcal[1] = 100;
        vcal[2] = 150;
        vcal[3] = 200;
        vcal[4] = 250;
        vcal[5] = 30;
        vcal[6] = 50;
        vcal[7] = 70;
        vcal[8] = 90;
        vcal[9] = 200;
        for (int i = 0; i < 5; i++)  ctrlReg[i] = 0;
        for (int i = 5; i < 10; i++)  ctrlReg[i] = 4;
    } else if (mode == 1) {
        vcalSteps = 100;
        for (int i = 0; i < vcalSteps; i++) {
            vcal[i] = 5 + i * 5;
            vcal[i + vcalSteps] = 5 + i * 5;
            if (i < 50) ctrlReg[i] = 0;
            else ctrlReg[i] = 4;
        }
    } else if (mode == 2) {
        vcalSteps = 102;
        for (int i = 0; i < 51; i++) {
            vcal[i] = 5 + i * 5;
            ctrlReg[i] = 0;
        }
        for (int i = 51; i < 102; i++) {
            vcal[i] = 5 + (i - 51) * 5;
            ctrlReg[i] = 4;
        }
    }
    if (mode == 3) {
        vcalSteps = 9;
        vcal[0] = 50;
        vcal[1] = 100;
        vcal[2] = 150;
        vcal[3] = 200;
        vcal[4] = 250;
        vcal[5] = 50;
        vcal[6] = 70;
        vcal[7] = 90;
        vcal[8] = 200;
        for (int i = 0; i < 5; i++)  ctrlReg[i] = 0;
        for (int i = 5; i < 9; i++)  ctrlReg[i] = 4;
    }
}

void PHCalibration::RocAction(TestRoc& roc)
{
    psi::LogInfo() << "[PHCalibration] Chip #" << roc.GetChipId() << " Calibration: start." << std::endl;

    psi::LogInfo().PrintTimestamp();
    SaveDacParameters(roc);

    // == Open file

    const ConfigParameters& configParameters = ConfigParameters::Singleton();
    char fname[1000];
    sprintf(fname, "%s/phCalibration_C%i.dat", configParameters.Directory().c_str(), roc.GetChipId());
    FILE *file = fopen(fname, "w");
    if (!file) {
        psi::LogInfo() << "[PHCalibration] Error: Can not open file '" << fname
                       << "' to write PH Calibration." << std::endl;

        return;
    }

    psi::LogInfo() << "[PHCalibration] Writing PH Calibration to '" << fname
                   << "'." << std::endl;

    fprintf(file, "Pulse heights for the following Vcal values:\n");
    fprintf(file, "Low range: ");
    for (int i = 0; i < vcalSteps; i++) {
        if (ctrlReg[i] == 0) fprintf(file, "%3i ", vcal[i]);
    }
    fprintf(file, "\n");
    fprintf(file, "High range: ");
    for (int i = 0; i < vcalSteps; i++) {
        if (ctrlReg[i] == 4) fprintf(file, "%3i ", vcal[i]);
    }
    fprintf(file, "\n");
    fprintf(file, "\n");

    // == Determine appropriate CalDel and VthrComp

    bool debug = false;

    int numFlagsRemaining = numPixels;
    TRandom u;
    bool pxlFlags[psi::ROCNUMROWS * psi::ROCNUMCOLS];
    if ( numPixels < 4160 ) {
        while ( numFlagsRemaining > 0 ) {
            int column = TMath::FloorNint(psi::ROCNUMCOLS * u.Rndm());
            int row    = TMath::FloorNint(psi::ROCNUMROWS * u.Rndm());

            if ( pxlFlags[column * psi::ROCNUMROWS + row] == false ) { // pixel not yet included in test
                psi::LogInfo() << "flagging pixel in column = " << column << ", row = " << row << " for testing" << std::endl;
                pxlFlags[column * psi::ROCNUMROWS + row] = true;
                numFlagsRemaining--;
            }
        }
    }

    if (debug) {
        calDel50 = 44;
        calDel100 = 63;
        calDel200 = 66;
        vthrComp50 = 114;
        vthrComp100 = 99;
        vthrComp200 = 85;
    } else if (calDelVthrComp) {
        roc.SetDAC(DACParameters::CtrlReg, 0);
        calDel200 = roc.GetDAC(DACParameters::CalDel);
        vthrComp200 = roc.GetDAC(DACParameters::VthrComp); // from Pretest
        roc.AdjustCalDelVthrComp(15, 15, 50, -0);
        calDel50 = roc.GetDAC(DACParameters::CalDel);
        vthrComp50 = roc.GetDAC(DACParameters::VthrComp);
        roc.AdjustCalDelVthrComp(15, 15, 100, -0);
        calDel100 = roc.GetDAC(DACParameters::CalDel);
        vthrComp100 = roc.GetDAC(DACParameters::VthrComp);
//		roc->AdjustCalDelVthrComp(15, 15, 200, -1); calDel200 = GetDAC("CalDel"); vthrComp200 = GetDAC("VthrComp");
    } else {
        calDel200 = roc.GetDAC(DACParameters::CalDel);
        vthrComp200 = roc.GetDAC(DACParameters::VthrComp); // from Pretest
        calDel100 = roc.GetDAC(DACParameters::CalDel);
        vthrComp100 = roc.GetDAC(DACParameters::VthrComp); // from Pretest
        calDel50 = roc.GetDAC(DACParameters::CalDel);
        vthrComp50 = roc.GetDAC(DACParameters::VthrComp); // from Pretest
    }

    // == Loop over all pixels

    int ph[vcalSteps][psi::ROCNUMROWS * psi::ROCNUMCOLS];
    int data[psi::ROCNUMROWS * psi::ROCNUMCOLS];
    int phPosition = 16 + roc.GetAoutChipPosition() * 3;

    for (int i = 0; i < vcalSteps; i++) {
        roc.SetDAC(DACParameters::CtrlReg, ctrlReg[i]);
        roc.SetDAC(DACParameters::CalDel, GetCalDel(i));
        roc.SetDAC(DACParameters::VthrComp, GetVthrComp(i));
        roc.SetDAC(DACParameters::Vcal, vcal[i]);
        roc.Flush();

        if ( numPixels >= 4160 )
            roc.AoutLevelChip(phPosition, nTrig, data);
        else
            roc.AoutLevelPartOfChip(phPosition, nTrig, data, pxlFlags);

        for (unsigned k = 0; k < psi::ROCNUMROWS * psi::ROCNUMCOLS; k++) ph[i][k] = data[k];
    }

    for (int col = 0; col < 52; col++) {
        for (int row = 0; row < 80; row++) {
            if (testRange->IncludesPixel(roc.GetChipId(), col, row)) {

                for (int i = 0; i < vcalSteps; i++) {
                    if (ph[i][col * psi::ROCNUMROWS + row] != 7777) fprintf(file, "%5i ", ph[i][col * psi::ROCNUMROWS + row]);
                    else fprintf(file, "  N/A ");
                }
                fprintf(file, "   Pix %2i %2i\n", col, row);
            }
        }
    }

    fclose(file);
    RestoreDacParameters(roc);
    psi::LogInfo().PrintTimestamp();
}


int PHCalibration::GetCalDel(int vcalStep)
{
    int conversion = 1;
    if (ctrlReg[vcalStep] == 4) conversion = 7;
    if (vcal[vcalStep]*conversion < 75.) return calDel50;
    else if (vcal[vcalStep]*conversion < 125.) return calDel100;
    else return calDel200;
}


int PHCalibration::GetVthrComp(int vcalStep)
{
    int conversion = 1;
    if (ctrlReg[vcalStep] == 4) conversion = 7;
    if (vcal[vcalStep]*conversion < 75.) return vthrComp50;
    else if (vcal[vcalStep]*conversion < 125.) return vthrComp100;
    else return vthrComp200;
}
