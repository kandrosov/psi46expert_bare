/*!
 * \file TestControlNetwork.h
 * \brief Definition of TestControlNetwork class.
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
    void Execute(const commands::AddressDecoding& addressDecoding);
    void Execute(const commands::PreTest&);
    void Execute(const commands::Calibration&);

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
