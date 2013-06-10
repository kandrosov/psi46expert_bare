/*!
 * \file TestDoubleColumn.h
 * \brief Definition of TestDoubleColumn class.
 */

#pragma once

#include <boost/shared_ptr.hpp>

#include "BasePixel/constants.h"
#include "TestPixel.h"
#include "BasePixel/TBAnalogInterface.h"

/*!
 * \brief Implementation of the tests at DoubleColumn level
 */
class TestDoubleColumn {

public:
    TestDoubleColumn(boost::shared_ptr<TBAnalogInterface> tbInterface, TestRoc& roc, unsigned dColumn);
    TestPixel& GetPixel(unsigned column, unsigned row) const;
    TestPixel& GetPixel(unsigned iPixel) const { return *pixels.at(iPixel); }
    TestRoc& GetRoc() const { return *roc; }

// == Tests =====================================================

    void TestWBCSBC();
    void TestTimeStampBuffer();
    void TestDataBuffer();
    void DoubleColumnTest();
    unsigned DoubleColumnNumber() const;

// == DoubleColumn actions =====================================

    void EnableDoubleColumn();
    void DisableDoubleColumn();
    void Mask();

// == Pixel actions ============================================

    void EnablePixel(unsigned col, unsigned row);
    void DisablePixel(unsigned col, unsigned row);
    void Cal(unsigned col, unsigned row);
    void Cals(unsigned col, unsigned row);
    void ArmPixel(unsigned column, unsigned row);
    void DisarmPixel(unsigned column, unsigned row);

    bool IsIncluded(boost::shared_ptr<const TestRange> testRange) const;
    void ADCData(short data[], unsigned readoutStop[]);



private:
    TestPixel& FindAlivePixel() const;
    std::vector< boost::shared_ptr<TestPixel> > FindAlivePixels(unsigned count) const;

private:
    static const unsigned NPixels = 2 * psi::ROCNUMROWS;
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    TestRoc* roc;
    unsigned doubleColumn;
    std::vector< boost::shared_ptr<TestPixel> > pixels;
};
