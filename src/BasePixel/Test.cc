/*!
 * \file Test.cc
 * \brief Implementation of Test class.
 *
 * \b Changelog
 * 18-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - New storage data format.
 * 09-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Corrected questionable language constructions, which was found using -Wall g++ option.
 * 26-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adoptation for the new multithread TestControlNetwork interface.
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using definitions from PsiCommon.h.
 * 21-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using DataStorage class to save the results.
 * 15-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using boost::units::quantity to represent physical values.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - removed deprecated conversion from string constant to char*
 */

#include "Test.h"
#include "psi46expert/TestModule.h"
#include "BasePixel/TBAnalogInterface.h"

class TreeWrapper {
public:
    TreeWrapper(const std::string& name) : tree(new TTree(name.c_str(), name.c_str())) {}
    ~TreeWrapper() {
        tree->Write();
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

static boost::scoped_ptr<TreeWrapper> performedTestsTree(MakePerformedTestTree());

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

Test::Test()
    : histograms(new TList())
{
    const std::string treeName = MakeTreeName(LastTestId, "Test");
    results = boost::shared_ptr<TTree>(new TTree(treeName.c_str(), treeName.c_str()));
    const std::string paramsTreeName = MakeParamsTreeName(LastTestId, "Test");
    params = boost::shared_ptr<TTree>(new TTree(paramsTreeName.c_str(), paramsTreeName.c_str()));

    id = LastTestId;
    name = "Test";
    start_time = psi::log::detail::DateTimeProvider::Now();
    result = 0;
    choosen = false;
    target_id = 0;
    ++LastTestId;
}

Test::Test(const std::string& name)
    : histograms(new TList())
{
    const std::string treeName = MakeTreeName(LastTestId, name);
    results = boost::shared_ptr<TTree>(new TTree(treeName.c_str(), treeName.c_str()));
    const std::string paramsTreeName = MakeParamsTreeName(LastTestId, name);
    params = boost::shared_ptr<TTree>(new TTree(paramsTreeName.c_str(), paramsTreeName.c_str()));

    id = LastTestId;
    this->name = name;
    start_time = psi::log::detail::DateTimeProvider::Now();
    result = 0;
    choosen = false;
    target_id = 0;
    ++LastTestId;
}

Test::~Test()
{
    end_time = psi::log::detail::DateTimeProvider::Now();
    testTreeRecord.name = name;
    testTreeRecord.start_time = start_time;
    testTreeRecord.end_time = end_time;
    testTreeRecord.result = result;
    testTreeRecord.choosen = choosen;
    testTreeRecord.target_id = target_id;
    (*performedTestsTree)->Fill();

    results->Write();
    params->Write();
    histograms->Write();
}

void Test::ReadTestParameters()
{
}


TList* Test::GetHistos()
{
    return histograms;
}


TH2D *Test::GetMap(const char *mapName)
{
    return new TH2D(Form("%s_C%d", mapName, chipId), Form("%s_C%d", mapName, chipId), psi::ROCNUMCOLS, 0.,
                    psi::ROCNUMCOLS, psi::ROCNUMROWS, 0., psi::ROCNUMROWS);
}


TH1D *Test::GetHisto(const char *histoName)
{
    return new TH1D(Form("%s_c%dr%d_C%d", histoName, column, row, chipId), Form("%s_c%dr%d_C%d", histoName, column, row, chipId), 256, 0., 256.);
}

void Test::ModuleAction()
{
    for (unsigned i = 0; i < module->NRocs(); i++) {
        if (testRange->IncludesRoc(module->GetRoc(i)->GetChipId())) {
            RocAction(module->GetRoc(i).get());
        }
    }
}


void Test::RocAction()
{
    for (unsigned i = 0; i < psi::ROCNUMDCOLS; i++) {
        DoubleColumnAction(roc->GetDoubleColumn(i * 2));
    }
}


void Test::DoubleColumnAction()
{
    doubleColumn->EnableDoubleColumn();
    for (unsigned i = 0; i < psi::ROCNUMROWS * 2; i++) {
        SetPixel(doubleColumn->GetPixel(i));
        if (testRange->IncludesPixel(chipId, column, row)) PixelAction();
    }
    doubleColumn->DisableDoubleColumn();
}


void Test::PixelAction()
{
}


void Test::ModuleAction(TestModule *aTestModule)
{
    module = aTestModule;
    ModuleAction();
}


void Test::RocAction(TestRoc *aTestRoc)
{
    SetRoc(aTestRoc);
    RocAction();
}


void Test::DoubleColumnAction(TestDoubleColumn *aTestDoubleColumn)
{
    doubleColumn = aTestDoubleColumn;
    dColumn = doubleColumn->DoubleColumnNumber();
    DoubleColumnAction();
}


void Test::PixelAction(TestPixel *aTestPixel)
{
    SetPixel(aTestPixel);
    PixelAction();
}


void Test::SetModule(TestModule * aModule)
{
    module = aModule;
}


void Test::SetRoc(TestRoc * aRoc)
{
    roc = aRoc;
    chipId = roc->GetChipId();
    aoutChipPosition = roc->GetAoutChipPosition();
}


void Test::SetPixel(TestPixel * aPixel)
{
    pixel = aPixel;
    column = pixel->GetColumn();
    row = pixel->GetRow();
}


// == testboard actions ===============================================

void Test::Flush()
{
    tbInterface->Flush();
}

int Test::GetRoCnt()
{
    return ((TBAnalogInterface*)tbInterface)->GetRoCnt();
}

void Test::SendRoCnt()
{
    ((TBAnalogInterface*)tbInterface)->SendRoCnt();
}

int Test::RecvRoCnt()
{
    return ((TBAnalogInterface*)tbInterface)->RecvRoCnt();
}


void Test::SendCal(int nTrig)
{
    ((TBAnalogInterface*)tbInterface)->SendCal(nTrig);
}

int Test::AoutLevel(int position, int nTriggers)
{
    return ((TBAnalogInterface*)tbInterface)->AoutLevel(position, nTriggers);
}


int Test::SCurve(int nTrig, int dacReg, int threshold, int res[])
{
    return ((TBAnalogInterface*)tbInterface)->SCurve(nTrig, dacReg, threshold, res);
}

// == roc actions =========================================================

void Test::SetDAC(const char* dacName, int value)
{
    roc->SetDAC(dacName, value);
}

void Test::SetDAC(int dacReg, int value)
{
    roc->SetDAC(dacReg, value);
}


int Test::GetDAC(const char* dacName)
{
    return roc->GetDAC(dacName);
}

int Test::GetDAC(int dacReg)
{
    return roc->GetDAC(dacReg);
}


TestPixel *Test::GetPixel(int col, int row)
{
    return roc->GetPixel(col, row);
}


void Test::EnableDoubleColumn(int column)
{
    roc->EnableDoubleColumn(column);
}


void Test::DisableDoubleColumn(int column)
{
    roc->DisableDoubleColumn(column);
}


void Test::ClrCal()
{
    roc->ClrCal();
}


void Test::Mask()
{
    roc->Mask();
}


void Test::EnableAllPixels()
{
    roc->EnableAllPixels();
}


void Test::RestoreDacParameters()
{
    roc->RestoreDacParameters(savedDacParameters);
}


void Test::SaveDacParameters()
{
    savedDacParameters = roc->SaveDacParameters();
}


void Test::SendADCTrigs(int nTrig)
{
    roc->SendADCTrigs(nTrig);
}


bool Test::GetADC(short buffer[], unsigned short buffersize, unsigned short &wordsread, int nTrig, int startBuffer[], int &nReadouts)
{
    return roc->GetADC(buffer, buffersize, wordsread, nTrig, startBuffer, nReadouts);
}


bool Test::ADCData(short buffer[], unsigned short buffersize, unsigned short &wordsread, int nTrig)
{
    return ADCData(buffer, buffersize, wordsread, nTrig);
}

// == pixel actions ==========================================================

void Test::EnablePixel()
{
    pixel->EnablePixel();
}


void Test::DisablePixel()
{
    pixel->DisablePixel();
}


void Test::ArmPixel()
{
    pixel->ArmPixel();
}


void Test::DisarmPixel()
{
    pixel->DisarmPixel();
}


void Test::Cal()
{
    pixel->Cal();
}

// == test range ===============================================================

bool Test::IncludesPixel()
{
    return testRange->IncludesPixel(chipId, column, row);
}


bool Test::IncludesDoubleColumn()
{
    return testRange->IncludesDoubleColumn(chipId, dColumn);
}
