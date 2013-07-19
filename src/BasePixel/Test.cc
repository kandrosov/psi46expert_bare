/*!
 * \file Test.cc
 * \brief Implementation of Test class.
 */

#include "Test.h"
#include "psi46expert/TestModule.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/DataStorage.h"

class TreeWrapper {
public:
    TreeWrapper(const std::string& name) : tree(new TTree(name.c_str(), name.c_str())) {}
    ~TreeWrapper() {
//        tree->Write();
        delete tree;
    }
    TTree* operator->() {
        return tree;
    }

private:
    TTree* tree;
};

struct TestRecord {
    unsigned id;
    std::string name;
    std::string start_time;
    std::string end_time;
    unsigned result;
    std::string comment;
    bool choosen;
    unsigned target_id;
};

static TestRecord testTreeRecord;

static TreeWrapper* MakePerformedTestTree()
{
    TreeWrapper* tree(new TreeWrapper("performed_tests"));
    (*tree)->SetDirectory(0);
    (*tree)->Branch("id", &testTreeRecord.id);
    (*tree)->Branch("name", &testTreeRecord.name);
    (*tree)->Branch("start_time", &testTreeRecord.start_time);
    (*tree)->Branch("end_time", &testTreeRecord.end_time);
    (*tree)->Branch("result", &testTreeRecord.result);
    (*tree)->Branch("comment", &testTreeRecord.comment);
    (*tree)->Branch("choosen", &testTreeRecord.choosen);
    (*tree)->Branch("target_id", &testTreeRecord.target_id);
    return tree;
}

static boost::shared_ptr<TreeWrapper> PerformedTestsTree()
{
    static boost::shared_ptr<TreeWrapper> performedTestsTree(MakePerformedTestTree());
    return performedTestsTree;
}

unsigned Test::LastTestId = 0;
static std::string MakeTreeName(unsigned id, const std::string& name)
{
    std::ostringstream ss;
    ss << "n" << id << "_" << name;
    return ss.str();
}

static std::string MakeParamsTreeName(unsigned id, const std::string& name)
{
    std::ostringstream ss;
    ss << "n" << id << "_" << name << "_params";
    return ss.str();
}

Test::Test(const std::string& name, PTestRange _testRange)
    : testRange(_testRange), histograms(new TList()), debug(false)
{
    psi::LogInfo(name) << "Starting... " << psi::LogInfo::TimestampString() << std::endl;
    const std::string treeName = MakeTreeName(LastTestId, name);
    psi::DataStorage::Active().EnterDirectory(treeName);
    results = boost::shared_ptr<TTree>(new TTree(treeName.c_str(), treeName.c_str()));
    const std::string paramsTreeName = MakeParamsTreeName(LastTestId, name);
    params = boost::shared_ptr<TTree>(new TTree(paramsTreeName.c_str(), paramsTreeName.c_str()));

    id = LastTestId;
    this->name = name;
    start_time = psi::DateTimeProvider::Now();
    result = 0;
    choosen = false;
    target_id = 0;
    ++LastTestId;
}

Test::~Test()
{
    end_time = psi::DateTimeProvider::Now();
    testTreeRecord.id = id;
    testTreeRecord.name = name;
    testTreeRecord.start_time = start_time;
    testTreeRecord.end_time = end_time;
    testTreeRecord.result = result;
    testTreeRecord.choosen = choosen;
    testTreeRecord.target_id = target_id;
    (*PerformedTestsTree())->Fill();
    (*PerformedTestsTree())->Write("", TObject::kOverwrite);

    results->Write();
    params->Write();
    histograms->Write();
    psi::DataStorage::Active().GoToPreviousDirectory();
    psi::LogInfo(name) << "Done. " << psi::LogInfo::TimestampString() << std::endl;
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
