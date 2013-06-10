/*!
 * \file TestDoubleColumn.cc
 * \brief Implementation of TestDoubleColumn class.
 */

#include "psi/log.h"
#include "psi/date_time.h"
#include "psi/exception.h"
#include "TestDoubleColumn.h"
#include "BasePixel/TBInterface.h"
#include "TestRoc.h"

TestDoubleColumn::TestDoubleColumn(boost::shared_ptr<TBAnalogInterface> _tbInterface, TestRoc& aRoc, unsigned dColumn)
    : tbInterface(_tbInterface), roc(&aRoc), doubleColumn(dColumn)
{
    pixels.assign(NPixels, boost::shared_ptr<TestPixel>());
    for (unsigned i = 0; i < psi::ROCNUMROWS; i++) {
        pixels[i] = boost::shared_ptr<TestPixel>(new TestPixel(*roc, doubleColumn * 2, i));
        pixels[i + psi::ROCNUMROWS] = boost::shared_ptr<TestPixel>(new TestPixel(*roc, doubleColumn * 2 + 1, i));
    }
}

// Performs three double column tests, not debugged nor tested yet
void TestDoubleColumn::DoubleColumnTest()
{
    TestWBCSBC();
    TestTimeStampBuffer();
    TestDataBuffer();
}

TestPixel& TestDoubleColumn::FindAlivePixel() const
{
    return *FindAlivePixels(1)[0];
}

// Find a good pixel in the double column
std::vector< boost::shared_ptr<TestPixel> > TestDoubleColumn::FindAlivePixels(unsigned count) const
{
    std::vector< boost::shared_ptr<TestPixel> > goodPixels;
    for(unsigned n = 0; n < pixels.size() && goodPixels.size() < count; ++n) {
        if(pixels[n]->IsAlive())
            goodPixels.push_back(pixels[n]);
    }
    if(goodPixels.size() != count)
        THROW_PSI_EXCEPTION("Unable to find " << count << " alive pixels. Found only " << goodPixels.size()
                            << " alive pixels.");
    return goodPixels;
}

void TestDoubleColumn::TestWBCSBC()
{
    TestPixel& pixel = FindAlivePixel();

    roc->SetDAC(DACParameters::Vcal, 180);
    roc->ClrCal();
    pixel.EnablePixel();
    pixel.Cal();
    EnableDoubleColumn();

    tbInterface->SaveTBParameters();
    roc->SaveDacParameters();

    tbInterface->SetTBParameter(TBParameters::trc, 15);
    tbInterface->SetTBParameter(TBParameters::tct, 15);
    tbInterface->SetTBParameter(TBParameters::ttk, 10);
    tbInterface->SetTBParameter(TBParameters::cc, 1);
    roc->SetDAC(DACParameters::WBC,  15);


    const unsigned wbcStep = 8;
    unsigned wbc[wbcStep] = { 0x08, 0x09, 0x0A, 0x0C, 0x10, 0x20, 0x40, 0x80 };

    bool err = false;
    bool res[wbcStep];
    for (unsigned n = 0; n < wbcStep; n++) {
        res[n] = false;
        roc->SetDAC(DACParameters::WBC, wbc[n]);
        for (unsigned td = 6; td <= 255; td++) {
            tbInterface->SetTBParameter(TBParameters::tct, td);
            roc->SingleCal();
            psi::Sleep(100.0 * psi::micro * psi::seconds);
            unsigned cnt = roc->GetRoCnt();
            if ( (wbc[n] == td && cnt == 0) || (wbc[n] != td && cnt != 0) ) {
                err = true;
                res[n] = true;
            }
        }
    }

    pixel.DisablePixel();
    DisableDoubleColumn();
    roc->ClrCal();
    tbInterface->RestoreTBParameters();
    roc->RestoreDacParameters();

    if (err)
        psi::LogInfo() << "[TestDoubleColumn] Error." << std::endl;
}

void TestDoubleColumn::TestTimeStampBuffer()
{
    TestPixel& pixel = FindAlivePixel();

    roc->SetDAC(DACParameters::Vcal, 180);
    roc->ClrCal();
    pixel.EnablePixel();
    pixel.Cal();
    EnableDoubleColumn();

    tbInterface->SaveTBParameters();
    roc->SaveDacParameters();

    tbInterface->SetTBParameter(TBParameters::trc, 15);
    tbInterface->SetTBParameter(TBParameters::tcc,  6);
    tbInterface->SetTBParameter(TBParameters::tct, 120);
    tbInterface->SetTBParameter(TBParameters::ttk, 15);

    roc->SetDAC(DACParameters::WBC,  120);

    const unsigned steps = 15;
    unsigned res[steps];

    bool err = false;
    for (unsigned n = 1; n < steps; n++) {
        tbInterface->SetTBParameter(TBParameters::cc, n);
        roc->SingleCal();
        psi::Sleep(200.0 * psi::micro * psi::seconds);
        res[n] = roc->GetRoCnt();
        if ( (n <= 12 && res[n] != 1) || (n > 12 && res[n] != 0) ) err = true;
    }
    pixel.DisablePixel();
    DisableDoubleColumn();
    roc->ClrCal();
    tbInterface->RestoreTBParameters();
    roc->RestoreDacParameters();

    if (err)
        psi::LogInfo() << "[TestDoubleColumn] Error." << std::endl;
}

void TestDoubleColumn::TestDataBuffer()
{
    static const unsigned nPixels = 32;
    unsigned res[nPixels];
    std::vector< boost::shared_ptr<TestPixel> > alivePixels = FindAlivePixels(nPixels);

    tbInterface->SaveTBParameters();
    roc->SaveDacParameters();

    tbInterface->SetTBParameter(TBParameters::tct, 80);
    tbInterface->SetTBParameter(TBParameters::ttk, 20);
    tbInterface->SetTBParameter(TBParameters::cc, 1);

    bool err = false;

    for (unsigned n = 0; n < nPixels; n++) {
        alivePixels[n]->EnablePixel();
        alivePixels[n]->Cal();
        roc->SingleCal();
        psi::Sleep(200.0 * psi::micro * psi::seconds);
        res[n] = roc->GetRoCnt();
        if ( (n < 31 && res[n] != n + 1) || (n >= 31 && res[n] != 0) ) err = true;
    }

    for (unsigned n = 0; n < nPixels; n++)
        alivePixels[n]->DisablePixel();

    DisableDoubleColumn();
    roc->ClrCal();
    tbInterface->RestoreTBParameters();
    roc->RestoreDacParameters();
    if (err)
        psi::LogInfo() << "[TestDoubleColumn] Error." << std::endl;
}

void TestDoubleColumn::EnableDoubleColumn()
{
    roc->ColEnable(doubleColumn * 2, 1);
}


unsigned TestDoubleColumn::DoubleColumnNumber() const
{
    return doubleColumn;
}

void TestDoubleColumn::DisableDoubleColumn()
{
    roc->ColEnable(doubleColumn * 2, 0);
}

void TestDoubleColumn::Mask()
{
    DisableDoubleColumn();
    for (unsigned i = 0; i < psi::ROCNUMROWS; i++) {
        pixels[i]->DisablePixel();
        pixels[i + psi::ROCNUMROWS]->DisablePixel();
    }
}
TestPixel& TestDoubleColumn::GetPixel(unsigned column, unsigned row) const
{
    const unsigned n = (column % 2) * psi::ROCNUMROWS + row;
    return GetPixel(n);
}

void TestDoubleColumn::EnablePixel(unsigned col, unsigned row)
{
    EnableDoubleColumn();
    GetPixel(col, row).EnablePixel();
}

void TestDoubleColumn::DisablePixel(unsigned col, unsigned row)
{
    GetPixel(col, row).DisablePixel();
}

void TestDoubleColumn::Cal(unsigned col, unsigned row)
{
    GetPixel(col, row).Cal();
}

void TestDoubleColumn::Cals(unsigned col, unsigned row)
{
    GetPixel(col, row).Cals();
}

void TestDoubleColumn::ArmPixel(unsigned column, unsigned row)
{
    EnableDoubleColumn();
    GetPixel(column, row).ArmPixel();
}

void TestDoubleColumn::DisarmPixel(unsigned column, unsigned row)
{
    DisableDoubleColumn();
    GetPixel(column, row).DisarmPixel();
}

bool TestDoubleColumn::IsIncluded(boost::shared_ptr<const TestRange> testRange) const
{
    return testRange && testRange->IncludesDoubleColumn(roc->GetChipId(), doubleColumn);
}

void TestDoubleColumn::ADCData(short data[], unsigned readoutStop[])
{
    roc->SetChip();
    roc->Flush();
    tbInterface->DoubleColumnADCData(doubleColumn, data, readoutStop);
}

