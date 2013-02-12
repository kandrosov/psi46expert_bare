/*!
 * \file TestControlNetwork.h
 * \brief Definition of TestControlNetwork class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new ConfigParameters class definition.
 */

#pragma once

#include "BasePixel/ControlNetwork.h"
#include "TestModule.h"
#include "TestParameters.h"

/*!
 * \brief This class provides support for the tests on the ControlNetwork level
 */
class TestControlNetwork: public ControlNetwork
{
public:
    TestControlNetwork(TBInterface* aTbInterface);
	void Execute(SysCommand &command);

	TestParameters *GetTestParameters();
	TestModule* GetModule(int iModule);

	void AdjustDACParameters();
	void AdjustVana();
	void DoIV();
	void FullTestAndCalibration();
        void ShortTestAndCalibration();
        void ShortCalibration();

			
protected:

	TestParameters *testParameters;
};
