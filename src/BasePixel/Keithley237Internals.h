/*!
 * \file Keithley237Internals.h
 * \brief Definition of the elements from Keithley237Internals namespace.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 25-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - IVoltageSource and Keithley237 moved into psi namespace.
 *      - Switched to ElectricPotential and ElectricCurrent defined in PsiCommon.h.
 * 07-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - All physical values now represented using boost::units::quantity.
 * 06-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Added some code documentation.
 *      - Added support of Keithley Status - Compliance Value.
 * 05-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version. The code is extracted from Keithley237.h in order to facilitate it.
 *      - Added Keithley Warning Status support.
 *      - Added class Range to store different bias and measurement ranges.
 */

#pragma once

#include <sstream>
#include <map>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/at.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bimap.hpp>
#include <boost/format.hpp>

#include "psi_exception.h"
#include "IVoltageSource.h"

/// Macros to generate a type definition inside the Keithley237Internals::Command class.
#define KEITHLEY237_COMMAND_CREATOR_TYPEDEF(z, n, tuple) \
    typedef typename boost::mpl::at< ParameterList, boost::mpl::int_<n> >::type Param##n;

/// Macros to generate a command output inside the Keithley237Internals::Command::_Creator class.
#define KEITHLEY237_COMMAND_CREATOR_OUTPUT_PARAM(z, n, tuple) \
    s_name << BOOST_PP_IF(n, "," << , ) ParameterFormatter< Param##n >::ToString(p##n);

/*!
 * \brief Macros to generate a specialization of the Keithley237Internals::Command::_Creator class with \a n parameters.
 * It creates a _Creator constructor which takes a reference to the corresponding Keithley237Internals::Command object
 * and defines operator () with number of arguments equival \a n with types that matches the types specified it the
 * ParameterList of Keithley237Internals::Command. Operator () returns a std::string that is ready to be send to the
 * Keithley.
 */
#define KEITHLEY237_DEFINE_CREATOR(z, n, tuple) \
    template<typename T> \
    class _Creator<n,T> \
    { \
    private: \
        const Command& cmd; \
    public: \
        BOOST_PP_REPEAT_FROM_TO(0, n, KEITHLEY237_COMMAND_CREATOR_TYPEDEF, ()) \
        _Creator(const Command& _cmd) : cmd(_cmd) {} \
        std::string operator()(BOOST_PP_ENUM_BINARY_PARAMS_Z(z, n, const Param, &p)) const \
        { \
            std::stringstream s_name; \
            s_name << cmd.GetName(); \
            BOOST_PP_REPEAT_FROM_TO(0, n, KEITHLEY237_COMMAND_CREATOR_OUTPUT_PARAM, ())		\
            return s_name.str(); \
        } \
    };

namespace psi {
namespace Keithley237Internals {

template<typename V>
struct ParameterFormatter
{
    static std::string ToString(const V& v)
    {
        std::stringstream s;
        s << std::scientific << v;
        return s.str();
    }
};

template<>
struct ParameterFormatter<ElectricPotential>
{
    static const ElectricPotential& UnitsFactor()
    {
        static const ElectricPotential factor = 1.0 * volts;
        return factor;
    }

    static std::string ToString(const ElectricPotential& p)
    {
        std::stringstream s;
        const double v = p / UnitsFactor();
        s << std::scientific << v;
        return s.str();
    }
};

template<>
struct ParameterFormatter<ElectricCurrent>
{
    static const ElectricCurrent& UnitsFactor()
    {
        static const ElectricCurrent factor = 1.0 * amperes;
        return factor;
    }

    static std::string ToString(const ElectricCurrent& c)
    {
        std::stringstream s;
        const double v = c / UnitsFactor();
        s << std::scientific << v;
        return s.str();
    }
};

/*!
 * \brief Represents a Keithley237 command.
 * A detailed command description can be found in the "Keithley Model 236/237/238 Source Measure Units Operator's
 * Manual Rev. E" Section 3 (http://www.keithley.com/data?asset=874).
 */
template<typename ParameterList>
class Command
{
public:
    /*!
     * \brief Creates a command string that is ready to be send to the Keithely.
     * \return command string.
     */
    template<unsigned N, typename T=unsigned>
    class _Creator {};

    BOOST_PP_REPEAT_FROM_TO(0, BOOST_PP_INC(3), KEITHLEY237_DEFINE_CREATOR, () )

    /// Type definition for the appropriate _Creator specialization.
    typedef _Creator< boost::mpl::size<ParameterList>::value > Creator;

public:
    /*!
     * \brief Command constructor.
     * \param _name - name of the command as it will be send to the Keithley.
     */
    Command(const std::string& _name) : name(_name), creator(*this) {}

    /// Returns a name of the command.
    const std::string& GetName() const { return name; }

    /// Returns a reference to the command creator that can be used to generate a command string.
    const Creator& operator ()() const
    {
        return creator;
    }

private:
    /// Name of the command as it will be send to the Keithley.
    std::string name;

    /// A command string creator.
    Creator creator;
};

/*!
 * \brief Measurement result container.
 */
struct Measurement
{
    /// Current in Amperes.
    ElectricCurrent Current;

    /// Voltage in Volts.
    ElectricPotential Voltage;

    /// Indicates if device is in compliance mode.
    bool Compliance;

    /// Default constructor.
    Measurement() : Compliance(false) {}

    /// Constructor.
    Measurement(ElectricCurrent current, ElectricPotential voltage, bool compliance)
        : Current(current), Voltage(voltage), Compliance(compliance) {}
};

/// Keithley 237 compliance value.
struct ComplianceValue
{
    /// Current compliance in Amperes.
    ElectricCurrent CurrentCompliance;

    /// Default constructor.
    ComplianceValue() {}

    /// Constructor.
    ComplianceValue(const ElectricCurrent& currentCompliance)
        : CurrentCompliance(currentCompliance) {}
};

/*!
 * \brief Represents an Error Status Word of the Keithley 237.
 */
struct ErrorStatus
{
    /// Enumaration of the possible error status codes.
    enum Errors { NoErrors = 0, CalInvalidError = 0x1, CalConstError = 0x2, CalValueError = 0x4,
                  CalComplianceError = 0x8, CalCommonModError = 0x10, CalSRCGainError = 0x20,
                  CalSRCZeroError = 0x40, CalADCGainError = 0x80, CalADCZeroError = 0x100,
                  DPRAMLinkError = 0x200, DPRAMLockup = 0x400, IOU_CalChecksumError = 0x800,
                  IOU_EEROM_Failed = 0x1000, IOU_DPRAM_Failed = 0x2000, UnitIsA236 = 0x4000, InStandby = 0x8000,
                  InCalibration = 0x10000, AutorangingSourceWithPulseSweep = 0x20000, LogCannotCrossZero = 0x40000,
                  InvalidSweepMix = 0x80000, IllegalSourceRange = 0x100000, IllegalMeasureRange = 0x200000,
                  InterlockPresent = 0x400000, IDDCO = 0x800000, IDDC = 0x1000000, TriggerOverrun = 0x2000000 };

    /// Type definition for Error Status message map.
    typedef std::map<ErrorStatus::Errors, std::string> MessageMap;

    /// Correspondace between error codes and their descriptions.
    static const MessageMap Messages;

    /// Binary representation of the Error Status Word.
    int statusWord;

    /// Default constructor.
    ErrorStatus() : statusWord(NoErrors) {}

    /// Constructor.
    ErrorStatus(unsigned _statusWord) : statusWord(_statusWord) {}

    /// Indicates if there are some errors.
    bool HasErrors() const { return statusWord != NoErrors; }

    /// Returns an error message.
    std::string GetErrorMessage() const;
};

/*!
 * \brief Represents an Warning Status Word of the Keithley 237.
 */
struct WarningStatus
{
    /// Enumaration of the possible warning status codes.
    enum Warnings { NoWarnings = 0, PendingTrigger = 0x1, MeasurementOverflowOrSweepAborted = 0x2,
                    MeasureRangeChangedDueToRangeSelect = 0x4, NotInRemote = 0x8, PulseTimesNotMet = 0x10,
                    NoSweepPoints = 0x20, SweepBufferFilled = 0x40, ValueOutOfRange = 0x80, TemporaryCal = 0x100,
                    Uncalibrated = 0x200 };

    /// Type definition for Warning Status message map.
    typedef std::map<WarningStatus::Warnings, std::string> MessageMap;

    /// Correspondace between warning codes and their descriptions.
    static const MessageMap Messages;

    /// Binary representation of the Warning Status Word.
    int statusWord;

    /// Default constructor.
    WarningStatus() : statusWord(NoWarnings) {}

    /// Constructor.
    WarningStatus(unsigned _statusWord) : statusWord(_statusWord) {}

    /// Indicates if there are some warnings.
    bool HasWarnings() const { return statusWord != NoWarnings; }

    /// Returns a warning message.
    std::string GetWarningMessage() const;
};

/*!
 * \brief Represents a Machine Status of the Keithley 237.
 */
struct MachineStatus
{
    struct OutputDataFormat
    {
        enum Items { NoItems = 0, SourceValue = 1, DelayValue = 2, MeasureValue = 4, TimeValue = 8 };
        enum Format { ASCII_Prefix_Suffix = 0, ASCII_Prefix_NoSuffix = 1, ASCII_NoPrefix_NoSuffix = 2,
                      HP_Binary = 3, IBM_Binary = 4 };
        enum Lines { OneLineFromDCBuffer = 0, OneLineFromSweepBuffer = 1, AllLinesFromSweepBuffer = 2 };

        Items items;
        Format format;
        Lines lines;
    };

    enum EOIAndBusHoldoff { EnableEOI_EnableHoldoff = 0, DisableEOI_EnableHoldoff = 1, EnableEOI_DisableHoldoff = 2,
                            DisableEOI_DisableHoldoff = 3 };

    struct SRQMaskAndComplianceSelect
    {
        enum Mask { MaskCleared = 0, Warning = 1, SweepDone = 2, TriggerOut = 4, ReadingDone = 8,
                    ReadyForTrigger = 16, Error = 32, ComplianceMask = 128 };
        enum Compliance { Delay_Measure_Idle = 0, Measurement_Compliance = 1 };

        Mask mask;
        Compliance compliance;
    };

    enum Operate { StandbyMode = 0, OperateMode = 1 };

    enum TriggerControl { DisableTriggering = 0, EnableTriggering = 1 };

    struct TriggerConfiguration
    {
        enum Origin { IEEE_X = 0, IEEE_GET = 1, IEEE_Talk = 2, External = 3, ImmediateTriggerOnly = 4 };
        enum TriggerIn { Continuous = 0, SRC_IN = 1, DLY_IN = 2, MSR_IN = 4, SinglePulse = 8 };
        enum TriggerOut { None = 0, SRC_OUT = 1, DLY_OUT = 2, MSR_OUT = 4, PulseEnd = 8 };
        enum SweepEndTriggerOut { Disable = 0, Enable = 1 };

        Origin origin;
        TriggerIn triggerIn;
        TriggerOut triggerOut;
        SweepEndTriggerOut sweepEndTriggerOut;
    };

    enum V1100RangeControl { V1100RangeDisabled = 0, V1100RangeEnabled = 1 };

    enum Terminator { CR_LF = 0, LF_CR = 1, CR = 2, LF = 3, NoTerminator = 4 };

    OutputDataFormat outputDataFormat;
    EOIAndBusHoldoff eoiAndBusHoldoff;
    SRQMaskAndComplianceSelect srqMaskAndComplianceSelect;
    Operate operate;
    TriggerControl triggerControl;
    TriggerConfiguration triggerConfiguration;
    V1100RangeControl v1100RangeControl;
    Terminator terminator;
};

/// Enumeration of the possible source operation modes of the Keithley.
enum SourceMode { SourceVoltageMode = 0, SourceCurrentMode = 1 };

/// Enumeration of the possible operation functions of the Keithley.
enum FunctionMode { DCFunction = 0, SweepFunction = 1 };

/// Enumaration of the possible status request command numbers.
enum StatusCommand { SendModelNumber = 0, SendErrorStatus = 1, SendStoredString = 2, SendMachineStatusWord = 3,
               SendMeasurementParameters = 4, SendComplianceValue = 5, SendSuppressionValue = 6,
               SendCalibrationStatus = 7, SendDefinedSweepSize = 8, SendWarningStatus = 9,
               SendFirstSweepPointInCompliance = 10, SendSweepSize = 11 };

/// Enumeration of the possible self-test command of the Keithley.
enum SelfTestCommand { RestoreFactoryDefaults = 0, PerformMemoryTest = 1, PerformDisplayTest = 2 };

/// Represents a bias or measurement range of the Keithley.
template<typename ValueType, typename ModeType = unsigned, typename DimensionlessValueType = ValueType,
         typename IntegerValueType = int64_t>
class Range
{
public:
    /// Type definition for map that contain correspondance between working mode and their actual values.
    typedef boost::bimaps::bimap<ModeType, IntegerValueType> ValueRangeMap;

public:
    /*!
     * \brief Constructor
     * \param _ranges             a bidirectional map that contains correspondance between mode types and their
     *                            associated values. Associated values are stored as an integer numbers.
     * \param _conversionFactor   a factor that should be applied to convert integer representation storred in the
     *                            \a _ranges map to the real value.
     * \param _rangeName          a name of the range that will be added to the description in case of an error.
     * \param _rangeTypeName      a name type of the range (e.g. "limit") that will be added to the description in
     *                            case of an error.
     */
    Range(const ValueRangeMap& _ranges, ValueType _conversionFactor, const std::string& _rangeName,
          const std::string& _rangeTypeName)
        : ranges(_ranges), conversionFactor(_conversionFactor), rangeName(_rangeName),
          rangeTypeName(_rangeTypeName) {}

    /*!
     * \brief Finds a working mode identificator that corresponds to the provided value.
     * \throw psi_exception if no working mode was found.
     * \param value - a value that should be found in the predefined map.
     * \return found working mode.
     */
    ModeType FindMode(ValueType value) const
    {
        static const std::string ERROR_MESSAGE_FORMAT = "[Keithley237Internals::Range::FindMode] %1% Mode with %2%"
                " %3% not found.";

        typename ValueRangeMap::right_const_iterator modeIter = ranges.right.end();
        const DimensionlessValueType storedValue = value / conversionFactor;
        if(storedValue <= std::numeric_limits<IntegerValueType>::max())
            modeIter = ranges.right.find((IntegerValueType)storedValue);
        if(modeIter == ranges.right.end())
        {
            const std::string message =
                    (boost::format(ERROR_MESSAGE_FORMAT) % rangeName % value % rangeTypeName).str();
            ReportError(message);
        }
        return modeIter->second;
    }

    /*!
     * \brief Get a value associated to the provided working mode.
     * \throw psi_exception if no working mode was found.
     * \param mode - a working mode that should be predefined in the ValueRangeMap.
     * \return value associated to the working mode in it natural units.
     */
    ValueType GetValue(ModeType mode) const
    {
        static const std::string ERROR_MESSAGE_FORMAT = "[Keithley237Internals::Range::GetValue] %1% Mode with id ="
                " %2% not found.";

        typename ValueRangeMap::left_const_iterator valueIter = ranges.left.find(mode);
        if(valueIter == ranges.left.end())
        {
            const std::string message = (boost::format(ERROR_MESSAGE_FORMAT) % rangeName % mode).str();
            ReportError(message);
        }
        const DimensionlessValueType value = valueIter->second;
        return value * conversionFactor;
    }

    /// Returns a value that associated with the first working mode in it natural units.
    ValueType GetFirstValue() const
    {
        const DimensionlessValueType value = ranges.left.begin()->second;
        return value * conversionFactor;
    }

    /// Returns a value that associated with the last working mode in it natural units.
    ValueType GetLastValue() const
    {
        const DimensionlessValueType value = ranges.left.rbegin()->second;
        return value * conversionFactor;
    }

    /// Returns the first working mode id.
    ModeType GetFirstMode() const { return ranges.left.begin()->first; }

    /// Returns the last working mode id.
    ModeType GetLastMode() const { return ranges.left.rbegin()->first; }

private:
    /*!
     * \brief Throw an exception with description of the error and with a list of possible working modes.
     * \param message - error message that will be added in the beginning of the error description.
     */
    void ReportError(const std::string& message) const
    {
        static const std::string MODE_LIST_HEADER = "Available modes are:";
        static const std::string MODE_OUTPUT_FORMAT = "Mode %1%: %2%.";

        std::stringstream s;
        s << message << std::endl << MODE_LIST_HEADER << std::endl;
        for(typename ValueRangeMap::left_const_iterator iter = ranges.left.begin(); iter != ranges.left.end();
            ++iter)
        {
            const DimensionlessValueType value = iter->second;
            const ValueType convertedValue = value * conversionFactor;
            s << boost::format(MODE_OUTPUT_FORMAT) % iter->first % convertedValue << std::endl;
        }
        THROW_PSI_EXCEPTION(s.str());
    }

private:
    /// Correspondance between range mode id and maximal absolut value that can be set in this mode.
    ValueRangeMap ranges;

    /// A factor to convert IntegerValueType to ValueType.
    ValueType conversionFactor;

    /// A name of the range.
    std::string rangeName;

    /// A name of the range type.
    std::string rangeTypeName;
};

/*!
 * \brief A range with a automatic adjustment mode.
 *
 * For the bias operation in the autorage mode the device adjust the bias range for the highest on-scale sourcing
 * possible.
 */
template<typename ValueType, typename ModeType = unsigned, typename DimensionlessValueType = ValueType,
         typename IntegerValueType = int64_t>
class RangeWithAutoMode : public Range<ValueType, ModeType, DimensionlessValueType, IntegerValueType>
{
public:
    /// A local type redefinition of Range::ValueRangeMap.
    typedef typename Range<ValueType, ModeType, DimensionlessValueType, IntegerValueType>::ValueRangeMap ValueRangeMap;

public:
    /*!
     * \copydoc Keithley237Internals::Range::Range
     * \param _autorangeModeId  an autorange working mode id. This mode may not be listed in the \a _ranges map,
     *                          because that is no fixed value associated with it.
     */
    RangeWithAutoMode(const ValueRangeMap& _ranges, ValueType _conversionFactor, const std::string& _rangeName,
          const std::string& _rangeTypeName, ModeType _autorangeModeId)
        : Range<ValueType, ModeType, DimensionlessValueType, IntegerValueType>(_ranges, _conversionFactor,
                                                                               _rangeName, _rangeTypeName),
          autorangeModeId(_autorangeModeId) {}

    /// Returns the autorange mode id.
    ModeType GetAutorangeModeId() const { return autorangeModeId; }

private:
    /// The autorange mode id.
    ModeType autorangeModeId;
};

/// Correspondance between voltage range mode id and maximal absolut value in mV that can be set in this mode.
extern const RangeWithAutoMode<ElectricPotential, unsigned, double> VoltageRanges;

/// Correspondance between current range mode id and maximal absolut value in nA that can be set in this mode.
extern const RangeWithAutoMode<ElectricCurrent, unsigned, double> CurrentRanges;

/// Contains definition of the commants that can be send to the Keithley 237.
namespace Commands
{
    /*!
     * \brief Command B - Bias.
     *
     * Purpose: To program the dc bias operation, the non-triggered sweep source value, and the t_OFF source value
     *          of pulsed sweeps.
     *
     * Parameters: level (V or A), range, delay in milliseconds (0..65000).
     */
    extern const Command< boost::mpl::vector<ElectricPotential, unsigned, unsigned> > CmdSetBias;

    /*!
     * \brief Command F - Source and Function
     *
     * Purpose: To program a source (V or I) and a function (de or sweep).
     *
     * Parameters: SourceMode, FunctionMode.
     */
    extern const Command< boost::mpl::vector<SourceMode, FunctionMode> > CmdSetSourceAndFunction;

    /*!
     * \brief Command G - Output Data Format.
     *
     * Purpose: To select the type, format, and quantity of output data transmitted over the bus.
     *
     * Parameters: binary mask of MachineStatus::OutputDataFormat::Items, MachineStatus::OutputDataFormat::Format,
     *             MachineStatus::OutputDataFormat::Lines.
     */
    extern const Command< boost::mpl::vector<int,
                                            MachineStatus::OutputDataFormat::Format,
                                            MachineStatus::OutputDataFormat::Lines> > CmdSetOutputDataFormat;

    /*!
     * \brief Command H - IEEE Immediate Trigger.
     *
     * Purpose: To provide an immediate trigger stimulus from the IEEE-488 bus.
     *
     * Format: H0X.
     */
    extern const Command< boost::mpl::vector<> > CmdImmediateBusTrigger;

    /*!
     * \brief Command J - Self-tests.
     *
     * Purpose: To restore factory defaults and test memory and front panel display.
     *
     * Parameters: SelfTestCommand.
     */
    extern const Command< boost::mpl::vector<SelfTestCommand> > CmdSelfTests;

    /*!
     * \brief Command L - Compliance.
     *
     * Purpose: To program the compliance value and compliance/measurement range.
     *
     * Parameters: level (V or A), range.
     */
    extern const Command< boost::mpl::vector<ElectricCurrent, unsigned> > CmdSetCompliance;

    /*!
     * \brief Command O - Operate.
     *
     * Purpose: To select local or remote output sensing.
     *
     * Parameters: MachineStatus::Operate.
     */
    extern const Command< boost::mpl::vector<MachineStatus::Operate> > CmdSetInstrumentMode;

    /*!
     * \brief Command P - Filter.
     *
     * Purpose: To control the number of readings averaged.
     *
     * Parameters: filter_mode.
     */
    extern const Command< boost::mpl::vector<unsigned> > CmdSetFilter;

    /*!
     * \brief Command S - Itegration Time.
     *
     * Purpose: To control the integration time and resolution.
     *
     * Parameters: integration_time_mode.
     */
    extern const Command< boost::mpl::vector<unsigned> > CmdSetIntegrationTime;

    /*!
     * \brief Command U - Status.
     *
     * Purpose: To obtain instrument status and configuration.
     *
     * Parameters: StatusCommand.
     */
    extern const Command< boost::mpl::vector<StatusCommand> > CmdSendStatus;

    /*!
     * \brief Command X - Execute.
     *
     * Purpose: To direct the Model236/237 /238 to execute device-dependent commands received since last X.
     */
    extern const Command< boost::mpl::vector<> > CmdExecute;
}
}

}

/*!
 * \brief Read Keithley237Internals::Measurement from an input stream.
 * \param s - input stream
 * \param m - measurement object
 * \return input stream
 */
std::istream& operator >>(std::istream& s, psi::Keithley237Internals::Measurement& m);

/*!
 * \brief Read Keithley237Internals::ComplianceValue from an input stream.
 * \param s - input stream
 * \param c - compliance value object
 * \return input stream
 */
std::istream& operator >>(std::istream& s, psi::Keithley237Internals::ComplianceValue& c);

/*!
 * \brief Read Keithley237Internals::ErrorStatus from an input stream.
 * \param s - input stream
 * \param e - error status object
 * \return input stream
 */
std::istream& operator >>(std::istream& s, psi::Keithley237Internals::ErrorStatus& e);

/*!
 * \brief Read Keithley237Internals::WarningStatus from an input stream.
 * \param s - input stream
 * \param e - warning status object
 * \return input stream
 */
std::istream& operator >>(std::istream& s, psi::Keithley237Internals::WarningStatus& w);

/*!
 * \brief Read Keithley237Internals::MachineStatus from an input stream.
 * \param s - input stream
 * \param m - machine status object
 * \return input stream
 */
std::istream& operator >>(std::istream& s, psi::Keithley237Internals::MachineStatus& m);
