/*!
 * \file ElectricCurrentMeasurements.h
 * \brief Definition of ElectricCurrentMeasurements class.
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 */

#pragma once

#include "data/SmartTree.h"

namespace psi {
namespace data {
class ElectricCurrentMeasurements : public root_ext::SmartTree {
public:
    static std::string TreeName() { return "current_measurements"; }
    ElectricCurrentMeasurements() : SmartTree(TreeName()) {}
    ElectricCurrentMeasurements(TFile& file) : SmartTree(TreeName(), file) {}
    SIMPLE_TREE_BRANCH(Double_t, current, 0)
    SIMPLE_TREE_BRANCH(Double_t, voltage, 0)
    SIMPLE_TREE_BRANCH(Double_t, timestamp, 0)
};
} // data
} // psi
