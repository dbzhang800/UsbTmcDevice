#include "usbtmcdevice.h"
#include "usbtmcdevice_p.h"

UsbTmcDevicePrivate::UsbTmcDevicePrivate(UsbTmcDevice *q) :
    q(q)
{
    init_sys();
}

UsbTmcDevicePrivate::~UsbTmcDevicePrivate()
{
    exit_sys();
}

/*! \class UsbTmcDevice
 */
UsbTmcDevice::UsbTmcDevice(QObject *parent) :
    QIODevice(parent), d(new UsbTmcDevicePrivate(this))
{
    d->venderId = 0;
    d->productId = 0;
}

UsbTmcDevice::UsbTmcDevice(ushort verderId, ushort productId, const QString &serialNumber, QObject *parent) :
    QIODevice(parent), d(new UsbTmcDevicePrivate(this))
{
    d->venderId = verderId;
    d->productId = productId;
    d->serialNumber = serialNumber;
}

UsbTmcDevice::~UsbTmcDevice()
{
    delete d;
}

bool UsbTmcDevice::open(QIODevice::OpenMode mode)
{
    if (mode != QIODevice::NotOpen && !isOpen()) {
        if (d->open_sys(mode))
            QIODevice::open(mode);
    }
   return isOpen();
}

void UsbTmcDevice::close()
{
    if (isOpen())
        d->close_sys();
    QIODevice::close();
}

bool UsbTmcDevice::isSequential() const
{
    return true;
}

qint64 UsbTmcDevice::readData(char *data, qint64 maxSize)
{
    return -1;
}

qint64 UsbTmcDevice::writeData(const char *data, qint64 maxSize)
{
    return -1;
}
