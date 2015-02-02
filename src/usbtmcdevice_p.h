#ifndef USBTMCDEVICE_P_H
#define USBTMCDEVICE_P_H

#include "usbtmcdevice.h"

#ifdef USBTMCDEVICE_USE_LIBUSB
#include <libusb.h>
#endif

class UsbTmcDevicePrivate
{
    friend class UsbTmcDevice;
public:
    UsbTmcDevicePrivate(UsbTmcDevice *q);
    ~UsbTmcDevicePrivate();

    bool open_sys(QIODevice::OpenMode mode);
    void close_sys();
    void init_sys();
    void exit_sys();

#ifdef USBTMCDEVICE_USE_LIBUSB
    libusb_context *libusbContext;
    libusb_device_handle *libusbHandle;
#endif

    ushort venderId;
    ushort productId;
    QString serialNumber;

    UsbTmcDevice *q;
};

#endif // USBTMCDEVICE_P_H
