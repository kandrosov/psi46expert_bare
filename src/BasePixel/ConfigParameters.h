/*!
 * \file ConfigParameters.h
 * \brief Definition of ConfigParameters class.
 *
 * \b Changelog
 * 09-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Corrected questionable language constructions, which was found using -Wall g++ option.
 * 06-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Added PSI_CONFIG_NAME macros.
 *      - Macros CONFIG_PARAMETER renamed to PSI_CONFIG_PARAMETER.
 *      - Macros FULL_CONFIG_FILE_NAME renamed to PSI_FULL_CONFIG_FILE_NAME.
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Changed type from int to unsigned int for some parameters.
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

#define PSI_FULL_CONFIG_FILE_NAME(name) \
    std::string Full##name() const { return FullFileName(name()); }

/*!
 * \brief Configuration parameters
 */
class ConfigParameters : public psi::BaseConfig {
public:
    PSI_CONFIG_PARAMETER(bool, HighVoltageOn, true)
    PSI_CONFIG_PARAMETER(bool, TbmEnable, true)
    PSI_CONFIG_PARAMETER(bool, TbmEmulator, false)
    PSI_CONFIG_PARAMETER(bool, GuiMode, false)

    PSI_CONFIG_PARAMETER(unsigned, NumberOfRocs, 16)
    PSI_CONFIG_PARAMETER(unsigned, NumberOfModules, 1)
    PSI_CONFIG_PARAMETER(int, HubId, 31)
    PSI_CONFIG_PARAMETER(unsigned, CustomModule, 0)
    PSI_CONFIG_PARAMETER(unsigned, TbmChannel, 0)
    PSI_CONFIG_PARAMETER(unsigned, HalfModule, 0)
    PSI_CONFIG_PARAMETER(int, DataTriggerLevel, -500)
    PSI_CONFIG_PARAMETER(unsigned, EmptyReadoutLength, 54)
    PSI_CONFIG_PARAMETER(unsigned, EmptyReadoutLengthADC, 64)
    PSI_CONFIG_PARAMETER(unsigned, EmptyReadoutLengthADCDual, 40)

    PSI_CONFIG_PARAMETER(std::string, TestboardType, "Analog")
    PSI_CONFIG_PARAMETER(std::string, TestboardName, "")
    PSI_CONFIG_PARAMETER(std::string, Directory, "")
    PSI_CONFIG_PARAMETER(std::string, DacParametersFileName, "defaultDACParameters.dat")
    PSI_FULL_CONFIG_FILE_NAME(DacParametersFileName)
    PSI_CONFIG_PARAMETER(std::string, TbmParametersFileName, "defaultTBMParameters.dat")
    PSI_FULL_CONFIG_FILE_NAME(TbmParametersFileName)
    PSI_CONFIG_PARAMETER(std::string, TbParametersFileName, "defaultTBParameters.dat")
    PSI_FULL_CONFIG_FILE_NAME(TbParametersFileName)
    PSI_CONFIG_PARAMETER(std::string, TrimParametersFileName, "defaultTrimParameters.dat")
    PSI_FULL_CONFIG_FILE_NAME(TrimParametersFileName)
    PSI_CONFIG_PARAMETER(std::string, TestParametersFileName, "defaultTestParameters.dat")
    PSI_FULL_CONFIG_FILE_NAME(TestParametersFileName)
    PSI_CONFIG_PARAMETER(std::string, MaskFileName, "defaultMaskFile.dat")
    PSI_FULL_CONFIG_FILE_NAME(MaskFileName)
    PSI_CONFIG_PARAMETER(std::string, LogFileName, "log.txt")
    PSI_FULL_CONFIG_FILE_NAME(LogFileName)
    PSI_CONFIG_PARAMETER(std::string, DebugFileName, "debug.log")
    PSI_FULL_CONFIG_FILE_NAME(DebugFileName)
    PSI_CONFIG_PARAMETER(std::string, RootFileName, "expert.root")
    PSI_FULL_CONFIG_FILE_NAME(RootFileName)

    PSI_CONFIG_PARAMETER(psi::ElectricCurrent, IA, 1.2 * psi::amperes)
    PSI_CONFIG_PARAMETER(psi::ElectricCurrent, ID, 1.0 * psi::amperes)
    PSI_CONFIG_PARAMETER(psi::ElectricPotential, VA, 1.7 * psi::volts)
    PSI_CONFIG_PARAMETER(psi::ElectricPotential, VD, 2.5 * psi::volts)
    PSI_CONFIG_PARAMETER(psi::ElectricCurrent, IA_BeforeSetup_HighLimit, 0.12 * psi::amperes)
    PSI_CONFIG_PARAMETER(psi::ElectricCurrent, ID_BeforeSetup_HighLimit, 0.12 * psi::amperes)
    PSI_CONFIG_PARAMETER(psi::ElectricCurrent, IA_AfterSetup_LowLimit, 0.01 * psi::amperes)
    PSI_CONFIG_PARAMETER(psi::ElectricCurrent, ID_AfterSetup_LowLimit, 0.01 * psi::amperes)
    PSI_CONFIG_PARAMETER(psi::ElectricCurrent, IA_AfterSetup_HighLimit, 0.1 * psi::amperes)
    PSI_CONFIG_PARAMETER(psi::ElectricCurrent, ID_AfterSetup_HighLimit, 0.1 * psi::amperes)

    PSI_CONFIG_PARAMETER(std::string, VoltageSource, "Keithley237")
    PSI_CONFIG_PARAMETER(std::string, VoltageSourceDevice, "keithley")
    PSI_CONFIG_PARAMETER(bool, SetVoltageSourceToLocalModeOnExit, true)
    PSI_CONFIG_PARAMETER(unsigned, NumberOfVoltageSourceReadingsToAverage, 4)
    PSI_CONFIG_PARAMETER(psi::Time, VoltageSourceIntegrationTime, 16.670e-3 * psi::seconds)

public:
    static ConfigParameters& ModifiableSingleton() {
        static ConfigParameters instance;
        return instance;
    }

    static const ConfigParameters& Singleton() {
        return ModifiableSingleton();
    }

public:
    void WriteConfigParameterFile() const {
        Write(FullFileName("configParameters.dat"));
    }

private:
    std::string FullFileName(const std::string& fileName) const {
        return Directory() + "/" + fileName;
    }
    PSI_CONFIG_NAME("ConfigParameters")
    ConfigParameters() {}
};
