/*!
 * \file ConfigParameters.h
 * \brief Definition of ConfigParameters class.
 *
 * \b Changelog
 * 12-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - ConfigParameters class now inherit psi::BaseConifg class.
 * 11-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - added voltage source configuration parameters.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - added current limits for 'ia' and 'id'
 */

#pragma once

#include "BaseConfig.h"

/*!
 * \brief Configuration parameters
 */
class ConfigParameters : public psi::BaseConfig
{
public:
    CONFIG_PARAMETER(bool, HighVoltageOn, true)
    CONFIG_PARAMETER(bool, TbmEnable, true)
    CONFIG_PARAMETER(bool, TbmEmulator, false)
    CONFIG_PARAMETER(bool, KeithleyRemote, false)
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
    CONFIG_PARAMETER(std::string, TbmParametersFileName, "defaultTBMParameters.dat")
    CONFIG_PARAMETER(std::string, TbParametersFileName, "defaultTBParameters.dat")
    CONFIG_PARAMETER(std::string, TrimParametersFileName, "defaultTrimParameters.dat")
    CONFIG_PARAMETER(std::string, TestParametersFileName, "defaultTestParameters.dat")
    CONFIG_PARAMETER(std::string, MaskFileName, "defaultMaskFile.dat")
    CONFIG_PARAMETER(std::string, LogFileName, "log.txt")
    CONFIG_PARAMETER(std::string, DebugFileName, "debug.log")
    CONFIG_PARAMETER(std::string, RootFileName, "expert.root")

    CONFIG_PARAMETER(double, IA, 1.2)
    CONFIG_PARAMETER(double, ID, 1.0)
    CONFIG_PARAMETER(double, VA, 1.7)
    CONFIG_PARAMETER(double, VD, 2.5)
    CONFIG_PARAMETER(double, IA_BeforeSetup_HighLimit, 0.12)
    CONFIG_PARAMETER(double, ID_BeforeSetup_HighLimit, 0.12)
    CONFIG_PARAMETER(double, IA_AfterSetup_LowLimit, 0.01)
    CONFIG_PARAMETER(double, ID_AfterSetup_LowLimit, 0.01)
    CONFIG_PARAMETER(double, IA_AfterSetup_HighLimit, 0.1)
    CONFIG_PARAMETER(double, ID_AfterSetup_HighLimit, 0.1)

public:
    static ConfigParameters& ModifiableSingleton()
    {
        static ConfigParameters instance;
        return instance;
    }

    static const ConfigParameters& Singleton() { return ModifiableSingleton(); }

public:
    std::string FullFileName(const std::string& fileName) const { return Directory() + "/" + fileName; }
    void WriteConfigParameterFile() const { Write(FullFileName("configParameters.dat")); }

private:
   ConfigParameters() {}
};
