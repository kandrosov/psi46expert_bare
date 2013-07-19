/*!
 * \file PerformedTests.h
 * \brief Definition of PerformedTests class.
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 */

#pragma once

#include "data/SmartTree.h"

namespace psi {
namespace data {

struct TestRecord {
    unsigned id;
    std::string name;
    std::string start_time;
    std::string end_time;
    unsigned result;

    TestRecord() : id(0), result(0) {}
    TestRecord(unsigned _id, const std::string& _name, const std::string _start_time)
        : id(_id), name(_name), start_time(_start_time), result(0) {}
};

class PerformedTests : public root_ext::SmartTree {
public:
    static std::string TreeName() { return "performed_tests"; }
    PerformedTests() : SmartTree(TreeName()) {}
    PerformedTests(TFile& file) : SmartTree(TreeName(), file) {}

    SIMPLE_TREE_BRANCH(UInt_t, id, 0)
    POINTER_TREE_BRANCH(std::string, name)
    POINTER_TREE_BRANCH(std::string, start_time)
    POINTER_TREE_BRANCH(std::string, end_time)
    SIMPLE_TREE_BRANCH(UInt_t, result, 0)

    void Fill(const TestRecord& record)
    {
        id() = record.id;
        name() = record.name;
        start_time() = record.start_time;
        end_time() = record.end_time;
        result() = record.result;
        SmartTree::Fill();
    }
};

} // data
} // psi
