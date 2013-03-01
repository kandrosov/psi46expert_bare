/*!
 * \file TestDoubleColumn.cc
 * \brief Implementation of TestDoubleColumn class.
 *
 * \b Changelog
 * 26-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Removed redundant dependency from class DoubleColumn.
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using definitions from PsiCommon.h.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 */

#include "psi/log.h"

#include "TestDoubleColumn.h"
#include "interface/Delay.h"
#include "BasePixel/TBInterface.h"
#include "TestRoc.h"

TestPixel *TestDoubleColumn::GetPixel(int iPixel)
{
    return pixel[iPixel];
}

TestDoubleColumn::TestDoubleColumn(TestRoc* aRoc, int dColumn)
{
	roc = aRoc;
	doubleColumn = dColumn;
    for (int i = 0; i<psi::ROCNUMROWS; i++)
	{
        pixel[i] = new TestPixel(roc,doubleColumn*2,i);
        pixel[i+psi::ROCNUMROWS] = new TestPixel(roc,doubleColumn*2+1,i);
	}
}

TestDoubleColumn::~TestDoubleColumn() {
    for (int i = 0; i<NPixels; i++) {
        delete pixel[i];
    }
}

// Performs three double column tests, not debugged nor tested yet
void TestDoubleColumn::DoubleColumnTest()
{
	TestWBCSBC();
	TestTimeStampBuffer();
	TestDataBuffer();
}


// Find a good pixel in the double column
int TestDoubleColumn::FindGoodPixels(int count, TestPixel* pix[])
{
	TestPixel* pixel;
	int x, y;
	int x2 = 2*doubleColumn+1;
	int pos = 0;

    for (x=2*doubleColumn; x<x2; x++) for (y=0; y<psi::ROCNUMROWS; y++)
	{
		if (!(pos < count)) return pos;
		pixel = GetPixel(x,y);
		if ( pixel->IsAlive() == 1) { pix[pos] = pixel; pos++; }
	}
	return pos;
}


void TestDoubleColumn::TestWBCSBC()
{
	TestPixel* pixel;
	
	if (FindGoodPixels(1, &pixel) != 1)
	{
    psi::Log<psi::Info>() << "[TestDoubleColumn] Error: can not find good pixel."
                   << std::endl;

		return;
	}
	TBInterface* tbInterface = roc->GetTBInterface();

	roc->SetDAC("Vcal", 180);
	roc->ClrCal();
	pixel->EnablePixel();
	pixel->Cal();
	EnableDoubleColumn();
	
	tbInterface->SaveTBParameters();
	roc->SaveDacParameters();
	
	tbInterface->SetTBParameter("trc", 15);
	tbInterface->SetTBParameter("tct", 15);
	tbInterface->SetTBParameter("ttk", 10);
	tbInterface->SetTBParameter("cc", 1);
	roc->SetDAC("WBC",  15);

	
	const int wbcStep = 8;
	int wbc[wbcStep] = { 0x08, 0x09, 0x0A, 0x0C, 0x10, 0x20, 0x40, 0x80 };

	int n, td;
	bool err = false;
	bool res[wbcStep];
	for (n=0; n<wbcStep; n++)
	{
		res[n] = false;
		roc->SetDAC("WBC", wbc[n]);
		for (td=6; td<=255; td++)
		{
			tbInterface->SetTBParameter("tct", td);
			roc->SingleCal();
			gDelay->Udelay(100);
			int cnt = roc->GetRoCnt();
			if (wbc[n]==td && cnt==0 || wbc[n]!=td && cnt!=0)
			{
				err = true;
				res[n] = true;
			}
		}
	}

	pixel->DisablePixel();
	DisableDoubleColumn();
	roc->ClrCal();
	tbInterface->RestoreTBParameters();
	roc->RestoreDacParameters();

	if (err)
    psi::Log<psi::Info>() << "[TestDoubleColumn] Error." << std::endl;
}


void TestDoubleColumn::TestTimeStampBuffer()
{
	TestPixel* pixel;
	
	if (FindGoodPixels(1, &pixel) != 1)
	{
    psi::Log<psi::Info>() << "[TestDoubleColumn] Error: can not find good pixel."
                   << std::endl;
		return;
	}
	TBInterface* tbInterface = roc->GetTBInterface();

	roc->SetDAC("Vcal", 180);
	roc->ClrCal();
	pixel->EnablePixel();
	pixel->Cal();
	EnableDoubleColumn();

	tbInterface->SaveTBParameters();
	roc->SaveDacParameters();
	
	tbInterface->SetTBParameter("trc", 15);
	tbInterface->SetTBParameter("tcc",  6);
	tbInterface->SetTBParameter("tct", 120);
	tbInterface->SetTBParameter("ttk", 15);

	roc->SetDAC("WBC",  120);

	const int steps = 15;
	int res[steps];

	bool err = false;
	for (int n=1; n<steps; n++)
	{
		tbInterface->SetTBParameter("cc", n);
		roc->SingleCal();
		gDelay->Udelay(200);
		res[n] = roc->GetRoCnt();
		if (n<=12 && res[n]!= 1 || n>12 && res[n]!= 0) err = true;
	}
	pixel->DisablePixel();
	DisableDoubleColumn();
	roc->ClrCal();
	tbInterface->RestoreTBParameters();
	roc->RestoreDacParameters();

	if (err)
    psi::Log<psi::Info>() << "[TestDoubleColumn] Error." << std::endl;
}


void TestDoubleColumn::TestDataBuffer()
{
	const int nPixels = 32;
	int res[nPixels];
	TestPixel* pixel[nPixels];

	if (FindGoodPixels(32, pixel) != 32)
	{
    psi::Log<psi::Info>() << "[TestDoubleColumn] Error: can not find good pixel."
                   << std::endl;
		return;
	}

	TBInterface* tbInterface = roc->GetTBInterface();

	tbInterface->SaveTBParameters();
	roc->SaveDacParameters();

	tbInterface->SetTBParameter("tct", 80);
	tbInterface->SetTBParameter("ttk", 20);
	tbInterface->SetTBParameter("cc", 1);

	bool err = false;

	for (int n = 0; n < nPixels; n++)

	{
		pixel[n]->EnablePixel();
		pixel[n]->Cal();
		roc->SingleCal();
		gDelay->Udelay(200);
		res[n] = roc->GetRoCnt();
		if (n<31 && res[n]!= n+1 || n>=31 && res[n]!= 0) err = true;
	}
	for (int n = 0; n < nPixels; n++) pixel[n]->DisablePixel();

	DisableDoubleColumn();
	roc->ClrCal();
	tbInterface->RestoreTBParameters();
	roc->RestoreDacParameters();
	if (err)
    psi::Log<psi::Info>() << "[TestDoubleColumn] Error." << std::endl;
}

void TestDoubleColumn::EnableDoubleColumn() {
    roc->ColEnable(doubleColumn * 2, 1);
}


int TestDoubleColumn::DoubleColumnNumber()
{
    return doubleColumn;
}

void TestDoubleColumn::DisableDoubleColumn() {
    roc->ColEnable(doubleColumn * 2, 0);
}


void TestDoubleColumn::Mask() {
    DisableDoubleColumn();
    for (int i = 0; i<psi::ROCNUMROWS; i++) {
        pixel[i]->DisablePixel();
        pixel[i+psi::ROCNUMROWS]->DisablePixel();
    }
}

TestPixel* TestDoubleColumn::GetPixel(int column, int row) {
    int n = (column % 2) * psi::ROCNUMROWS + row;
    return pixel[n];
}


void TestDoubleColumn::EnablePixel(int col, int row)
{
    EnableDoubleColumn();
    GetPixel(col,row)->EnablePixel();
}

void TestDoubleColumn::DisablePixel(int col, int row)
{
    GetPixel(col,row)->DisablePixel();
}

void TestDoubleColumn::Cal(int col, int row)
{
    GetPixel(col,row)->Cal();
}

void TestDoubleColumn::Cals(int col, int row)
{
    GetPixel(col,row)->Cals();
}

// == Tests ===========================================

void TestDoubleColumn::ArmPixel(int column, int row) {
    EnableDoubleColumn();
    GetPixel(column,row)->ArmPixel();
}


void TestDoubleColumn::DisarmPixel(int column, int row) {
    DisableDoubleColumn();
    GetPixel(column,row)->DisarmPixel();
}
