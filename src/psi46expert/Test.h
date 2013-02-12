/*!
 * \file Test.h
 * \brief Definition of Test class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new ConfigParameters class definition.
 *      - Adaptation for the new TestParameters class definition.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - removed deprecated conversion from string constant to char*
 *      - added SaveMeasurement method to save single measurement into the output ROOT file
 */

#pragma once

#include <boost/scoped_ptr.hpp>
#include <TList.h>
#include <TH2D.h>
#include <TH1D.h>
#include <TParameter.h>

#include "TestRange.h"
#include "BasePixel/TBInterface.h"
#include "interface/Delay.h"
#include "BasePixel/DACParameters.h"

#include "BasePixel/psi_exception.h"

class TestControlNetwork;
class TestModule;
class TestRoc;
class TestDoubleColumn;
class TestPixel;

/*!
 * \brief Basic test functionalities
 */
class Test
{
public:
    /*!
     * Save a single measurement into the output ROOT file.
     */
    template<typename M>
    static void SaveMeasurement(const std::string& name, const M& value)
    {
       boost::scoped_ptr< TParameter<M> > parameter(new TParameter<M>(name.c_str(), value));
        if(!parameter->Write())
            THROW_PSI_EXCEPTION("ERROR: measurement '" << name << "' equal to '" << value
                                << "' can't be saved into the output ROOT file.");
    }

public:
	Test();
  virtual ~Test() {}
	
	TList *GetHistos();
    TH2D *GetMap(const char *mapName);
    TH1D *GetHisto(const char *histoName);
    virtual void ReadTestParameters();
	virtual void ControlNetworkAction(TestControlNetwork *controlNetwork);
	virtual void ModuleAction(TestModule *testModule);
	virtual void RocAction(TestRoc *testRoc);
	virtual void DoubleColumnAction(TestDoubleColumn *testDoubleColumn);
	virtual void PixelAction(TestPixel *textPixel);
	virtual void ModuleAction();
	virtual void RocAction();
	virtual void DoubleColumnAction();
	virtual void PixelAction();
	
//  == tbInterface actions =====================================================

	void Flush();
	int GetRoCnt();
	void SendRoCnt();
	int RecvRoCnt();
	void SendCal(int nTrig);

	
// == roc actions ==============================================================

	void SetModule(TestModule *module);
	void SetRoc(TestRoc *roc);
	void SetPixel(TestPixel *pixel);
	void SetDAC(const char* dacName, int value);
	void SetDAC(int dacReg, int value);
	int GetDAC(const char* dacName);
	int GetDAC(int dacReg);
	TestPixel *GetPixel(int col, int row);
	void EnableDoubleColumn(int column);
	void DisableDoubleColumn(int column);
	void ClrCal();
	void SaveDacParameters();
	void RestoreDacParameters();
	void Mask();
	void EnableAllPixels();
	void SendADCTrigs(int nTrig);
	bool GetADC(short buffer[], unsigned short buffersize, unsigned short &wordsread, int nTrig, int startBuffer[], int &nReadouts);
	bool ADCData(short buffer[], unsigned short buffersize, unsigned short &wordsread, int nTrig);
	int AoutLevel(int position, int nTriggers);
	int SCurve(int nTrig, int dacReg, int threshold, int res[]);
// == pixel actions ============================================================

	void EnablePixel();
	void DisablePixel();
	void ArmPixel();
	void DisarmPixel();
	void Cal();
	
// == test range ===============================================================

	bool IncludesPixel();
	bool IncludesDoubleColumn();
	
protected:

	TestRange      *testRange;
	TBInterface    *tbInterface;
	TList          *histograms;
	
	TestControlNetwork *controlNetwork;
	TestModule *module;
	TestRoc* roc;
	TestDoubleColumn *doubleColumn;
	TestPixel *pixel;
	int chipId, column, row, dColumn, aoutChipPosition;
	DACParameters *savedDacParameters;

  bool debug;
};
