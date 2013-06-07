/*!
 * \file GpibStream.h
 * \brief Definition GpibStream and GpibDevice classes.
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 */

#pragma once

#include <boost/iostreams/stream.hpp>

/*!
 * \brief Represents a GPIB device.
 * Should be used with Linux-GPIB driver.
 */
class GpibDevice {
public:
    /// Returns a GPIB error message.
    static std::string GetErrorMessage();

    /// Returns a GPIB status message.
    static std::string GetStatusMessage();

    /// Returns a repornt message that includes the GPIB status message and the GPIB error message.
    static std::string GetReportMessage();

public:
    /*!
     * \brief The character type of the GPIB device.
     * Used by boost::iostreams::stream.
     */
    typedef char char_type;

    /*!
     * \brief Type of the GPIB device.
     * Used by boost::iostreams::stream.
     */
    typedef boost::iostreams::bidirectional_device_tag category;

public:
    /*!
     * \brief Create GPIB Device for the givend device name.
     * \param deviceName - name of the device as it declared in gpib.conf.
     * \param goLocalOnDestruction - indicates if the LOC signal should be send to the GPIB bus during the destruction
     *                               of the GpibDevice object. The LOC signal switches all devices connected to the GPIB
     *                               bus to the local mode.
     */
    explicit GpibDevice(const std::string& deviceName, bool goLocalOnDestruction);

    /*!
     * \brief Destructor
     * Destroy GpibDevice object and return devices in a local mode.
     */
    ~GpibDevice();

    /*!
     * \brief Read from GPIB device.
     * \param s - a pointer to where to store read characters.
     * \param n - maximal number of characters to read.
     * \return
     */
    std::streamsize read(char_type *s, std::streamsize n);

    /*!
     * \brief Write to GPIB device.
     * \param s - a pointer to the output characters.
     * \param n - a number of characters to write.
     * \return
     */
    std::streamsize write(const char_type *s, std::streamsize n);

private:
    /// The handle of an opened GPIB device.
    int device_handle;

    /// Indicates if the LOC signal should be send to the GPIB bus during the destruction of the GpibDevice object.
    bool goLocalOnDestruction;
};

/*!
 * \brief Provides a streaming interface to GpibDevice.
 * To have a consistent behaviour,  you should alway enable exceptions with the exceptions(ios::failbit | ios::badbit)
 * member function (e.g. s.exceptions(ios::failbit | ios::badbit);).
 */
typedef boost::iostreams::stream<GpibDevice> GpibStream;
