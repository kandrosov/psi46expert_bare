/*!
 * \file DetectorSummary.h
 * \brief Definition of DetectorSummary class.
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 */

#pragma once

#include "data/SmartTree.h"

namespace psi {
namespace data {
class DetectorSummary : public root_ext::SmartTree {
public:
    static std::string TreeName() { return "detector_test_summary"; }
    DetectorSummary() : SmartTree(TreeName()) {}
    DetectorSummary(TFile& file) : SmartTree(TreeName(), file) {}
    POINTER_TREE_BRANCH(std::string, detector_name)
    POINTER_TREE_BRANCH(std::string, operator_name)
    POINTER_TREE_BRANCH(std::string, date)
};
} // data
} // psi
