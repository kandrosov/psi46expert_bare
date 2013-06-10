/*!
 * \file ThrComp.h
 * \brief Definition of ThrComp class.
 */

#pragma once

#include "BasePixel/Test.h"

class ThrComp : public Test {
public:
    ThrComp(PTestRange testRange, boost::shared_ptr<TBAnalogInterface> aTBInterface);
    virtual void RocAction(TestRoc& roc);

private:
    void RocActionAuxiliary(TestRoc& roc, double data[], double dataMax[]);

private:
    boost::shared_ptr<TBAnalogInterface> tbInterface;
};
