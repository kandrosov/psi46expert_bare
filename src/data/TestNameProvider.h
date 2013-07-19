/*!
 * \file TestNameProvider.h
 * \brief Definition of test names.
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 */

#pragma once

#include <string>
#include <sstream>

namespace psi {
namespace data {

struct TestNameProvider {
    static std::string TestResultsTreeName(unsigned testId, const std::string& testName)
    {
        std::ostringstream ss;
        ss << "n" << testId << "_" << testName;
        return ss.str();
    }

    static std::string TestParametersTreeName(unsigned testId, const std::string& testName)
    {
        return TestResultsTreeName(testId, testName) + "_params";
    }

    static const std::string& BumpBondingTestName() { static std::string name = "BumpBonding"; return name; }

private:
    TestNameProvider(){}
};
} // data
} // psi
