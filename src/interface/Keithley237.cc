/*!
 * \file Keithley237.cc
 * \brief Implementation of Keithley237 class.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 25-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Added method Accuracy.
 *      - IVoltageSource and Keithley237 moved into psi namespace.
 *      - Switched to ElectricPotential, ElectricCurrent and Time defined in PsiCommon.h.
 * 10-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Added a workaround method for the problem that Keithley does not receives the first send command.
 * 07-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - All physical values now represented using boost::units::quantity.
 * 06-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Added Keithley237::SendAndCheck method.
 *      - The main operation logic is reviewed and corrected.
 * 05-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - All code within Keithley237Internals namespace is extracted to Keithley237Internals.cc.
 * 04-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Removed CommandExecute class. To the method Send added a boolean parameter that indicates if command should
 *        be executed or not.
 *      - Implemented parse of the Keithley Machine Status string.
 *      - Added commands to set compliance, filtering and integration time.
 *      - Added Keithley237::Configuration class to store all parameters required to initialize Keithley.
 * 01-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - GPIB driver API calls are extracted into GpibDevice class. Keithley237 are now access to GPIB using GpibStream
 *        class.
 *      - Class Keithley237Internals::Command refactored with Boost.Preprocessor macros.
 * 31-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Changed Set method signature. Now it takes IVoltageSource::Value as parameter and returns value that was
 *        really set on the device.
 *      - Implemented parse of the Keithley Error Status Word and convertion of it to the messages that can be uderstood
 *        by the end-user.
 *      - Now all commands for Keithley used in the code are defined through Keithley237Internals::Command class.
 * 30-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#include "Keithley237.h"

using namespace psi::Keithley237Internals;
using namespace psi::Keithley237Internals::Commands;

const psi::ElectricCurrent psi::Keithley237::MAX_COMPLIANCE = 0.01 * psi::amperes;
const psi::ElectricPotential psi::Keithley237::ACCURACY = 0.1 * psi::volts;


psi::Keithley237::Keithley237(const Configuration& configuration)
{
    try
    {
        gpibStream = boost::shared_ptr<GpibStream>(new GpibStream(configuration.GetDeviceName(),
                     configuration.GoLocalOnDestruction()));
        gpibStream->exceptions(std::ios::badbit | std::ios::failbit);
        Prepare();
        SendAndCheck(CmdSelfTests()(RestoreFactoryDefaults));
        SendAndCheck(CmdSetFilter()(configuration.GetFilterMode()));
        SendAndCheck(CmdSetIntegrationTime()(configuration.GetIntegrationTimeMode()));
        SendAndCheck(CmdSetOutputDataFormat()(MachineStatus::OutputDataFormat::SourceValue |
                                              MachineStatus::OutputDataFormat::MeasureValue,
                                              MachineStatus::OutputDataFormat::ASCII_Prefix_NoSuffix,
                                              MachineStatus::OutputDataFormat::OneLineFromDCBuffer));
    }
    catch(std::ios_base::failure& e)
    {
        THROW_PSI_EXCEPTION("Unable to connect to the device '" << configuration.GetDeviceName() << "'. " << std::endl
                            << e.what());
    }
}

psi::Keithley237::~Keithley237()
{
    Off();
}

void psi::Keithley237::Prepare()
{
    try
    {
        (*gpibStream) << CmdExecute()();
        gpibStream->flush();
    }
    catch(std::ios_base::failure&)
    {
        gpibStream->clear();
        gpibStream->flush();
    }
}

psi::IVoltageSource::Value psi::Keithley237::Set(const Value& value)
{
    if(psi::abs(value.Voltage) > VoltageRanges.GetLastValue())
        THROW_PSI_EXCEPTION("Voltage value is out of range. Requested voltage value to set is " << value.Voltage
                            << ". Maximal supported absolut value is " << VoltageRanges.GetLastValue() << ".");
    if(psi::abs(value.Compliance) > MAX_COMPLIANCE)
        THROW_PSI_EXCEPTION("Compliance value is out of range. Requested compliance value to set is "
                            << value.Compliance << ". Maximal supported absolut value is " << MAX_COMPLIANCE << ".");
    SendAndCheck(CmdSetSourceAndFunction()(SourceVoltageMode, DCFunction));
    SendAndCheck(CmdSetCompliance()(value.Compliance, CurrentRanges.GetAutorangeModeId()));
    SendAndCheck(CmdSetBias()(value.Voltage, VoltageRanges.GetLastMode(), 0));
    SendAndCheck(CmdSetInstrumentMode()(MachineStatus::OperateMode));
    SendAndCheck(CmdImmediateBusTrigger()());
    Send(CmdSendStatus()(SendMachineStatusWord));
    const MachineStatus machineStatus = Read<MachineStatus>();
    if(machineStatus.operate != MachineStatus::OperateMode)
        THROW_PSI_EXCEPTION("Unable to set a voltage = " << value.Voltage << " and compliance =" << value.Compliance
                            << ". After execution of all required commands Keithley is still not in the Operate Mode.");
    Send(CmdSendStatus()(SendComplianceValue));
    ComplianceValue compliance = Read<ComplianceValue>();
    IVoltageSource::Measurement measurement = Measure();

    return Value(measurement.Voltage, compliance.CurrentCompliance);
}

psi::ElectricPotential psi::Keithley237::Accuracy(const psi::ElectricPotential&)
{
    return ACCURACY;
}

psi::IVoltageSource::Measurement psi::Keithley237::Measure()
{
    Keithley237Internals::Measurement m = Read<Keithley237Internals::Measurement>();
    return IVoltageSource::Measurement(m.Current, m.Voltage, m.Compliance);
}

void psi::Keithley237::Off()
{
    SendAndCheck(CmdSetInstrumentMode()(MachineStatus::StandbyMode));
}

void psi::Keithley237::Send(const std::string& command, bool execute)
{
    try
    {
        (*gpibStream) << command;
        if(execute)
            (*gpibStream) << CmdExecute()();
        gpibStream->flush();
    }
    catch(std::ios_base::failure& e)
    {
        THROW_PSI_EXCEPTION("Unable to send a command to the Keithley. Command = '" << command << "'. " << std::endl
                            << e.what() << std::endl << GpibDevice::GetReportMessage());
    }
}

void psi::Keithley237::SendAndCheck(const std::string& command)
{
    static const std::string ERROR_MESSAGE_FORMAT = "Keithley reported %1% after executing the last command ="
            " '%2%'.\n%3%";
    Send(command, true);
    Send(CmdSendStatus()(SendErrorStatus));
    const ErrorStatus errorStatus = Read<ErrorStatus>();
    if(errorStatus.HasErrors())
        THROW_PSI_EXCEPTION(boost::format(ERROR_MESSAGE_FORMAT) % "an error" % command % errorStatus.GetErrorMessage());
    Send(CmdSendStatus()(SendWarningStatus));
    const WarningStatus warningStatus = Read<WarningStatus>();
    if(warningStatus.HasWarnings())
        THROW_PSI_EXCEPTION(boost::format(ERROR_MESSAGE_FORMAT) % "a warning" % command
                            % warningStatus.GetWarningMessage());
}

std::string psi::Keithley237::ReadString()
{
    try
    {
        std::string str;
        (*gpibStream) >> str;
        return str;
    }
    catch(std::ios_base::failure& e)
    {
        THROW_PSI_EXCEPTION("Unable to read a data from the Keithley. " << std::endl << e.what());
    }
}

static Range<unsigned>::ValueRangeMap CreateFilterModes()
{
    typedef Range<unsigned>::ValueRangeMap Map;
    Map m;
    for(Map::left_key_type n = 0; n < 6; ++n)
        m.insert(Map::value_type(n, 1 << n));
    return m;
}
const Range<unsigned> psi::Keithley237::Configuration::FilterModes(CreateFilterModes(), 1, "Filter",
        "readings to average");

static Range<psi::Time, unsigned, double>::ValueRangeMap CreateIntegrationTimeModes()
{
    typedef Range<psi::Time, unsigned, double>::ValueRangeMap Map;
    Map m;
    m.insert(Map::value_type(0, 416));
    m.insert(Map::value_type(1, 4000));
    m.insert(Map::value_type(2, 16670));
    m.insert(Map::value_type(3, 20000));
    return m;
}
const Range<psi::Time, unsigned, double> psi::Keithley237::Configuration::IntegrationTimeModes
(CreateIntegrationTimeModes(), 1e-6 * psi::seconds, "Integration Time", "interval");

psi::Keithley237::Configuration::Configuration(const std::string& _deviceName, bool _goLocalOnDestruction,
        unsigned numberOfReadingsToAverage, psi::Time integrationTime)
    : deviceName(_deviceName), goLocalOnDestruction(_goLocalOnDestruction),
      filterMode(FilterModes.FindMode(numberOfReadingsToAverage)),
      integrationTimeMode(IntegrationTimeModes.FindMode(integrationTime)) {}
