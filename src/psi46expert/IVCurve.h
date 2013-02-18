/*!
 * \file IVCurve.h
 * \brief Definition of IVCurve class.
 *
 * \b Changelog
 * 18-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - IVCurve test algorithm changed for the bare module tests.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new TestParameters class definition.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Changed to support IHighVoltageSource interface.
 */

#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include "Test.h"
#include "BasePixel/IVoltageSource.h"

class IVCurve : public Test
{
public:
    IVCurve(TestRange *testRange, TBInterface *aTBInterface);
    virtual void ReadTestParameters();
	virtual void ModuleAction();
	
private:
    psi::ElectricPotential voltStep, voltStart, voltStop;
    boost::posix_time::milliseconds delay;
    boost::shared_ptr<IVoltageSource> hvSource;
};
