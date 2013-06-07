/*!
 * \file Keithley237.h
 * \brief Definition of Keithley237 class.
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 */

#pragma once

#include "IVoltageSource.h"
#include "GpibStream.h"
#include "Keithley237Internals.h"

namespace psi {
/*!
 * \brief Controls Keithley 237 high voltage source over USB-GPIB.
 *
 * This class designed to work with GpibStream that uses linux-gpib driver (see http://linux-gpib.sourceforge.net/).
 * To establish a proper connection to the Keithley the linux-gpib driver should be correctly configured.
 * Here is an example of the device entry inside the driver configuration file '/etc/gpib.conf' that was tested with
 * Keithley 237:
 *
 * device {
 *   minor = 0
 *	 name = "keithley"
 *	 pad = 20
 *	 sad = 0
 *	 eos = 0xa
 *	 set-reos = no
 *	 set-bin = yes
 * }
 *
 */
class Keithley237 : public IVoltageSource {
public:
    /// Maximal compliance value that can be set on the Keithley in Amperes.
    static const ElectricCurrent MAX_COMPLIANCE;

    /// The accuracy of the Keithley.
    static const ElectricPotential ACCURACY;

    class Configuration;
public:
    /*!
     * \brief Keithley237 constructor
     * \param configuration - all configuration parameters that are required to initialize the Keithley.
     */
    Keithley237(const Configuration& configuration);

    /*!
     * \brief Keithley237 destructor.
     * It returns Keithley to the default conditions and switches it to the local mode.
     */
    virtual ~Keithley237();

    /// \copydoc IVoltageSource::Set
    virtual Value Set(const Value& value);

    /// \copydoc IVoltageSource::Accuracy
    virtual ElectricPotential Accuracy(const ElectricPotential& voltage);

    /// \copydoc IVoltageSource::Measure
    virtual IVoltageSource::Measurement Measure();

    /// \copydoc IVoltageSource::Off
    virtual void Off();

private:
    /*!
     * \brief Prepare Keithley to receive remote commands.
     *
     * This method is a workaround for the problem that Keithley does not receives the first send command.
     */
    void Prepare();
    /*!
     * \brief Send a command string to the Keithley.
     * \throw psi_exception if some errors occured while sending a command.
     * \param command - a command string to send.
     * \param execute - indicates if command should be immediately executed by the Keithley.
     */
    void Send(const std::string& command, bool execute = true);

    /*!
     * \brief Send a command string followed by execute command to the Keithley and then check error and warning status.
     * \throw psi_exception if some errors occured while sending a command or if after the command execution Keithley
     *                      signalized about some errors or some warnings.
     * \param command - a command string to send.
     */
    void SendAndCheck(const std::string& command);

    /*!
     * \brief Read a string from the Keithley.
     * \return readed string
     */
    std::string ReadString();

    /*!
     * \brief Read a quantity from the Keithley.
     * \return readed quantity
     */
    template<typename Result>
    Result Read() {
        std::string s = ReadString();
        std::stringstream s_stream;
        s_stream << s;
        Result r;
        s_stream >> r;
        return r;
    }

private:
    /// The handle of an opened GPIB device.
    boost::shared_ptr<GpibStream> gpibStream;
};

/*!
 * \brief Contains all configuration parameters that should be providet to the Keithley237's constructor.
 */
class Keithley237::Configuration {
public:
    /// Correspondance between filter mode ids and numbers of readings to average.
    static const Keithley237Internals::Range<unsigned> FilterModes;

    /// Correspondance between integration time mode ids and integration time in micro seconds.
    static const Keithley237Internals::Range<Time, unsigned, double> IntegrationTimeModes;

public:
    /*!
     * \brief Constructor
     * \throw psi_exception when there is no fiter mode or integraation time mode found for the given parameters.
     * \param deviceName - the name of the device to which Keithley is connected.
     * \param goLocalOnDestruction - indicates if the LOC signal should be send to the GPIB bus during the
     *                               destruction of Keithley object. The LOC signal switches all devices connected
     *                               to the GPIB bus to the local mode.
     * \param numberOfReadingsToAverage - the amount of filtering for each measurement.
     * \param integrationTime - the A/D hardware integration time during each measure phase in seconds.
     */
    explicit Configuration(const std::string& deviceName, bool goLocalOnDestruction = true,
                           unsigned numberOfReadingsToAverage = FilterModes.GetFirstValue(),
                           Time integrationTime = IntegrationTimeModes.GetFirstValue());

    /// Returns the name of the device to which Keithely is connected.
    const std::string& GetDeviceName() const {
        return deviceName;
    }

    /*!
     * \brief Indicates if the LOC signal should be send to the GPIB bus during the destruction of the Keithley237
     *        object.
     *
     * The LOC signal switches all devices connected to the GPIB bus to the local mode.
     */
    bool GoLocalOnDestruction() const {
        return goLocalOnDestruction;
    }

    /// Returns the amount of filtering for each measurement.
    unsigned GetNumberOfReadingsToAverage() const {
        return FilterModes.GetValue(filterMode);
    }

    /// Returns the A/D hardware integration time during each measure phase in seconds.
    Time GetIntegrationTime() const {
        return IntegrationTimeModes.GetValue(integrationTimeMode);
    }

    /// Returns the filter mode id.
    unsigned GetFilterMode() const {
        return filterMode;
    }

    /// Returns the integration time mode id.
    unsigned GetIntegrationTimeMode() const {
        return integrationTimeMode;
    }

private:
    /// The name of the device to which Keithley is connected.
    std::string deviceName;

    /// Indicates if the LOC signal should be send to the GPIB bus during the destruction of Keithley object.
    bool goLocalOnDestruction;

    /// The filter mode id.
    unsigned filterMode;

    /// The integration time mode id.
    unsigned integrationTimeMode;
};

}
