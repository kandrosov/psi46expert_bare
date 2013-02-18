/*!
 * \file Keithley237Internals.cc
 * \brief Implementation of the elements from Keithley237Internals namespace.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 18-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Fixed bug with reading measurements when Keithley is in compliance mode.
 * 07-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - All physical values now represented using boost::units::quantity.
 * 05-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version. The code is extracted from Keithley237.cc in order to facilitate it.
 *      - Added Keithley Warning Status support.
 *      - Added class Range to store different bias and measurement ranges.
 */

#include "Keithley237Internals.h"

static Keithley237Internals::ErrorStatus::MessageMap CreateErrorMessages();
static Keithley237Internals::WarningStatus::MessageMap CreateWarningMessages();
static Keithley237Internals::RangeWithAutoMode<IVoltageSource::ElectricPotential>::ValueRangeMap CreateVoltageRanges();
static Keithley237Internals::RangeWithAutoMode<IVoltageSource::ElectricCurrent>::ValueRangeMap CreateCurrentRanges();

namespace Keithley237Internals
{
    namespace Commands
    {
        const Command< boost::mpl::vector<IVoltageSource::ElectricPotential, unsigned, unsigned> > CmdSetBias("B");
        const Command< boost::mpl::vector<SourceMode, FunctionMode> > CmdSetSourceAndFunction("F");
        const Command< boost::mpl::vector<int,
                                          MachineStatus::OutputDataFormat::Format,
                                          MachineStatus::OutputDataFormat::Lines> > CmdSetOutputDataFormat("G");
        const Command< boost::mpl::vector<> > CmdImmediateBusTrigger("H0");
        const Command< boost::mpl::vector<SelfTestCommand> > CmdSelfTests("J");
        const Command< boost::mpl::vector<IVoltageSource::ElectricCurrent, unsigned> > CmdSetCompliance("L");
        const Command< boost::mpl::vector<MachineStatus::Operate> > CmdSetInstrumentMode("N");
        const Command< boost::mpl::vector<unsigned> > CmdSetFilter("P");
        const Command< boost::mpl::vector<unsigned> > CmdSetIntegrationTime("S");
        const Command< boost::mpl::vector<StatusCommand> > CmdSendStatus("U");
        const Command< boost::mpl::vector<> > CmdExecute("X");
    }

    const RangeWithAutoMode<IVoltageSource::ElectricPotential, unsigned, double>
                          VoltageRanges(CreateVoltageRanges(), 0.1 * boost::units::si::volts, "Voltage", "limit", 0);
    const RangeWithAutoMode<IVoltageSource::ElectricCurrent, unsigned, double>
                          CurrentRanges(CreateCurrentRanges(), 1e-9 * boost::units::si::amperes, "Current", "limit", 0);

    const WarningStatus::MessageMap WarningStatus::Messages = CreateWarningMessages();
    const ErrorStatus::MessageMap ErrorStatus::Messages = CreateErrorMessages();

    template<typename Status, typename ValueType>
    std::string GetMessage(const Status& status, const std::string& firstLinePrefix, const ValueType& defaultValue)
    {
        std::stringstream output;
        output << firstLinePrefix << " = 0x" << std::hex << status.statusWord << std::dec << "." << std::endl;
        if(status.statusWord == defaultValue)
            output << Status::Messages.find(defaultValue)->second << std::endl;
        else
        {
            for(typename Status::MessageMap::const_iterator iter = Status::Messages.begin();
                iter != Status::Messages.end(); ++iter)
            {
                if(status.statusWord & iter->first)
                    output << iter->second << std::endl;
            }
        }
        return output.str();
    }

    std::string WarningStatus::GetWarningMessage() const
    {
        return GetMessage(*this, "Warning code", NoWarnings);
    }

    std::string ErrorStatus::GetErrorMessage() const
    {
        return GetMessage(*this, "Error code", NoErrors);
    }
}

/*!
 * \brief Compares sizes of two strings inside of the first item of the pair.
 * \return True if the size of the first string size is less than the size of the second string; false, otherwise.
 */
template<typename Pair>
bool compare_string_sizes(const Pair& first, const Pair& second)
{
    return first.first.size() < second.first.size();
}

/*!
 * \brief Read a string value from a stream and compare it with the expected set of values.
 * \throw psi_exception if no match found for the received string.
 * \param steam - an input stream
 * \param expected - a set of expected values for the prefix.
 * \param error_message - in case of exception, this message would be added in the begging of the error description.
 * \return An iterator pointing to the matched value or expected.end() if match does not found.
 */
template<typename Container>
static typename Container::const_iterator read_expected_string(std::istream& stream, const Container& expected,
                                                               const std::string& error_message)
{
    typename Container::const_iterator longest_expected_prefix =
            std::max_element(expected.begin(), expected.end(), compare_string_sizes<typename Container::value_type>);
    if(longest_expected_prefix == expected.end())
        return expected.end();
    size_t max_prefix_size = longest_expected_prefix->first.size();
    std::vector<char> prefix;
    prefix.assign(max_prefix_size + 1, 0);

    stream.read(&prefix[0], max_prefix_size);
    const std::string received = std::string(&prefix[0]);
    typename Container::const_iterator found_iter = expected.find(received);
    if(found_iter == expected.end())
    {
        std::stringstream s_stream;
        s_stream << error_message << "Received string is '" << received << "';";// expected strings are:";
        typename Container::const_iterator iter = expected.begin();
        if(expected.size() == 1)
            s_stream << " expected string is";
        else
            s_stream << " expected strings are:";
        s_stream << " '" << iter->first << "'";
        ++iter;
        for(; iter != expected.end(); ++iter)
            s_stream << ", '" << iter->first << "'";
        s_stream << ".";
        THROW_PSI_EXCEPTION(s_stream.str());
    }
    return found_iter;
}

/*!
 * \brief Read a string value from a stream and compare it with the expected value.
 * \throw psi_exception if received string mismathces the expected.
 * \param steam - an input stream
 * \param expected - expected prefix value
 * \param error_message - in case of exception, this message would be added in the begging of the error description.
 */
static void read_expected_string(std::istream& stream, const std::string& expected, const std::string& error_messaage)
{
    std::map<std::string, int> expected_set;
    expected_set[expected] = 0;
    read_expected_string(stream, expected_set, error_messaage);
}

/*!
 * \brief Reads a binary mask from an input stream.
 * \param stream - input stream
 * \param expected_number_of_bits - expected number of bits
 * \return readed bit mask
 */
static unsigned read_binary_mask(std::istream& stream, unsigned expected_number_of_bits)
{
    typedef std::map<char, unsigned> SymbolMap;
    static SymbolMap symbolMap;
    if(!symbolMap.size())
    {
        symbolMap['0'] = 0;
        symbolMap['1'] = 1;
    }

    if(expected_number_of_bits > 32)
        THROW_PSI_EXCEPTION("[Keithley237::read_binary_mask] Expected number of bits is too big.");

    unsigned result = 0;
    for(unsigned n = 0; n < expected_number_of_bits; ++n)
    {
        if(stream.bad())
            THROW_PSI_EXCEPTION("[Keithley237::read_binary_mask] Number of bits in stream is less than"
                                << "expected. Readed number of bits = " << n << ". Expected number of bits = "
                                << expected_number_of_bits << ".");
        char c;
        stream.get(c);
        const SymbolMap::const_iterator iter = symbolMap.find(c);
        if(iter == symbolMap.end())
            THROW_PSI_EXCEPTION("[Keithley237::read_binary_mask] Unexpected symbol '" << c << "' in the stream.");
        result = (result << 1) + iter->second;
    }
    return result;
}

using namespace Keithley237Internals;

template<typename T>
static T integer_pow(const T& x, const T& y)
{
    T result = 1;
    for(T n = 0; n < y; ++n)
        result *= x;
    return result;
}

static RangeWithAutoMode<IVoltageSource::ElectricPotential, unsigned, double>::ValueRangeMap CreateVoltageRanges()
{
    typedef RangeWithAutoMode<IVoltageSource::ElectricPotential, unsigned, double>::ValueRangeMap Map;

    Map m;
    for(Map::left_key_type n = 1; n < 5; ++n)
    {
        const Map::right_key_type v = 11 * integer_pow<Map::right_key_type>(10, n - 1);
        m.insert( Map::value_type(n, v));
    }
    return m;
}

static RangeWithAutoMode<IVoltageSource::ElectricCurrent, unsigned, double>::ValueRangeMap CreateCurrentRanges()
{
    typedef RangeWithAutoMode<IVoltageSource::ElectricCurrent, unsigned, double>::ValueRangeMap Map;

    Map m;
    for(Map::left_key_type n = 1; n < 10; ++n)
    {
        const Map::right_key_type v = integer_pow<Map::right_key_type>(10, n - 1);
        m.insert(Map::value_type(n, v));
    }
    return m;
}

/*!
 * \brief Read enum value stored as an integer number from an input stream.
 * \param s - an input stream.
 * \param e - an enum variable where the result will be stored.
 */
template<typename Enum>
static void read_enum(std::istream& s, Enum& e)
{
    int value;
    s >> value;
    e = (Enum)value;
}

std::istream& operator >>(std::istream& s, Keithley237Internals::Measurement& m)
{
    static const std::string SEPARATOR = ",";
    static const std::string EXPECTED_SOURCE_PREFIX_NORMAL_MODE = "NSDCV";
    static const std::string EXPECTED_SOURCE_PREFIX_COMPLIANCE_MODE = "OSDCV";
    static const std::string EXPECTED_MEASUREMENT_PREFIX_NORMAL_MODE = "NMDCI";
    static const std::string EXPECTED_MEASUREMENT_PREFIX_COMPLIANCE_MODE = "OMDCI";
    typedef std::map<std::string, bool> PrefixMap;
    static PrefixMap EXPECTED_MEASUREMENT_PREFIXES;
    if(!EXPECTED_MEASUREMENT_PREFIXES.size())
    {
        EXPECTED_MEASUREMENT_PREFIXES[EXPECTED_MEASUREMENT_PREFIX_NORMAL_MODE] = false;
        EXPECTED_MEASUREMENT_PREFIXES[EXPECTED_MEASUREMENT_PREFIX_COMPLIANCE_MODE] = true;
    }
    static PrefixMap EXPECTED_SOURCE_PREFIXES;
    if(!EXPECTED_SOURCE_PREFIXES.size())
    {
        EXPECTED_SOURCE_PREFIXES[EXPECTED_SOURCE_PREFIX_NORMAL_MODE] = false;
        EXPECTED_SOURCE_PREFIXES[EXPECTED_SOURCE_PREFIX_COMPLIANCE_MODE] = true;
    }

    PrefixMap::const_iterator received_iter =
        read_expected_string(s, EXPECTED_SOURCE_PREFIXES, "[Keithley237Internals::Measurement] Unable to parse a source"
                         " prefix from the output of the device.");
    m.Compliance = received_iter->second;

    double voltage;
    s >> voltage;
    m.Voltage = voltage * ParameterFormatter<IVoltageSource::ElectricPotential>::UnitsFactor();
    read_expected_string(s, SEPARATOR, "[Keithley237Internals::Measurement] Unexpected separator between source and"
                         " measurement values.");

//    PrefixMap::const_iterator received_iter =
            read_expected_string(s, EXPECTED_MEASUREMENT_PREFIXES, "[Keithley237Internals::Measurement] Unable to parse"
                                 " a measurement prefix from the output of the device.");
//    m.Compliance = received_iter->second;

    double current;
    s >> current;
    m.Current = current * ParameterFormatter<IVoltageSource::ElectricCurrent>::UnitsFactor();
    return s;
}

std::istream& operator >>(std::istream& s, Keithley237Internals::ComplianceValue& c)
{
    static const std::string EXPECTED_COMPLIANCE_PREFIX = "ICP";
    read_expected_string(s, EXPECTED_COMPLIANCE_PREFIX, "[Keithley237Internals::ComplianceValue] Unable to parse a"
                         " compliance prefix from the output of the device.");
    double value;
    s >> value;
    c.CurrentCompliance = value * ParameterFormatter<IVoltageSource::ElectricCurrent>::UnitsFactor();
    return s;
}

std::istream& operator >>(std::istream& s, Keithley237Internals::ErrorStatus& e)
{
    static const std::string EXPECTED_ERROR_STRING_PREFIX = "ERS";
    static const unsigned NUMBER_OF_EXPECTED_BITS = 26;

    read_expected_string(s, EXPECTED_ERROR_STRING_PREFIX, "[Keithley237Internals::ErrorStatus] Unable to parse an error"
                         " string prefix from the output of the device.");

    e.statusWord = read_binary_mask(s, NUMBER_OF_EXPECTED_BITS);
    return s;
}

std::istream& operator >>(std::istream& s, Keithley237Internals::WarningStatus& w)
{
    static const std::string EXPECTED_WARNING_STRING_PREFIX = "WRS";
    static const unsigned NUMBER_OF_EXPECTED_BITS = 10;

    read_expected_string(s, EXPECTED_WARNING_STRING_PREFIX, "[Keithley237Internals::WarningStatus] Unable to parse a"
                         " warning string prefix from the output of the device.");

    w.statusWord = read_binary_mask(s, NUMBER_OF_EXPECTED_BITS);
    return s;
}

std::istream& operator >>(std::istream& s, Keithley237Internals::MachineStatus& m)
{
    static const std::string ERROR_MESSAGE_PREFIX = "[Keithley237Internals::MachineStatus] ";
    static const std::string BAD_PREFIX_MESSAGE_FORMAT = ERROR_MESSAGE_PREFIX + "Unable to parse a %1% prefix from the"
            " output of the device.";

    static const std::string SEPARATOR = ",";
    static const std::string BAD_SEPARATOR_MESSAGE = ERROR_MESSAGE_PREFIX + "Unexpected separator between machine"
            " status values.";

    static const std::string EXPECTED_STATUS_STRING_PREFIX = "MSTG";
    static const std::string BAD_STATUS_STRING_PREFIX_MESSAGE =
            (boost::format(BAD_PREFIX_MESSAGE_FORMAT) % "machine status string").str();

    static const std::string EXPECTED_EOI_PREFIX = "K";
    static const std::string BAD_EOI_PREFIX_MESSAGE =
            (boost::format(BAD_PREFIX_MESSAGE_FORMAT) % "EOI and Bus Hold-off").str();

    static const std::string EXPECTED_SRQ_PREFIX = "M";
    static const std::string BAD_SRQ_PREFIX_MESSAGE =
            (boost::format(BAD_PREFIX_MESSAGE_FORMAT) % "SRQ Mask and Compliance Select").str();

    static const std::string EXPECTED_OPERATE_PREFIX = "N";
    static const std::string BAD_OPERATE_PREFIX_MESSAGE =
            (boost::format(BAD_PREFIX_MESSAGE_FORMAT) % "Operate").str();

    static const std::string EXPECTED_TRIGGER_CONTROL_PREFIX = "R";
    static const std::string BAD_TRIGGER_CONTROL_PREFIX_MESSAGE =
            (boost::format(BAD_PREFIX_MESSAGE_FORMAT) % "Trigger Control").str();

    static const std::string EXPECTED_TRIGGER_CONFIGURATION_PREFIX = "T";
    static const std::string BAD_TRIGGER_CONFIGURATION_PREFIX_MESSAGE =
            (boost::format(BAD_PREFIX_MESSAGE_FORMAT) % "Trigger Configuration").str();

    static const std::string EXPECTED_RANGE_CONTROL_PREFIX = "V";
    static const std::string BAD_RANGE_CONTROL_PREFIX_MESSAGE =
            (boost::format(BAD_PREFIX_MESSAGE_FORMAT) % "Range Control").str();

    static const std::string EXPECTED_TERMINATOR_PREFIX = "Y";
    static const std::string BAD_TERMINATOR_PREFIX_MESSAGE =
            (boost::format(BAD_PREFIX_MESSAGE_FORMAT) % "Terminator").str();

    read_expected_string(s, EXPECTED_STATUS_STRING_PREFIX, BAD_STATUS_STRING_PREFIX_MESSAGE);
    read_enum(s, m.outputDataFormat.items);
    read_expected_string(s, SEPARATOR, BAD_SEPARATOR_MESSAGE);
    read_enum(s, m.outputDataFormat.format);
    read_expected_string(s, SEPARATOR, BAD_SEPARATOR_MESSAGE);
    read_enum(s, m.outputDataFormat.lines);
    read_expected_string(s, EXPECTED_EOI_PREFIX, BAD_EOI_PREFIX_MESSAGE);
    read_enum(s, m.eoiAndBusHoldoff);
    read_expected_string(s, EXPECTED_SRQ_PREFIX, BAD_SRQ_PREFIX_MESSAGE);
    read_enum(s, m.srqMaskAndComplianceSelect.mask);
    read_expected_string(s, SEPARATOR, BAD_SEPARATOR_MESSAGE);
    read_enum(s, m.srqMaskAndComplianceSelect.compliance);
    read_expected_string(s, EXPECTED_OPERATE_PREFIX, BAD_OPERATE_PREFIX_MESSAGE);
    read_enum(s, m.operate);
    read_expected_string(s, EXPECTED_TRIGGER_CONTROL_PREFIX, BAD_TRIGGER_CONTROL_PREFIX_MESSAGE);
    read_enum(s, m.triggerControl);
    read_expected_string(s, EXPECTED_TRIGGER_CONFIGURATION_PREFIX, BAD_TRIGGER_CONFIGURATION_PREFIX_MESSAGE);
    read_enum(s, m.triggerConfiguration.origin);
    read_expected_string(s, SEPARATOR, BAD_SEPARATOR_MESSAGE);
    read_enum(s, m.triggerConfiguration.triggerIn);
    read_expected_string(s, SEPARATOR, BAD_SEPARATOR_MESSAGE);
    read_enum(s, m.triggerConfiguration.triggerOut);
    read_expected_string(s, SEPARATOR, BAD_SEPARATOR_MESSAGE);
    read_enum(s, m.triggerConfiguration.sweepEndTriggerOut);
    read_expected_string(s, EXPECTED_RANGE_CONTROL_PREFIX, BAD_RANGE_CONTROL_PREFIX_MESSAGE);
    read_enum(s, m.v1100RangeControl);
    read_expected_string(s, EXPECTED_TERMINATOR_PREFIX, BAD_TERMINATOR_PREFIX_MESSAGE);
    read_enum(s, m.terminator);

    return s;
}

static ErrorStatus::MessageMap CreateErrorMessages()
{
    ErrorStatus::MessageMap messages;
    messages[ErrorStatus::TriggerOverrun] = "Trigger Overrun - Set when the instrument receives a trigger while it is"
            " still processing the action of a previous trigger. The TRIG OVERRUN message is displayed and the"
            " additional trigger is ignored.";

    messages[ErrorStatus::IDDC] = "IDDC - Set when an illegal device-dependent command (IDDC) is received (such as the"
            " \"E\" in E1X). The IDDC message is displayed; commands up to and including the next X are ignored.";

    messages[ErrorStatus::IDDCO] = "IDDCO - Set when an illegal device-dependent command option (IDDCO) is received"
            " (such as the \"3\" in F3,0X). The IDDCO message is displayed; commands up to and including the next X are"
            " ignored.";

    messages[ErrorStatus::InterlockPresent] = "Interlock Present - Set when one of two error conditions exists:\n"
            "• An operate/standby command (N) was attempted when an interlock condition was present. The"
            " INTERLOCK PRESENT message is displayed and the N command is ignored.\n"
            "• An interlock condition presents itself when the unit is in operate. The INTERLOCK PRESENT message is"
            " displayed and the source-measure unit goes to standby.";

    messages[ErrorStatus::IllegalMeasureRange] = "Illegal Measure Range - Set when the selected measure range is"
            " incompatible with the source and instrument (L command). Valid range values are:\n"
            "• 0-4 (measure V on 237)\n"
            "• 0-3 (measure V on 236 or 238)\n"
            "• 0-9 (measure I on 236 or 237)\n"
            "• 0-10 (measure I on 238)\n"
            "The instrument displays an ILLEGAL MEAS RANGE message; pending DOCs are ignored; no execution action is"
            " taken.";

    messages[ErrorStatus::IllegalSourceRange] = "Illegal Source Range - Set when the selected source range is"
            " incompatible with the source and instrument (A, B, and Q commands). Valid range values are:\n"
            "• 0-4 (source V on 237)\n"
            "• 0-3 (source V on 236 or 238)\n"
            "• 0-9 (source I on 236 or 237)\n"
            "• 0-10 (source I on 238)\n"
            "The instrument displays an ILLEGAL SRC RANGE message; pending DDCs are ignored; no execution action is"
            " taken.";

    messages[ErrorStatus::InvalidSweepMix] = "Invalid Sweep Mix - Set when you create a sweep from one group, then try"
            " to append a sweep from the other group. (The sweep types are grouped as pulsed and non-pulsed.) The"
            " instrument displays an INVALID SWEEP MIX message; pending DOCs are ignored; no execution action is taken."
            " Affected DDC is Q.";

    messages[ErrorStatus::LogCannotCrossZero] = "Log Cannot Cross Zero - Set when you attempt to create or append a log"
            " sweep that passes through zero. (Since zero is assumed to be positive, a log sweep cannot have a negative"
            " start value and a zero stop value.) The instrument displays a LOG CANNOT CROSS 0 message; pending DDCs"
            " are ignored; no execution action is taken. Affected DOC is Q.";

    messages[ErrorStatus::AutorangingSourceWithPulseSweep] = "Autoranging Source with Pulse Sweep - Set when you try to"
            " select autoranging of the source in a pulse sweep (Q commands). The instrument displays a NO AUTO IN"
            " PULSE message; pending DDCs are ignored; no execution action is taken.";

    messages[ErrorStatus::InCalibration] = "In Calibration - Set when non-calibration commands are requested while in"
            " calibration mode. The instrument displays an IN CAL message; pending DDCs are ignored except for N and O;"
            " the instrument stays in calibration mode. Non-affected DDCs are C, N, 0, and U.";

    messages[ErrorStatus::InStandby] = "In Standby - Set when a calibration command is requested while in standby mode"
            " (C command). The instrument displays an IN STBY message; the calibration command is ignored; the"
            " instrument stays in standby mode.";

    messages[ErrorStatus::UnitIsA236] = "Unit is a 236 - Set when 1.1kV calibration commands are sent to a Model 236"
            " (C command). The instrument displays a 236 UNIT message; the calibration command is ignored; the"
            " instrument stays in calibration mode. This character is always zero for the Models 237/238.";

    messages[ErrorStatus::IOU_DPRAM_Failed] = "IOU DPRAM Failed - Set when the dual-port RAM in the I/O controller of"
            " the instrument failed. The message IOU DPRAM FAILED is displayed; the unit is not functional.";

    messages[ErrorStatus::IOU_EEROM_Failed] = "IOU EEROM Failed - Set when the electrically erasable ROM in the I/O"
            " controller of the instrument failed. The message IOU EEROM FAILED is displayed; the unit is not"
            " functional.";

    messages[ErrorStatus::IOU_CalChecksumError] = "IOU Cal. Checksum Error - Set when the JlX or power-up checksum test"
            " of the IOU cal constants failed. The message IOU CAL CHECKSUM is displayed. Any cal constants outside"
            " predetermined limits are replaced with default values (see Cal Constants Error).";

    messages[ErrorStatus::DPRAMLockup] = "DPRAM Lockup - Set when there is a ROM or RAM failure in the source/measure"
            " controller so that it is not responding to the I/O controller. The message DPRAM LOCKUP is displayed; the"
            " unit is not functional.";

    messages[ErrorStatus::DPRAMLinkError] = "DPRAM Link Error - Set when there is a communication error in the dual"
            " port RAM between the I/O controller and the source/measure controller. The message DPRAM LINK ERR is"
            " displayed; the unit is not functional.";

    messages[ErrorStatus::CalADCZeroError] =
    messages[ErrorStatus::CalADCGainError] =
    messages[ErrorStatus::CalSRCZeroError] =
    messages[ErrorStatus::CalSRCGainError] = "Cal. ADC Zero Error, Cal. ADC Gain Error, Cal. SRC Zero Error, Cal. SRC"
            " Gain Error - Set when the newly created cal constant is outside the expected value for A/D converter"
            " zero, A/D converter gain, source zero, or source gain. A message is displayed (CAL ADC ZERO, CAL ADC"
            " GAIN, CAL SRC ZERO, or CAL SRC GAIN), and a default cal constant value is substituted.";

    messages[ErrorStatus::CalCommonModError] = "Cal. Common Mode Error - Set when there is an error in calibrating the"
            " common mode adjustment. The CAL MODE ERR message is displayed.";

    messages[ErrorStatus::CalComplianceError] = "Cal. Compliance Error - Set if compliance occurs during a calibration"
            " procedure. The CAL COMPLI message is displayed.";

    messages[ErrorStatus::CalValueError] = "Cal. Value Error - Set when the entered value of a cal constant is outside"
            " the expected value for the present step. The message CAL VALUE ERR is displayed; check the specified"
            " value sent with the C command and re-enter.";

    messages[ErrorStatus::CalConstError] = "Cal. Constants Error - Set when the power-up verification of the cal"
            " constants finds one or more values outside of predetermined limits. The message CAL CONSTANTS ERR is"
            " displayed; default value(s) are used.";

    messages[ErrorStatus::CalInvalidError] = "Cal. Invalid Error - Set when one or more cal errors (Cal. ADC Zero, Cal."
            " ADC Gain, Cal. SRC Zero, Cal. SRC Gain, Cal. Mode, Cal. Compliance) are present on power-up, factory"
            " initialization DCL, or SDC. The message CAL INVALID ERR is displayed.";

    messages[ErrorStatus::NoErrors] = "No errors";
    return messages;
}


static WarningStatus::MessageMap CreateWarningMessages()
{
    WarningStatus::MessageMap messages;
    messages[WarningStatus::Uncalibrated] = "Uncalibrated - Set when the unit detects that illegal cal constants are"
            " stored in EEROM. The message UNCALIBRATED is displayed; the Source/Measure LED blinks.";

    messages[WarningStatus::TemporaryCal] = "Temporary Cal - Set when calibration mode is entered or exited when the"
            " CAL LOCK button is in the LOCKED position. The message TEMPORARY CAL is displayed; entered cal constants"
            " are saved in volatile RAM upon exit of cal procedure (C59X) and thus are cleared when power is cycled.";

    messages[WarningStatus::ValueOutOfRange] = "Value Is Out of Range - Set when one of the following conditions"
            " exist:\n"
            "• The selected de or sweep source value exceeds the source range.\n"
            "• The selected compliance value exceeds the measurement range.\n"
            "• The step value is too small for the selected range.\n"
            "• The step value is more than twice the selected range.\n"
            "The message VALUE OUT OF RANGE is displayed. With the first two conditions, the maximum (signed) value for"
            " the specified range is used. For the third condition, a value of zero is used. (A step of zero yields a"
            " buffer filled with one source value for linear stair sweeps or two source values for linear stair pulsed"
            " sweeps.) With the last condition, a step value equal to twice the selected range is used (e.g., 22V for"
            " the 11 V range).";

    messages[WarningStatus::SweepBufferFilled] = "Sweep Buffer Filled - Set when the number of generated points exceeds"
            " the buffer capacity (Q command). A BUFFER FILLED message is displayed; generated points are preserved up"
            " to buffer capacity of 1000.";

    messages[WarningStatus::NoSweepPoints] = "No Sweep Points; Must Create Sweep - Set when an attempt is made to"
            " modify a non-existent sweep with an A command or append a non-existent sweep with a Q6 through Q11"
            " command. Also set if a trigger for a sweep occurs and there are no sweep points. The message MUST CREATE"
            " SWEEP is displayed; no sweep points are generated.";

    messages[WarningStatus::PulseTimesNotMet] = "Pulse Times Not Met - Set when the specified pulse times (t_ON and"
            " t_OFF) cannot be met (P, Q, S and W commands). Also, for pulsed sweeps, set if autorange measure is"
            " programmed (L command) and the pulse times might not be not long enough. The message PULSE TIME NOT MET"
            " is displayed, and, if necessary, the t_ON/t_OFF times will be extended to the minimum needed to support"
            " the waveform create/append or instrument configuration.";

    messages[WarningStatus::NotInRemote] = "Not in Remote - Set when an X command is received over the bus but the"
            " Model 236/237/238 is not in remote. The NOT IN REMOTE message is displayed.";

    messages[WarningStatus::MeasureRangeChangedDueToRangeSelect] = "Measure Range Changed Due to 1.1kV/1OOmA or 11OV/1A"
            " Range Select - Set when the source and measure ranges are in conflict (A, B, F, L, and Q commands). The"
            " selected ranges cannot be 1.1kV and 100mA (237), or 110V and 1A (238) simultaneously, either when"
            " sourcing voltage, measuring current, or when sourcing current, measuring voltage for either de or sweep"
            " operation. The instrument displays a MEAS RANGE CHANGED message and the measure range is lowered. A VALUE"
            " OUT OF RANGE message is displayed as the compliance value is applied to the lower range.";

    messages[WarningStatus::MeasurementOverflowOrSweepAborted] = "Measurement Overflow/Sweep Aborted - Set when an"
            " external source of power has overloaded the measurement hardware of the Model 236/237/238. A measurement"
            " overflow is indicated at 112.5% of full scale range. The message OFLO is displayed in place of the"
            " measurement value. Also set when a sweep has aborted (e.g., in response to a command sent while a sweep"
            " is in progress).";

    messages[WarningStatus::PendingTrigger] = "Pending Trigger - Set when the unit is triggered while it is processing"
            " a previous command (e.g., if a front panel trigger is received during the bus hold-off time of a Q"
            " command, or if a bus trigger is received while sweep points are being generated from the front panel). A"
            " PENDING TRIGGER message is displayed and the trigger is saved until it can be acted upon.";

    messages[WarningStatus::NoWarnings] = "No Warnings.";
    return messages;
}
