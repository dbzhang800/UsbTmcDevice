#include "usbtmcdevice.h"
#include "usbtmcdevice_p.h"

UsbTmcDevicePrivate::UsbTmcDevicePrivate(UsbTmcDevice *q) :
    q(q)
{
}

/*! \class UsbTmcDevice
 */
UsbTmcDevice::UsbTmcDevice(QObject *parent) :
    QIODevice(parent), d(new UsbTmcDevicePrivate(this))
{
}

UsbTmcDevice::UsbTmcDevice(ushort verderId, ushort productId, QObject *parent) :
    QIODevice(parent), d(new UsbTmcDevicePrivate(this))
{
}

UsbTmcDevice::~UsbTmcDevicePrivate()
{
    delete d;
}

qint64 UsbTmcDevice::readData(char *data, qint64 maxSize)
{
    return -1;
}

qint64 UsbTmcDevice::writeData(const char *data, qint64 maxSize)
{
    return -1;
}
