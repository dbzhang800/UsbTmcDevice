#include "usbtmcdevice.h"
#include "usbtmcdevice_p.h"

UsbTmcDevicePrivate::UsbTmcDevicePrivate(ushort vid, ushort pid, const QString &serialNumber, UsbTmcDevice *q) :
    venderId(vid), productId(pid), serialNumber(serialNumber), interfaceNumber(0),
    bulkOutEndpointNumber(0), bulkInEndpointNumber(0), interruptInEndpointNumber(-1),
    interfaceProtocol(UsbTmcDevice::USBTMCProtocol),
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
    QIODevice(parent), d(new UsbTmcDevicePrivate(0, 0, QString(), this))
{
}

UsbTmcDevice::UsbTmcDevice(ushort venderId, ushort productId, const QString &serialNumber, QObject *parent) :
    QIODevice(parent), d(new UsbTmcDevicePrivate(venderId, productId, serialNumber, this))
{
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
