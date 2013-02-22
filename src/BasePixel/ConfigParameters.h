/*!
 * \file ConfigParameters.h
 * \brief Definition of ConfigParameters class.
 *
 * \b Changelog
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Add configuration parameters for Voltage Source.
 * 21-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Add personal methods for each file to get it full name.
 * 15-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using boost::units::quantity to represent physical values.
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - ConfigParameters class now inherit psi::BaseConifg class.
 * 11-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - added voltage source configuration parameters.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - added current limits for 'ia' and 'id'
 */

#pragma once

#include "BaseConfig.h"

#define FULL_CONFIG_FILE_NAME(name) \
    std::string Full##name() const { return FullFileName(name()); }

/*!
 * \brief Configuration parameters
 */
class ConfigParameters : public psi::BaseConfig
{
public:
    CONFIG_PARAMETER(bool, HighVoltageOn, true)
    CONFIG_PARAMETER(bool, TbmEnable, true)
    CONFIG_PARAMETER(bool, TbmEmulator, false)
    CONFIG_PARAMETER(bool, GuiMode, false)

    CONFIG_PARAMETER(int, NumberOfRocs, 16)
    CONFIG_PARAMETER(int, NumberOfModules, 1)
    CONFIG_PARAMETER(int, HubId, 31)
    CONFIG_PARAMETER(int, CustomModule, 0)
    CONFIG_PARAMETER(int, TbmChannel, 0)
    CONFIG_PARAMETER(int, HalfModule, 0)
    CONFIG_PARAMETER(int, DataTriggerLevel, -500)
    CONFIG_PARAMETER(int, EmptyReadoutLength, 54)
    CONFIG_PARAMETER(int, EmptyReadoutLengthADC, 64)
    CONFIG_PARAMETER(int, EmptyReadoutLengthADCDual, 40)

    CONFIG_PARAMETER(std::string, TestboardName, "")
    CONFIG_PARAMETER(std::string, Directory, "")
    CONFIG_PARAMETER(std::string, DacParametersFileName, "defaultDACParameters.dat")
    FULL_CONFIG_FILE_NAME(DacParametersFileName)
    CONFIG_PARAMETER(std::string, TbmParametersFileName, "defaultTBMParameters.dat")
    FULL_CONFIG_FILE_NAME(TbmParametersFileName)
    CONFIG_PARAMETER(std::string, TbParametersFileName, "defaultTBParameters.dat")
    FULL_CONFIG_FILE_NAME(TbParametersFileName)
    CONFIG_PARAMETER(std::string, TrimParametersFileName, "defaultTrimParameters.dat")
    FULL_CONFIG_FILE_NAME(TrimParametersFileName)
    CONFIG_PARAMETER(std::string, TestParametersFileName, "defaultTestParameters.dat")
    FULL_CONFIG_FILE_NAME(TestParametersFileName)
    CONFIG_PARAMETER(std::string, MaskFileName, "defaultMaskFile.dat")
    FULL_CONFIG_FILE_NAME(MaskFileName)
    CONFIG_PARAMETER(std::string, LogFileName, "log.txt")
    FULL_CONFIG_FILE_NAME(LogFileName)
    CONFIG_PARAMETER(std::string, DebugFileName, "debug.log")
    FULL_CONFIG_FILE_NAME(DebugFileName)
    CONFIG_PARAMETER(std::string, RootFileName, "expert.root")
    FULL_CONFIG_FILE_NAME(RootFileName)

    CONFIG_PARAMETER(psi::ElectricCurrent, IA, 1.2 * psi::amperes)
    CONFIG_PARAMETER(psi::ElectricCurrent, ID, 1.0 * psi::amperes)
    CONFIG_PARAMETER(psi::ElectricPotential, VA, 1.7 * psi::volts)
    CONFIG_PARAMETER(psi::ElectricPotential, VD, 2.5 * psi::volts)
    CONFIG_PARAMETER(psi::ElectricCurrent, IA_BeforeSetup_HighLimit, 0.12 * psi::amperes)
    CONFIG_PARAMETER(psi::ElectricCurrent, ID_BeforeSetup_HighLimit, 0.12 * psi::amperes)
    CONFIG_PARAMETER(psi::ElectricCurrent, IA_AfterSetup_LowLimit, 0.01 * psi::amperes)
    CONFIG_PARAMETER(psi::ElectricCurrent, ID_AfterSetup_LowLimit, 0.01 * psi::amperes)
    CONFIG_PARAMETER(psi::ElectricCurrent, IA_AfterSetup_HighLimit, 0.1 * psi::amperes)
    CONFIG_PARAMETER(psi::ElectricCurrent, ID_AfterSetup_HighLimit, 0.1 * psi::amperes)

    CONFIG_PARAMETER(std::string, VoltageSource, "Keithley237")
    CONFIG_PARAMETER(std::string, VoltageSourceDevice, "keithley")
    CONFIG_PARAMETER(bool, SetVoltageSourceToLocalModeOnExit, true)
    CONFIG_PARAMETER(unsigned, NumberOfVoltageSourceReadingsToAverage, 4)
    CONFIG_PARAMETER(psi::Time, VoltageSourceIntegrationTime, 16.670e-3 * psi::seconds)

public:
    static ConfigParameters& ModifiableSingleton()
    {
        static ConfigParameters instance;
        return instance;
    }

    static const ConfigParameters& Singleton() { return ModifiableSingleton(); }

public:
    void WriteConfigParameterFile() const { Write(FullFileName("configParameters.dat")); }

private:
    std::string FullFileName(const std::string& fileName) const { return Directory() + "/" + fileName; }
    ConfigParameters() {}
};
