/*!
 * \file FullTest.h
 * \brief Definition of FullTest class.
 */

#include "BasePixel/Test.h"

/*!
 * \brief Full test of a module
 */
class FullTest : public Test {
public:
    FullTest(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface);

    virtual void RocAction(TestRoc& roc);
    virtual void ModuleAction(TestModule& module);

private:
    template<typename T>
    void DoTest(TestModule& module)
    {
        T test(testRange, tbInterface);
        test.ModuleAction(module);
        //CollectHistograms(test);
    }

    template<typename T>
    void DoTest(TestRoc& roc)
    {
        T test(testRange, tbInterface);
        test.RocAction(roc);
        //CollectHistograms(test);
    }

    boost::shared_ptr<TBAnalogInterface> tbInterface;
};
