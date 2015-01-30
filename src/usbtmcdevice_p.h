#ifndef USBTMCDEVICE_P_H
#define USBTMCDEVICE_P_H

#include "usbtmcdevice.h"
#include <libusb.h>

class UsbTmcDevicePrivate
{
public:
    UsbTmcDevicePrivate(UsbTmcDevice *q);

    UsbTmcDevice *q;
};

#endif // USBTMCDEVICE_P_H
