/*!
 * \file VsfOptimization.cc
 * \brief Implementation of VsfOptimization class.
 */

#include <string>

#include <TRandom.h>
#include <TMath.h>
#include <TF1.h>

#include "psi/log.h"
#include "psi/date_time.h"
#include "VsfOptimization.h"
#include "psi46expert/TestRoc.h"
#include "BasePixel/DACParameters.h"
#include "BasePixel/constants.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/ConfigParameters.h"
#include "BasePixel/TestParameters.h"
#include "BasePixel/DataStorage.h"

VsfOptimization::VsfOptimization(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface)
    : Test("VsfOptimization", testRange), tbInterface(aTBInterface)
{
    const int PIXELS = psi::ROCNUMROWS * psi::ROCNUMCOLS;

    bestVsf_pixel    .Set( PIXELS);
    bestVhldDel_pixel.Set( PIXELS);
    bestQuality_pixel.Set( PIXELS);

    const TestParameters& testParameters = TestParameters::Singleton();
    vsf.start     = testParameters.vsfStart();
    vsf.stop      = testParameters.vsfStop();
    vsf.steps     = testParameters.vsfSteps();

    vhldDel.start = testParameters.vhldDelStart();
    vhldDel.stop  = testParameters.vhldDelStop();
    vhldDel.steps = testParameters.vhldDelSteps();

    goalPar1      = testParameters.goalPar1();
    goalCurrent   = testParameters.goalCurrent();

    debug = true;
}

void VsfOptimization::RocAction(TestRoc& roc)
{
    psi::LogInfo() << "[VsfOptimization] Roc #" << roc.GetChipId() << ": Start.\n";

    SaveDacParameters(roc);
    //  DoDacDacScan();
    VsfOpt(roc);
    RestoreDacParameters(roc);
    roc.SetDAC(DACParameters::Vsf, optVsf);

//   // ###################################################################
//   // ####### use this only when 100% sure what you are doing ###########
//   ConfigParameters *configParameters = ConfigParameters::Singleton();
//   roc->WriteDACParameterFile(configParameters->GetDacParametersFileName());
//   // ####### use this only when 100% sure what you are doing ###########
//   // ###################################################################

}

void VsfOptimization::VsfOpt(TestRoc& roc)
{
    if( debug )
        psi::LogInfo() << "start = " << vsf.start << ", stop = " << vsf.stop << ", steps = " << vsf.steps << std::endl;
    psi::LogInfo() << "+++ par1 optimization +++\n";

    // PART 1: Get Vsf that corresponds to minimum of Linearity Fit Parameter.
    //         Linearity Fit Parameter varies in range [1.. ~3]
    //           1   Linear
    //           >1  Non-linear. Higher value is worse say 2 is bad enough
    par1Vsf = Par1Opt(roc);

    psi::LogDebug() << "[VsfOptimization] After PH Linearity Paramater vs Vsf "
                    << "plot scan optimized Vsf is " << par1Vsf << std::endl;

    RestoreDacParameters(roc);
    psi::LogInfo() << "+++ current test +++\n";

    // PART 2: Get Vsf for Digital Current below threshold. Scan is done by
    //         lowering Vsf starting from value obtained in PART 1
    int currentVsf = CurrentOpt2(roc);
    RestoreDacParameters(roc);

    psi::LogDebug() << "[VsfOptimization] After Digital Current vs Vsf "
                    << "plot scan optimized Vsf is " << currentVsf << std::endl;

    /*
    // ?????????? What is this ????????????
    if( currentVsf > par1Vsf) optVsf = par1Vsf;
    else optVsf = currentVsf;
    */

    optVsf = currentVsf;

    psi::LogInfo() << "[VsfOptimization] Optimized Vsf is " << optVsf
                   << std::endl;
}

int VsfOptimization::CurrentOpt2(TestRoc& roc)
{
    const DACParameters::Register DAC_REGISTER = DACParameters::Vsf;

    const std::string& dacName = DACParameters::GetRegisterName(DAC_REGISTER);

    // Get Digital Current corresponding to ZERO Vsf
    roc.SetDAC( DAC_REGISTER, 0);
    tbInterface->Flush();
    psi::Sleep(2.0 * psi::seconds);
    psi::ElectricCurrent dc0 = tbInterface->GetID();

    // Get Digital Current corresponding to Vsf obtained from PH Linearity
    // test
    roc.SetDAC( DAC_REGISTER, par1Vsf);
    tbInterface->Flush();
    psi::Sleep(2.0 * psi::seconds);
    psi::ElectricCurrent dcBestPar1 = tbInterface->GetID();

    psi::LogInfo() << "dc0 =  " << dc0 << ", dcBestPar1 = " << dcBestPar1 << std::endl;

    psi::ElectricCurrent diff = dcBestPar1 - dc0;

    if( debug )
        psi::LogInfo() << "diff = " << diff << std::endl;

    // Scan Vsf for value that gives Digital Current less then threshold
    // specified in Input parameters file. Vsf will be lowered each time
    // by step starting from value obtained from PH Linearity test
    int step     = ( vsf.stop - vsf.start) / vsf.steps;
    int dacValue = par1Vsf;
    int newVsf   = par1Vsf;
    for( psi::ElectricCurrent dc = 0.0 * psi::amperes; diff > goalCurrent; ) {
        if( debug )
            psi::LogInfo() << "+++++++++++++ while ++++++++++++++\n";

        dacValue -= step;
        roc.SetDAC( DAC_REGISTER, dacValue);

        if( debug ) psi::LogInfo() << dacName << " set to " << dacValue << std::endl;

        tbInterface->Flush();
        psi::Sleep(2.0 * psi::seconds);
        dc = tbInterface->GetID();

        if( debug ) psi::LogInfo() << "Digital current: " << dc << std::endl;

        diff = dc - dc0;

        if( debug ) psi::LogInfo() << "diff = " << diff << std::endl;
        if( debug ) psi::LogInfo() << "goalCurrent = " << goalCurrent << std::endl;

        newVsf = dacValue;
    }

    return newVsf;
}



int VsfOptimization::CurrentOpt(TestRoc& roc)
{
    psi::ElectricCurrent dc[255] = {0.0 * psi::amperes};
    psi::ElectricCurrent diff = 0;
    DACParameters::Register dacRegister = DACParameters::Vsf;
    int newVsf = 150, loopcount = 0;
    const std::string& dacName = DACParameters::GetRegisterName(dacRegister);

    std::ostringstream currentHistName;
    currentHistName << "currentHist" << dacRegister << "_ROC" << roc.GetChipId();
    TH1D *currentHist = new TH1D(currentHistName.str().c_str(), dacName.c_str(), vsf.steps, vsf.start, vsf.stop);

    roc.SetDAC(dacRegister, 0);
    tbInterface->Flush();
    psi::Sleep(2.0 * psi::seconds);
    dc[0] = tbInterface->GetID();

    for(int dacValue = vsf.start; dacValue < vsf.stop; dacValue += ((vsf.stop - vsf.start) / vsf.steps)) {
        loopcount++;
        roc.SetDAC(dacRegister, dacValue);
        if (debug) psi::LogInfo() << dacName << " set to " << dacValue << std::endl;
        tbInterface->Flush();
        psi::Sleep(2.0 * psi::seconds);
        dc[dacValue] = tbInterface->GetID();
        if (debug) psi::LogInfo() << "Digital current: " << dc[dacValue] << std::endl;
        currentHist->SetBinContent(loopcount, psi::DataStorage::ToStorageUnits(dc[dacValue]));
        diff = dc[dacValue] - dc[0];
        if (debug) psi::LogInfo() << "diff = " << diff << std::endl;
        if (debug) psi::LogInfo() << "goalCurrent = " << goalCurrent << std::endl;
        if (diff < goalCurrent) newVsf = dacValue;
    }
    histograms->Add(currentHist);
    return newVsf;
}

namespace {
Double_t FitfcnTan( Double_t *x, Double_t *par)
{
    return par[3] + par[2] * TMath::TanH(par[0] * x[0] - par[1]);
}
}

int VsfOptimization::Par1Opt(TestRoc& roc)
{
    double par1        = 777.;
    double chindf      = 777;
    DACParameters::Register dacRegister = DACParameters::Vsf;
    int    newVsf      = 150;
    int    offset = tbInterface->TBMPresent() ? 16 : 9;
    int    col;

    const std::string& dacName = DACParameters::GetRegisterName(dacRegister);

    roc.SetDAC(DACParameters::CtrlReg, 4);
    // Get Column # that will be used for testing
    col = TestCol(roc);

    //TF1 *phFit = new TF1( "phFit", FitfcnTan, 50., 1500., 4);
    TF1 *phFit = new TF1( "phFit", FitfcnTan, 0., 1500., 4);
    phFit->SetNpx( 1000);

    TH1D *hist = new TH1D( Form( "hist%i_ROC%i", dacRegister, roc.GetChipId()),
                           Form( "%s", dacName.c_str()), vsf.steps, vsf.start, vsf.stop);
    newVsf = 150;
    roc.ArmPixel( col, 5);

    int count = 1;

    for( int dacValue = vsf.start, step = ( vsf.stop - vsf.start ) / vsf.steps;
            dacValue < vsf.stop;
            dacValue += step ) {
        roc.SetDAC( dacRegister, dacValue);

        if( debug ) psi::LogInfo() << dacName << " set to " << dacValue << std::endl;

        tbInterface->Flush();

        short result[256];
        tbInterface->PHDac( 25, 256, phDacScan.GetNTrig(), offset + roc.GetAoutChipPosition() * 3, result);
        TH1D *histo = new TH1D( Form( "Vsf%dROC%i", dacValue, roc.GetChipId()),
                                Form( "Vsf%dROC%i", dacValue, roc.GetChipId()), 256, 0., 256.);

        for (int dac = 0; dac < 256; ++dac) {
            histo->SetBinContent( dac + 1, 7777 == result[dac] ? 7777 : result[dac] );
        }

        // Find Bin with MINIMUM
        double minFit = histo->GetBinCenter( histo->GetMinimumBin() );

        // you also need to cut off the upper part!
        float delta;
        int bin;
        for( bin = histo->GetMinimumBin(); bin < 255; bin++) {

            delta = histo->GetBinContent(bin + 1) - histo->GetBinContent(bin);
            if (delta > 1000) break;
        }
        if( debug ) psi::LogInfo() << "upper BIN = " << bin << "bin center " << histo->GetBinCenter(bin) - 1 << std::endl;
        if( debug ) psi::LogInfo() << "lower BIN = " <<  histo->GetMinimumBin() << "bin center " << minFit << std::endl;
        // Fit Histogram
        phFit->SetParameter( 0, 0.004);
        phFit->SetParameter( 1, 1.4);
        phFit->SetParameter( 2, 500.);
        phFit->SetParameter( 3, -200.);
        //set the fitting range such that the step is excluded
        phFit->SetRange    ( minFit, histo->GetBinCenter(bin) - 1);
        //phFit->SetParLimits( 2, 0, 10000);

        histo->Fit( "phFit", "RQ", ""); //, minFit + 10, 255);

        par1 = phFit->GetParameter( 1);

        hist->SetBinContent( count, par1);
        chindf = phFit->GetChisquare() / phFit->GetNDF();

        if( debug )
            psi::LogInfo() << "par1 = " << par1 << std::endl;

        histo->GetYaxis()->SetRangeUser(-1000, 1500);

        // Save Histogram in Output
        histograms->Add( histo);
        newVsf = dacValue;

        if( debug )
            psi::LogInfo() << "goalPar1 = " << goalPar1 << std::endl;

        if( par1 < goalPar1 && 0. < par1 && 20 > chindf) break;

        ++count;
    }

    roc.DisarmPixel( col, 5);
    histograms->Add( hist);

    return newVsf;
}

// Test 5 columns in step of 5 (1 pixel per column) to get mean linearity
// parameter and find then first column with deviation of its Linearity
// parameter less than threshold. Such column will be used for tests.
int VsfOptimization::TestCol(TestRoc& roc)
{
    int col = 5;
    roc.SetDAC(DACParameters::CtrlReg, 4);
    double par1[5];
    double allPar1 = 0;

    TF1 *phFit = new TF1( "phFit", FitfcnTan, 50., 1500., 4);
    phFit->SetNpx      ( 1000);
    phFit->SetParameter( 0, 0.00382);
    phFit->SetParameter( 1, 0.886);
    phFit->SetParameter( 2, 112.7);
    phFit->SetParameter( 3, 113.0);
    phFit->SetRange    ( 50, 1500);

    int offset = tbInterface->TBMPresent() ? 16 : 9;

    // Try 5 columns (1 pixel per column) to get Linearity parameter for
    // them and calculate arithmetic mean value.
    for( int pix = 0; 5 > pix; ++pix) {
        roc.ArmPixel( col, 5);
        tbInterface->Flush();

        short result[256];
        tbInterface->PHDac( 25, 256, phDacScan.GetNTrig(), offset + roc.GetAoutChipPosition() * 3, result);
        TH1D *histo = new TH1D( Form( "Col%dROC%i", col, roc.GetChipId()),
                                Form( "Col%dROC%i", col, roc.GetChipId()), 256, 0., 256.);
        for (int dac = 0; 256 > dac; ++dac) {
            histo->SetBinContent( dac + 1, 7777 == result[dac] ? 7777 : result[dac] );
        }

        histo->Fit( "phFit", "RQ", "", phDacScan.FitStartPoint( histo) + 10, 255);
        par1[pix] = phFit->GetParameter( 1);
        allPar1 += par1[pix];

        roc.DisarmPixel( col, 5);
        col += 5;
    }
    if( debug )
        psi::LogInfo() << "mean par1 = " << (allPar1 / 5) << std::endl;

    double bestDiff = 777;
    double diff;
    int testCol = 0;

    // Get first column that has deviation from mean value less than specified
    // threshold
    for( int pix = 0; 5 > pix; ++pix) {
        diff = TMath::Abs( allPar1 / 5 - par1[pix]);
        if( diff < bestDiff) {
            bestDiff = diff;
            testCol = 5 + 5 * pix;
        }
    }

    if( debug )
        psi::LogInfo() << "test col = " << testCol << std::endl;

    psi::LogDebug() << "[VsfOptimization] Using Pixel( " << testCol << ", 5)."
                    << std::endl;

    return testCol;
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void VsfOptimization::DoDacDacScan(TestRoc& roc)
{
//   const int vsfStart     = 120;
//   const int vsfStop      = 180;
//   const int vsfSteps     =  10;
//   const int vhldDelStart =  10;
//   const int vhldDelStop  = 250;
//   const int vhldDelSteps =  10;
    /*
    const int vcalSteps = 9;
    int vcal[vcalSteps];
    int ctrlReg[vcalSteps];
    vcal[0] = 50;
    vcal[1] = 100;
    vcal[2] = 150;
    vcal[3] = 200;
    vcal[4] = 250;
    vcal[5] = 50;
    vcal[6] = 70;
    vcal[7] = 90;
    vcal[8] = 200;
    for ( int i = 0; i < 5; i++ ) ctrlReg[i] = 0;
    for ( int i = 5; i < 9; i++ ) ctrlReg[i] = 4;
    */
    const int vcalSteps = 102;
    int vcal[vcalSteps];
    int ctrlReg[vcalSteps];
    for (int ivcal = 0; ivcal < (vcalSteps / 2); ivcal++) {
        vcal[ivcal] = 5 + ivcal * 5;
        ctrlReg[ivcal] = 0;
        vcal[ivcal + (vcalSteps / 2)] = 5 + ivcal * 5;
        ctrlReg[ivcal + (vcalSteps / 2)] = 4;
    }

    const int numPixels = 100;
    //const int numPixels = 4160;
    int numFlagsRemaining = numPixels;
    TRandom u;
    bool pxlFlags[psi::ROCNUMROWS * psi::ROCNUMCOLS];
    if ( numPixels < 4160 ) {
        while ( numFlagsRemaining > 0 ) {
            int column = TMath::FloorNint(psi::ROCNUMCOLS * u.Rndm());
            int row    = TMath::FloorNint(psi::ROCNUMROWS * u.Rndm());

            if ( pxlFlags[column * psi::ROCNUMROWS + row] == false ) { // pixel not yet included in test
                //psi::LogInfo() << "flagging pixel in column = " << column << ", row = " << row << " for testing" << endl;
                pxlFlags[column * psi::ROCNUMROWS + row] = true;
                numFlagsRemaining--;
            }
        }
    }

    int ph[vcalSteps][psi::ROCNUMROWS * psi::ROCNUMCOLS];
    int data[psi::ROCNUMROWS * psi::ROCNUMCOLS];
    int phPosition = 16 + roc.GetAoutChipPosition() * 3;

    TString histogramNameHighRange = "PHCalibration_HighRange";
    TH1D* histogramHighRange = new TH1D(histogramNameHighRange, histogramNameHighRange, 256, -0.5, 255.5);
    TString histogramNameLowRange  = "PHCalibration_LowRange";
    TH1D* histogramLowRange = new TH1D(histogramNameLowRange, histogramNameLowRange, 256, -0.5, 255.5);

    for ( unsigned ipixel = 0; ipixel < psi::ROCNUMROWS * psi::ROCNUMCOLS; ipixel++ ) {
        bestVsf_pixel[ipixel] = -1;
        bestVhldDel_pixel[ipixel] = -1;
        bestQuality_pixel[ipixel] = -1.e6;
    }

    for ( int nVsf = vsf.start; nVsf <= vsf.stop; nVsf += vsf.steps ) {
        roc.SetDAC(DACParameters::Vsf, nVsf);
        tbInterface->Flush();

        for ( int nVhldDel = vhldDel.start; nVhldDel <= vhldDel.stop; nVhldDel += vhldDel.steps ) {
            roc.SetDAC(DACParameters::VhldDel, nVhldDel);
            tbInterface->Flush();

            psi::LogDebug() << "[VsfOptimization] Testing Vsf = " << nVsf
                            << ", VhldDel = " << nVhldDel << std::endl;

            for ( int ivcal = 0; ivcal < vcalSteps; ivcal++ ) {
                roc.SetDAC(DACParameters::CtrlReg, ctrlReg[ivcal]);
                //SetDAC("CalDel", GetCalDel(ivcal));     --> copied from PHCalibration, is this neccessary ?
                //SetDAC("VthrComp", GetVthrComp(ivcal)); --> copied from PHCalibration, is this neccessary ?
                roc.SetDAC(DACParameters::Vcal, vcal[ivcal]);
                tbInterface->Flush();

                if ( numPixels >= 4160 )
                    roc.AoutLevelChip(phPosition, phDacScan.GetNTrig(), data);
                else
                    roc.AoutLevelPartOfChip(phPosition, phDacScan.GetNTrig(), data, pxlFlags);

                for ( unsigned ipixel = 0; ipixel < psi::ROCNUMROWS * psi::ROCNUMCOLS; ipixel++ ) {
                    ph[ivcal][ipixel] = data[ipixel];
                    if ( debug ) {
                        if ( pxlFlags[ipixel] == true ) {
                            //const char* flag = (pxlFlags[ipixel] == true) ? "true" : "false";
                            //psi::LogInfo() << "pxlFlag = " << flag << endl;
                            psi::LogInfo() << "PH = " << ph[ivcal][ipixel] << std::endl;
                        }
                    }
                }
            }

            for ( unsigned column = 0; column < psi::ROCNUMCOLS; column++ ) {
                for ( unsigned row = 0; row < psi::ROCNUMROWS; row++ ) {
                    Int_t ipixel = column * psi::ROCNUMROWS + row;

                    if ( pxlFlags[ipixel] == true ) {
                        histogramHighRange->Reset();
                        histogramLowRange->Reset();

                        for ( int ivcal = 0; ivcal < vcalSteps; ivcal++) {

//--- skip Vcal values that did not exceed comparator thresholds and did not produce hits
                            if ( ph[ivcal][ipixel] == 7777 ) continue;

                            TH1* histogram = 0;
                            if ( ctrlReg[ivcal] == 0 ) {
                                histogram = histogramLowRange;
                            } else if ( ctrlReg[ivcal] == 4 ) {
                                histogram = histogramHighRange;
                            } else {
                                continue;
                            }

                            histogram->Fill(vcal[ivcal], ph[ivcal][ipixel]);
                        }

                        if ( debug ) {
                            psi::LogInfo() << "histogramLowRange:" << std::endl;
                            for ( int ibin = 1; ibin <= 256; ibin++ ) {
                                psi::LogInfo() << " bin-content(ibin = " << ibin << ") = " << histogramLowRange->GetBinContent(ibin) << std::endl;
                            }

                            psi::LogInfo() << "histogramHighRange:" << std::endl;
                            for ( int ibin = 1; ibin <= 256; ibin++ ) {
                                psi::LogInfo() << " bin-content(ibin = " << ibin << ") = " << histogramHighRange->GetBinContent(ibin) << std::endl;
                            }
                        }

                        double quality = phDacScan.Quality(histogramLowRange, histogramHighRange);

                        if ( debug ) {
                            psi::LogInfo() << "quality = " << quality << std::endl;
                        }

                        if ( quality > bestQuality_pixel[ipixel] ) {
                            bestVsf_pixel[ipixel] = nVsf;
                            bestVhldDel_pixel[ipixel] = nVhldDel;
                            bestQuality_pixel[ipixel] = quality;
                        }
                    }
                }
            }
        }
    }

    int numVhldDelSteps = ((vhldDel.stop - vhldDel.start) / vhldDel.steps) + 1;
    int numVsfSteps = ((vsf.stop - vsf.start) / vsf.steps) + 1;
    TH2D* histogramBestVsfVhldDel = new TH2D("bestVsfVhldDel", "bestVsfVhldDel",
            numVhldDelSteps, vhldDel.start - 0.5 * vhldDel.steps, vhldDel.stop + 0.5 * vhldDel.steps,
            numVsfSteps, vsf.start - 0.5 * vsf.steps, vsf.stop + 0.5 * vsf.steps);
    TH1D* histogramBestQuality = new TH1D("bestQuality", "bestQuality", 200, 0., 2000.);

    for ( unsigned column = 0; column < psi::ROCNUMCOLS; column++ ) {
        for ( unsigned row = 0; row < psi::ROCNUMROWS; row++ ) {
            Int_t ipixel = column * psi::ROCNUMROWS + row;

            if ( pxlFlags[ipixel] == true ) {
                histogramBestVsfVhldDel->Fill(bestVhldDel_pixel[ipixel], bestVsf_pixel[ipixel]);
                histogramBestQuality->Fill(bestQuality_pixel[ipixel]);
            }
        }
    }

    histograms->Add(histogramBestVsfVhldDel);
    histograms->Add(histogramBestQuality);

    double maxBinContent = 0.;
    bestVsf_ROC = -1;
    bestVhldDel_ROC = -1;
    for ( int ibinx = 1; ibinx <= histogramBestVsfVhldDel->GetNbinsX(); ibinx++ ) {
        for ( int ibiny = 1; ibiny <= histogramBestVsfVhldDel->GetNbinsY(); ibiny++ ) {
            if ( histogramBestVsfVhldDel->GetBinContent(ibinx, ibiny) > maxBinContent ) {
                bestVsf_ROC = vsf.start + (ibiny - 1) * vsf.steps;
                bestVhldDel_ROC = vhldDel.start + (ibinx - 1) * vhldDel.steps;
                maxBinContent = histogramBestVsfVhldDel->GetBinContent(ibinx, ibiny);
            }
        }
    }

    psi::LogInfo() << " optimal Vsf = " << bestVsf_ROC << std::endl;
    psi::LogInfo() << " optimal VhldDel = " << bestVhldDel_ROC << std::endl;
}
