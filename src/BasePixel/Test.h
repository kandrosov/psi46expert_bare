/*!
 * \file Test.h
 * \brief Definition of Test class.
 */

#pragma once

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <TTree.h>
#include <TList.h>
#include <TH2D.h>
#include <TH1D.h>


#include "TestRange.h"
#include "BasePixel/TBInterface.h"
#include "BasePixel/DACParameters.h"

#include "psi/exception.h"


class TestModule;
class TestRoc;
class TestDoubleColumn;
class TestPixel;

/*!
 * \brief Basic test functionalities
 */
class Test {
private:
    static unsigned LastTestId;

public:
    typedef boost::shared_ptr<const TestRange> PTestRange;
    explicit Test(const std::string& name, PTestRange testRange = PTestRange());
    virtual ~Test();

    boost::shared_ptr<TList> GetHistos();
    boost::shared_ptr<TTree> GetResults() { return results; }
    static TH2D *CreateMap(const std::string& mapName, unsigned chipId, unsigned mapId = 0);
    static TH1D *CreateHistogram(const std::string& histoName, unsigned chipId, unsigned column, unsigned row);
    virtual void ModuleAction(TestModule& testModule);
    virtual void RocAction(TestRoc& testRoc);
    virtual void DoubleColumnAction(TestDoubleColumn& testDoubleColumn);
    virtual void PixelAction(TestPixel&) {}

    void SaveDacParameters(TestRoc& roc);
    void RestoreDacParameters(TestRoc& roc);

protected:
    PTestRange testRange;
    boost::shared_ptr<TList> histograms;
    boost::shared_ptr<TTree> results, params;
    boost::shared_ptr<DACParameters> savedDacParameters;

    bool debug;

    unsigned id;
    std::string name;
    std::string start_time;
    std::string end_time;
    unsigned result;
    std::string comment;
    bool choosen;
    unsigned target_id;
};
