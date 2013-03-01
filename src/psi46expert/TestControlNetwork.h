/*!
 * \file TestControlNetwork.h
 * \brief Definition of TestControlNetwork class.
 *
 * \b Changelog
 * 01-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Class SysCommand removed.
 * 26-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Preparations for the further multithread support.
 *      - Removed redundant dependency from ControlNetwork class.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Adaptation for the new ConfigParameters class definition.
 *      - Adaptation for the new TestParameters class definition.
 */

#pragma once

#include "TestModule.h"

/*!
 * \brief This class provides support for the tests on the ControlNetwork level
 */
class TestControlNetwork
{
public:
    TestControlNetwork(TBInterface* aTbInterface);
//	void Execute(SysCommand &command);

	void AdjustDACParameters();
	void AdjustVana();
	void DoIV();
	void FullTestAndCalibration();
    void ShortTestAndCalibration();
    void ShortCalibration();

private:
    void Initialize();

    std::vector< boost::shared_ptr<TestModule> > modules;
    TBInterface *tbInterface;

};
