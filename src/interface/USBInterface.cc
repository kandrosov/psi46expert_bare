#include <cstdio>
#include <cstring>

#include "../config.h"
#if HAVE_LIBUSB_1_0_LIBUSB_H
#  include <libusb-1.0/libusb.h>
#endif

#include "USBInterface.h"

const char* CUSB::GetErrorMsg(int error)
{
  switch (error)
  {
    case FT_OK:                          return "ok";
    case FT_INVALID_HANDLE:              return "invalid handle";
    case FT_DEVICE_NOT_FOUND:            return "device not found";
    case FT_DEVICE_NOT_OPENED:           return "device not opened";
    case FT_IO_ERROR:                    return "io error";
    case FT_INSUFFICIENT_RESOURCES:      return "insufficient resource";
    case FT_INVALID_PARAMETER:           return "invalid parameter";
    case FT_INVALID_BAUD_RATE:           return "invalid baud rate";
    case FT_DEVICE_NOT_OPENED_FOR_ERASE: return "device not opened for erase";
    case FT_DEVICE_NOT_OPENED_FOR_WRITE: return "device not opened for write";
    case FT_FAILED_TO_WRITE_DEVICE:      return "failed to write device";
    case FT_EEPROM_READ_FAILED:          return "eeprom read failed";
    case FT_EEPROM_WRITE_FAILED:         return "eeprom write failed";
    case FT_EEPROM_ERASE_FAILED:         return "eeprom erase failed";
    case FT_EEPROM_NOT_PRESENT:          return "eeprom not present";
    case FT_EEPROM_NOT_PROGRAMMED:       return "eeprom not programmed";
    case FT_INVALID_ARGS:                return "invalid args";
    case FT_NOT_SUPPORTED:               return "not supported";
    case FT_OTHER_ERROR:                 return "other error";
  }
  return "unknown error";
}


bool CUSB::EnumFirst(unsigned int &nDevices)
{
  ftStatus = FT_ListDevices(&enumCount,
    NULL,FT_LIST_NUMBER_ONLY);
  if (ftStatus != FT_OK)
  {
    nDevices = enumCount = enumPos = 0;
    return false;
  }

  nDevices = enumCount;
  enumPos = 0;
  return true;
}


bool CUSB::EnumNext(char name[])
{
  if (enumPos >= enumCount) return false;
  ftStatus = FT_ListDevices((PVOID)enumPos, name, FT_LIST_BY_INDEX);
  if (ftStatus != FT_OK)
  {
    enumCount = enumPos = 0;
    return false;
  }

  enumPos++;
  return true;
}


bool CUSB::Open(char serialNumber[])
{
  if (isUSB_open) { ftStatus = FT_DEVICE_NOT_OPENED; return false; }

  m_posR = m_sizeR = m_posW = 0;
  ftStatus = FT_OpenEx(serialNumber, FT_OPEN_BY_SERIAL_NUMBER, &ftHandle);
  if (ftStatus != FT_OK) {
#if HAVE_LIBUSB_1_0_LIBUSB_H
    /* maybe the ftdi_sio and usbserial kernel modules are attached to the device */
    /* try to detach them using the libusb library directly */

    /* prepare libusb structures */
    libusb_device ** list;
    libusb_device_handle *handle;
    struct libusb_device_descriptor descriptor;

    /* initialise libusb and get device list*/
    libusb_init(NULL);
    ssize_t ndevices = libusb_get_device_list(NULL, &list);
    if (ndevices < 0)
      return false;

    char serial [20];

    bool found = false;

    /* loop over all USB devices */
    for (int dev = 0; dev < ndevices; dev++) {
      /* get the device descriptor */
      int ok = libusb_get_device_descriptor(list[dev], &descriptor);
      if (ok != 0)
        continue;

      /* we're only interested in devices with one vendor and product ID */
      if (descriptor.idVendor != 0x0403 || descriptor.idProduct != 0x6001)
        continue;

      /* open the device */
      ok = libusb_open(list[dev], &handle);
      if (ok != 0)
        continue;

      /* Read the serial number from the device */
      ok = libusb_get_string_descriptor_ascii(handle, descriptor.iSerialNumber, (unsigned char *) serial, 20);
      if (ok < 0)
        continue;

      /* Check the device serial number */
      if (strcmp(serialNumber, serial) == 0) {
        /* that's our device */
        found = true;

        /* Detach the kernel module from the device */
        ok = libusb_detach_kernel_driver(handle, 0);
        if (ok == 0)
          printf("Detached kernel driver from selected testboard.\n");
        else
          printf("Unable to detach kernel driver from selected testboard.\n");
        break;
      }

      libusb_close(handle);
    }

    libusb_free_device_list(list, 1);

    /* if the device was not found in the previous loop, don't try again */
    if (!found)
      return false;

    /* try to re-open with the detached device */
    ftStatus = FT_OpenEx(serialNumber, FT_OPEN_BY_SERIAL_NUMBER, &ftHandle);
    if (ftStatus != FT_OK)
      return false;
#else
    return false;
#endif /* HAVE_LIBUSB_1_0_LIBUSB_H */
  }

  FT_SetTimeouts(ftHandle,300000,300000);
  isUSB_open = true;
  return true;
}


void CUSB::Close()
{
  if (!isUSB_open) return;
  FT_Close(ftHandle);
  isUSB_open = 0;
}


bool CUSB::Write(unsigned int bytesToWrite, const void *buffer)
{
  if (!isUSB_open) return false;

  unsigned int k=0;
  for (k=0; k < bytesToWrite; k++)
  {
    if (m_posW >= USBWRITEBUFFERSIZE) { if (!Flush()) return false; }
    m_bufferW[m_posW++] = ((unsigned char*)buffer)[k];
  }
  return true;
}


bool CUSB::Flush()
{
  DWORD bytesWritten; // @KA : unsigned int -> DWORD 
  DWORD bytesToWrite = m_posW; // @KA : unsigned int -> DWORD
  m_posW = 0;

  if (!isUSB_open) return false;

  if (!bytesToWrite) return true;

  ftStatus = FT_Write(ftHandle, m_bufferW, bytesToWrite, &bytesWritten);

  if (ftStatus != FT_OK) return false;
  if (bytesWritten != bytesToWrite) { ftStatus = FT_IO_ERROR; return false; }

  return true;
}

bool CUSB::FillBuffer(unsigned int minBytesToRead)
{
  if (!isUSB_open) return false;

  DWORD bytesAvailable, bytesToRead; // @KA : unsigned int -> DWORD

  ftStatus = FT_GetQueueStatus(ftHandle, &bytesAvailable);
  if (ftStatus != FT_OK) return false;

  if (m_posR<m_sizeR) return false;

  bytesToRead = (bytesAvailable>minBytesToRead)? bytesAvailable : minBytesToRead;
  if (bytesToRead>USBREADBUFFERSIZE) bytesToRead = USBREADBUFFERSIZE;

  ftStatus = FT_Read(ftHandle, m_bufferR, bytesToRead, &m_sizeR);
  m_posR = 0;
  if (ftStatus != FT_OK)
  {
    m_sizeR = 0;
    return false;
  }
  return true;
}


bool CUSB::Read(unsigned int bytesToRead, void *buffer, unsigned int &bytesRead)
{
  bool timeout = false;
  bytesRead = 0;

  if (!isUSB_open) return false;

  unsigned int i;

  for (i=0; i<bytesToRead; i++)
  {
    if (m_posR<m_sizeR)
      ((unsigned char*)buffer)[i] = m_bufferR[m_posR++];

    else if (!timeout)
    {
      unsigned int n = bytesToRead-i;
      if (n>USBREADBUFFERSIZE) n = USBREADBUFFERSIZE;

      if (!FillBuffer(n)) return false;
      if (m_sizeR < n) timeout = true;

      if (m_posR<m_sizeR)
        ((unsigned char*)buffer)[i] = m_bufferR[m_posR++];
      else
      {   // timeout (bytesRead < bytesToRead)
        bytesRead = i;
        return true;
      }
    }

    else
    {
      bytesRead = i;
      return true;
    }
  }

  bytesRead = bytesToRead;
  return true;
}


bool CUSB::Clear()
{
  if (!isUSB_open) return false;

  ftStatus = FT_Purge(ftHandle, FT_PURGE_RX|FT_PURGE_TX);
  m_posR = m_sizeR = 0;
  m_posW = 0;

  return ftStatus != FT_OK;
}


bool CUSB::Read_String(char *s, unsigned short maxlength)
{
  char ch = 0;
  unsigned short i=0;
  do
  {
    if (!Read_CHAR(ch)) return false;
    if (i<maxlength) { s[i] = ch; i++; }
  } while (ch != 0);
  if (i >= maxlength) s[maxlength-1] = 0;
  return true;
}


bool CUSB::Write_String(const char *s)
{
  do
  {
    if (!Write_CHAR(*s)) return false;
    s++;
  } while (*s != 0);
  return true;
}
