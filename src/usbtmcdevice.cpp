#include "usbtmcdevice.h"
#include "usbtmcdevice_p.h"

UsbTmcDevicePrivate::UsbTmcDevicePrivate(ushort vid, ushort pid, const QString &serialNumber, UsbTmcDevice *q) :
    isOpen(false), venderId(vid), productId(pid), serialNumber(serialNumber), interfaceNumber(0),
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

void UsbTmcDevicePrivate::fillBulkOutHeader(QByteArray &data, uchar msgId)
{
    Q_ASSERT(data.length() >= 12);

    //The Host must set bTag different than the bTag used in the previous BulkOUT Header.
    last_bTag += 1;
    if (!last_bTag)
        last_bTag = 1;

    data[0] = msgId;
    data[1] = last_bTag;
    data[2] = ~last_bTag;
    data[3] = 0x00;
}

void UsbTmcDevicePrivate::fillBulkOutHeader_DevDepMsgOut(QByteArray &data, int transferSize, bool eom)
{
    Q_ASSERT(data.length() >= 12);

    fillBulkOutHeader(data, USBTMC_MSGID_DEV_DEP_MSG_OUT);
    data[4] = transferSize & 0xFF;
    data[5] = (transferSize >> 8) & 0xFF;
    data[6] = (transferSize >> 16) & 0xFF;
    data[7] = (transferSize >> 24) & 0xFF;
    data[8] = eom ? 0x01 : 0x00;

    for (int i=9; i<12; ++i)
        data[i] = 0x00;
}

void UsbTmcDevicePrivate::fillBulkOutHeader_RequestDevDepMsgIn(QByteArray &data, int transferSize, int termChar)
{
    Q_ASSERT(data.length() >= 12);

    fillBulkOutHeader(data, USBTMC_MSGID_REQUEST_DEV_DEP_MSG_IN);
    data[4] = transferSize & 0xFF;
    data[5] = (transferSize >> 8) & 0xFF;
    data[6] = (transferSize >> 16) & 0xFF;
    data[7] = (transferSize >> 24) & 0xFF;

    if (termChar == -1) {
        for (int i=8; i<12; ++i)
            data[i] = 0x00;
    } else {
        data[8] = 0x02;
        data[9] = termChar;
        for (int i=10; i<12; ++i)
            data[i] = 0x00;
    }
}

/*! \class UsbTmcDevice
 */
UsbTmcDevice::UsbTmcDevice(QObject *parent) :
    QObject(parent), d(new UsbTmcDevicePrivate(0, 0, QString(), this))
{
}

UsbTmcDevice::UsbTmcDevice(ushort venderId, ushort productId, const QString &serialNumber, QObject *parent) :
    QObject(parent), d(new UsbTmcDevicePrivate(venderId, productId, serialNumber, this))
{
}

UsbTmcDevice::~UsbTmcDevice()
{
    delete d;
}

bool UsbTmcDevice::isOpen() const
{
    return d->isOpen;
}

bool UsbTmcDevice::open()
{
    if (!isOpen()) {
        if (d->open_sys())
            d->isOpen = true;
    }
   return isOpen();
}

void UsbTmcDevice::close()
{
    if (isOpen())
        d->close_sys();
    d->isOpen = false;
}

qint64 UsbTmcDevice::write(const QByteArray &data)
{
    int len = data.size();
    int pad = (4 - len % 4) % 4;
    QByteArray buffer(12 + len + pad, Qt::Uninitialized);
    d->fillBulkOutHeader_DevDepMsgOut(buffer, len);
    memcpy(buffer.data()+12, data.data(), len);
    for (int i=0; i<pad; ++i)
        buffer[int(12 + len + i)] = 0x00;

    //Write raw data to bulk out endpoint.
    return d->writeToBulkOutEndpoint_sys(data.data(), data.length());
}

QByteArray UsbTmcDevice::read()
{
    return QByteArray();
}
