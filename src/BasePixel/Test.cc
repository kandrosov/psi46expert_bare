/*!
 * \file Test.cc
 * \brief Implementation of Test class.
 */

#include "Test.h"
#include "psi46expert/TestModule.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/DataStorage.h"

namespace {
psi::data::PerformedTests& PerformedTestsTree()
{
    static psi::data::PerformedTests performedTestsTree;
    return performedTestsTree;
}

std::string MakeTreeName(unsigned id, const std::string& name)
{
    std::ostringstream ss;
    ss << "n" << id << "_" << name;
    return ss.str();
}

std::string MakeParamsTreeName(unsigned id, const std::string& name)
{
    std::ostringstream ss;
    ss << "n" << id << "_" << name << "_params";
    return ss.str();
}
} // anonymous namespace

unsigned Test::LastTestId = 0;

Test::Test(const std::string& name, PTestRange _testRange)
    : testRange(_testRange), histograms(new TList()), debug(false),
      record(LastTestId++, name, psi::DateTimeProvider::Now())
{
    psi::LogInfo(name) << "Starting... " << psi::LogInfo::TimestampString() << std::endl;
    const std::string treeName = MakeTreeName(record.id, name);
    psi::DataStorage::Active().EnterDirectory(treeName);
    results = boost::shared_ptr<TTree>(new TTree(treeName.c_str(), treeName.c_str()));
    const std::string paramsTreeName = MakeParamsTreeName(record.id, name);
    params = boost::shared_ptr<TTree>(new TTree(paramsTreeName.c_str(), paramsTreeName.c_str()));
}

Test::~Test()
{
    record.end_time = psi::DateTimeProvider::Now();
    PerformedTestsTree().Fill(record);
    psi::DataStorage::Active().EnterDirectory("/");
    PerformedTestsTree().RootTree().Write("", TObject::kWriteDelete);
    psi::DataStorage::Active().GoToPreviousDirectory();

    results->Write();
    params->Write();
    histograms->Write();
    psi::DataStorage::Active().GoToPreviousDirectory();
    psi::LogInfo(record.name) << "Done. " << psi::LogInfo::TimestampString() << std::endl;
}

boost::shared_ptr<TList> Test::GetHistos()
{
    return histograms;
}

TH2D *Test::CreateMap(const std::string& mapName, unsigned chipId, unsigned mapId)
{
    std::ostringstream name;
    name << mapName << "_C" << chipId;
    if(mapId)
        name << "_nb" << mapId;
    return new TH2D(name.str().c_str(), name.str().c_str(), psi::ROCNUMCOLS, 0.,
                    psi::ROCNUMCOLS, psi::ROCNUMROWS, 0., psi::ROCNUMROWS);
}

TH1D *Test::CreateHistogram(const std::string& histoName, unsigned chipId, unsigned column, unsigned row)
{
    std::ostringstream name;
    name << histoName << "_c" << column << "r" << row << "_C" << chipId;
    return new TH1D(name.str().c_str(), name.str().c_str(), 256, 0., 256.);
}

void Test::ModuleAction(TestModule& module)
{
    for (unsigned i = 0; i < module.NRocs(); i++) {
        if (testRange && testRange->IncludesRoc(module.GetRoc(i).GetChipId()))
            RocAction(module.GetRoc(i));
    }
}

void Test::RocAction(TestRoc& roc)
{
    for (unsigned i = 0; i < psi::ROCNUMDCOLS; i++) {
        if (testRange && testRange->IncludesDoubleColumn(roc.GetChipId(), i))
            DoubleColumnAction(roc.GetDoubleColumnById(i));
    }
}

void Test::DoubleColumnAction(TestDoubleColumn& doubleColumn)
{
    doubleColumn.EnableDoubleColumn();
    for (unsigned i = 0; i < psi::ROCNUMROWS * 2; i++) {
        TestPixel& pixel = doubleColumn.GetPixel(i);
        if (testRange && testRange->IncludesPixel(pixel.GetRoc().GetChipId(), pixel.GetColumn(), pixel.GetRow()))
            PixelAction(doubleColumn.GetPixel(i));
    }
    doubleColumn.DisableDoubleColumn();
}

void Test::RestoreDacParameters(TestRoc& roc)
{
    roc.RestoreDacParameters(savedDacParameters);
}

void Test::SaveDacParameters(TestRoc& roc)
{
    savedDacParameters = roc.SaveDacParameters();
}
