/*!
 * \file TestControlNetwork.h
 * \brief Definition of TestControlNetwork class.
 *
 * \b Changelog
 * 07-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Added TestControlNetwork as supported target in psi::Shell.
 *      - TestControlNetwork moved into psi::control namespace
 * 04-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - The startup current checks moved into TestControlNetwork constructor.
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
#include "TestControlNetworkCommands.h"
#include "BiasVoltageController.h"

namespace psi {
namespace control {
/*!
 * \brief This class provides support for the tests on the ControlNetwork level
 */
class TestControlNetwork {
public:
    TestControlNetwork(boost::shared_ptr<TBAnalogInterface> aTbInterface,
                       boost::shared_ptr<BiasVoltageController> aBiasVoltageController);

    void Execute(const commands::Bias& bias);
    void Execute(const commands::FullTest&);
    void Execute(const commands::IV&);
    void Execute(const commands::TestDacProgramming&);

private:
    void Initialize();

    void AdjustDACParameters();
    void AdjustVana();
    void ShortTestAndCalibration();
    void ShortCalibration();

    std::vector< boost::shared_ptr<TestModule> > modules;
    boost::shared_ptr<TBAnalogInterface> tbInterface;
    boost::shared_ptr<BiasVoltageController> biasVoltageController;
};

} // control
} // psi
