/*!
 * \file IVCurve.h
 * \brief Definition of IVCurve class.
 *
 * \b Changelog
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Changed to support IHighVoltageSource interface.
 */

#ifndef IVCURVE
#define IVCURVE

#include <boost/shared_ptr.hpp>
#include "Test.h"
#include "BasePixel/IVoltageSource.h"

class IVCurve : public Test
{

public:
	IVCurve(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface);
	
	virtual void ReadTestParameters(TestParameters *testParameters);
	virtual void ModuleAction();
	
protected:

	int voltStep, voltStart, voltStop, delay;
    boost::shared_ptr<IVoltageSource> hvSource;

};


#endif

