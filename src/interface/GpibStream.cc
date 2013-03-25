/*!
 * \file GpibStream.cc
 * \brief Definition of GpibStream and GpibDevice classes.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 05-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Added boolean indicator goLocalOnDestruction to indicate if the LOC signal should be send in
 *        GpibDevice::~GpibDevice().
 * 01-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#include <map>
#include <sstream>
#include <gpib/ib.h>

#include "GpibStream.h"

GpibDevice::GpibDevice(const std::string& deviceName, bool _goLocalOnDestruction)
    : goLocalOnDestruction(_goLocalOnDestruction)
{
    device_handle = ibfind(deviceName.c_str());
    if(device_handle < 0)
        throw std::ios_base::failure(GetReportMessage());

    ibclr(device_handle);
    if(ibsta & ERR)
        throw std::ios_base::failure(GetReportMessage());
}

GpibDevice::~GpibDevice()
{
    if(goLocalOnDestruction)
        ibloc(device_handle);
}

std::streamsize GpibDevice::read(char_type *s, std::streamsize n)
{
    ibrd(device_handle, s, n);
    if(ibsta & ERR)
        throw std::ios_base::failure(GetReportMessage());
    return (std::streamsize) ibcnt;
}

std::streamsize GpibDevice::write(const char_type *s, std::streamsize n)
{
    ibwrt(device_handle, s, n);
    if(ibsta & ERR)
        throw std::ios_base::failure(GetReportMessage());
    return (std::streamsize) ibcnt;
}

std::string GpibDevice::GetErrorMessage()
{
    typedef std::map<iberr_code, std::string> MessageMap;
    static const std::string UNKNOWN_ERROR = "Unknown error.";
    static MessageMap messages;
    if(!messages.size()) {
        messages[EDVR] = "A system call has failed. ibcnt/ibcntl will be set to the value of errno.";
        messages[ECIC] = "Your interface board needs to be controller-in-charge, but is not.";
        messages[ENOL] = "You have attempted to write data or command bytes, but there are no listeners currently"
                         " addressed.";
        messages[EADR] = "The interface board has failed to address itself properly before starting an io operation.";
        messages[EARG] = "One or more arguments to the function call were invalid.";
        messages[ESAC] = "The interface board needs to be system controller, but is not.";
        messages[EABO] = "A read or write of data bytes has been aborted, possibly due to a timeout or reception of a"
                         " device clear command.";
        messages[ENEB] = "The GPIB interface board does not exist, its driver is not loaded, or it is not configured"
                         " properly.";
        messages[EDMA] = "Not used (DMA error), included for compatibility purposes.";
        messages[EOIP] = "Function call can not proceed due to an asynchronous IO operation (ibrda(), ibwrta(), or"
                         " ibcmda()) in progress.";
        messages[ECAP] = "Incapable of executing function call, due the GPIB board lacking the capability, or the"
                         " capability being disabled in software.";
        messages[EFSO] = "File system error. ibcnt/ibcntl will be set to the value of errno.";
        messages[EBUS] = "An attempt to write command bytes to the bus has timed out.";
        messages[ESTB] = "One or more serial poll status bytes have been lost. This can occur due to too many status"
                         " bytes accumulating (through automatic serial polling) without being read.";
        messages[ESRQ] = "The serial poll request service line is stuck on. This can occur if a physical device on the"
                         " bus requests service, but its GPIB address has not been opened (via ibdev() for example) by any"
                         " process. Thus the automatic serial polling routines are unaware of the device's existence and will"
                         " never serial poll it.";
        messages[ETAB] = "This error can be returned by ibevent(), FindLstn(), or FindRQS(). See their descriptions"
                         " for more information.";
    }

    iberr_code code = (iberr_code)iberr;
    MessageMap::const_iterator iter = messages.find(code);
    if(iter != messages.end())
        return iter->second;
    return UNKNOWN_ERROR;
}

std::string GpibDevice::GetStatusMessage()
{
    typedef std::map<ibsta_bits, std::string> MessageMap;
    static const std::string UNKNOWN_STATUS = "Unknown status.";
    static MessageMap messages;
    if(!messages.size()) {
        messages[DCAS] = "DCAS is set when a board receives the device clear command (that is, the SDC or DCL command"
                         " byte). It is cleared on the next 'traditional' or 'multidevice' function call following ibwait()"
                         " (with DCAS set in the wait mask), or following a read or write (ibrd(), ibwrt(), Receive(), etc.)."
                         " The DCAS and DTAS bits will only be set if the event queue is disabled. The event queue may be"
                         " disabled with ibconfig().";
        messages[DTAS] = "DTAS is set when a board has received a device trigger command (that is, the GET command"
                         " byte). It is cleared on the next 'traditional' or 'multidevice' function call following ibwait()"
                         " (with DTAS in the wait mask). The DCAS and DTAS bits will only be set if the event queue is disabled."
                         " The event queue may be disabled with ibconfig().";
        messages[LACS] = "Board is currently addressed as a listener.";
        messages[TACS] = "Board is currently addressed as talker.";
        messages[ATN] = "The ATN line is asserted.";
        messages[CIC] = "Board is controller-in-charge, so it is able to set the ATN line.";
        messages[REM] = "Board is in 'remote' state.";
        messages[LOK] = "Board is in 'lockout' state.";
        messages[CMPL] = "I/O operation is complete. Useful for determining when an asynchronous io operation (ibrda(),"
                         " ibwrta(), etc) has completed.";
        messages[EVENT] = "One or more clear, trigger, or interface clear events have been received, and are available"
                          " in the event queue (see ibevent()). The EVENT bit will only be set if the event queue is enabled. The"
                          " event queue may be enabled with ibconfig().";
        messages[SPOLL] = "If this bit is enabled (see ibconfig()), it is set when the board is serial polled. The"
                          " SPOLL bit is cleared when the board requests service (see ibrsv()) or you call ibwait() on the board"
                          " with SPOLL in the wait mask.";
        messages[RQS] = "RQS indicates that the device has requested service, and one or more status bytes are"
                        " available for reading with ibrsp(). RQS will only be set if you have automatic serial polling enabled"
                        " (see ibconfig()).";
        messages[SRQI] = "SRQI indicates that a device connected to the board is asserting the SRQ line. It is only set"
                         " if the board is the controller-in-charge. If automatic serial polling is enabled (see ibconfig()),"
                         " SRQI will generally be cleared, since when a device requests service it will be automatically polled"
                         " and then unassert SRQ.";
        messages[END] = "END is set if the last io operation ended with the EOI line asserted, and may be set on"
                        " reception of the end-of-string character. The IbcEndBitIsNormal option of ibconfig() can be used to"
                        " configure whether or not END should be set on reception of the eos character.";
        messages[TIMO] = "TIMO indicates that the last io operation or ibwait() timed out.";
        messages[ERR] = "ERR is set if the last 'traditional' or 'multidevice' function call failed. The global"
                        " variable iberr will be set indicate the cause of the error.";
    }
    std::stringstream output;
    for(MessageMap::const_iterator iter = messages.begin(); iter != messages.end(); ++iter) {
        if(ibsta & iter->first)
            output << iter->second << std::endl;
    }
    if(!output.str().size())
        output << UNKNOWN_STATUS << std::endl;
    return output.str();
}

std::string GpibDevice::GetReportMessage()
{
    std::stringstream output;
    output << "GPIB Status: " << std::hex << ibsta << std::dec << std::endl << GetStatusMessage();
    if(ibsta & ERR)
        output << "GPIB Error: " << std::hex << iberr << std::dec << std::endl << GetErrorMessage() << std::endl;
    return output.str();
}
