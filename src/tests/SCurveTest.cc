/*!
 * \file SCurveTest.cc
 * \brief Implementation of SCurveTest class.
 */

#include "psi/log.h"
#include "BasePixel/constants.h"
#include "SCurveTest.h"
#include "BasePixel/ThresholdMap.h"
#include "psi46expert/TestModule.h"
#include "BasePixel/CalibrationTable.h"
#include "BasePixel/ConfigParameters.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/TestParameters.h"

SCurveTest::SCurveTest(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface)
    : Test("SCurveTest", testRange), tbInterface(aTBInterface)
{
    const TestParameters& testParameters = TestParameters::Singleton();
    vcal = testParameters.SCurveVcal();
    vthr = testParameters.SCurveVthr();
    mode = testParameters.SCurveMode();
    nTrig = testParameters.SCurveNTrig();
}

void SCurveTest::ModuleAction(TestModule& module)
{
    testDone = false;
    ThresholdMap thresholdMap;
    const ConfigParameters& configParameters = ConfigParameters::Singleton();
    char fname[1000];

    for (unsigned i = 0; i < module.NRocs(); i++)
        module.GetRoc(i).SaveDacParameters();

    if (mode == 0) { // -- S curve in terms of VTHR
        dacReg = 12; //VthrComp
        if (vcal != -1) {
            for (unsigned i = 0; i < module.NRocs(); i++)
                module.GetRoc(i).SetDAC(DACParameters::Vcal, vcal);
        }
        tbInterface->Flush();
        mapName = "CalThresholdMap";
    } else if (mode == 1) { // -- S curve in terms of VCAL
        dacReg = 25;  //Vcal
        if (vthr != -1) {
            for (unsigned i = 0; i < module.NRocs(); i++)
                module.GetRoc(i).SetDAC(DACParameters::VthrComp, vthr);
        }
        tbInterface->Flush();
        mapName = "VcalThresholdMap";
    }

    for (unsigned i = 0; i <  module.NRocs(); i++) {

        // == Open file
        sprintf(fname, "%s/SCurveData_C%i.dat", configParameters.Directory().c_str(), module.GetRoc(i).GetChipId());
        file[i] = fopen(fname, "w");
        if (!file[i]) {
            psi::LogInfo() << "[SCurveTest] Error: Can not open file '" << fname
                           << "' to write pulse SCurves." << std::endl;
            return;
        }

        psi::LogInfo() << "[SCurveTest] Writing pulse SCurves to '" << fname
                       << "'." << std::endl;

        fprintf(file[i], "Mode %i\n", mode);

        if (testRange->IncludesRoc(module.GetRoc(i).GetChipId())) {
            psi::LogInfo() << "thr map for chip " << module.GetRoc(i).GetChipId() << std::endl;
            map[i] = thresholdMap.GetMap(mapName, module.GetRoc(i), *testRange, 4);
            histograms->Add(map[i]);
        }
    }

    Test::ModuleAction(module);

    for (unsigned i = 0; i < module.NRocs(); i++) {
        module.GetRoc(i).RestoreDacParameters();
        fclose(file[i]);
    }
}

void SCurveTest::RocAction(TestRoc& roc)
{
    if (testDone) return;  // do it only for one chip;
    testDone = true;

    Test::RocAction(roc);
}


void SCurveTest::DoubleColumnAction(TestDoubleColumn& doubleColumn)
{
    TGraph *graph;
    TestRoc& roc = doubleColumn.GetRoc();
    TestModule& module = roc.GetModule();
    int nRocs = module.NRocs();
    int thr[16 * psi::ROCNUMROWS] = {0};
    int trims[16 * psi::ROCNUMROWS] = {0};
    int chipId[16] = {0};

    for (unsigned iCol = doubleColumn.DoubleColumnNumber() * 2;
         iCol < doubleColumn.DoubleColumnNumber() * 2 + 2; iCol++) {
        if (testRange->IncludesColumn(iCol)) {
            psi::LogInfo() << "column " << iCol << std::endl;

            for (unsigned iRow = 0; iRow < psi::ROCNUMROWS; iRow++) {
                for (int iRoc = 0; iRoc < nRocs; iRoc++) {
                    chipId[iRoc] = module.GetRoc(iRoc).GetChipId();
                    thr[iRow * nRocs + iRoc] = 80; //default value
                    if (testRange->IncludesRoc(chipId[iRoc])) {
                        thr[iRow * nRocs + iRoc] = static_cast<int>( map[iRoc]->GetBinContent(iCol + 1, iRow + 1) );
                        trims[iRow * nRocs + iRoc] = module.GetRoc(iRoc).GetPixel(iCol, iRow).GetTrim();
                    }
                }
            }

            tbInterface->SCurveColumn(iCol, nTrig, dacReg, thr, trims, chipId, sCurve);

            double x[255], y[255];
            int start, stop, n, position = 0;

            for (unsigned iRow = 0; iRow < psi::ROCNUMROWS; iRow++) {
                for (int iRoc = 0; iRoc < nRocs; iRoc++) {
                    if (testRange->IncludesPixel(chipId[iRoc], iCol, iRow)) {
                        n = 0;
                        start = thr[iRow * nRocs + iRoc] - 16;
                        stop = thr[iRow * nRocs + iRoc] + 16;
                        if (start < 0) start = 0;
                        if (stop > 255) stop = 255;

                        for (int vthr = start; vthr < stop; vthr++) {
                            if (mode == 1) x[n] = CalibrationTable::VcalDAC(0, vthr);
                            else x[n] = vthr;
                            y[n] = sCurve[position + (vthr - start) * nRocs + iRoc];
                            n++;
                        }

                        if (ConfigParameters::Singleton().GuiMode()) {
                            graph = new TGraph(n, x, y);
                            graph->SetNameTitle(Form("SCurve_c%ir%i_C%d", iCol, iRow, chipId[iRoc]), Form("SCurve_c%ir%i_C%d", iCol, iRow, chipId[iRoc]));
                            histograms->Add(graph);
                            graph->Write();
                        }

                        fprintf(file[iRoc], "%2i %3i ", n, start);
                        for (int i = 0; i < n; i++) fprintf(file[iRoc], "%3i ", (int)y[i]);
                        fprintf(file[iRoc], "\n");
                    }
                }
                position += nRocs * 32;
            }
        }
    }
}
