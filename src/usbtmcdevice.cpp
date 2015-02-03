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

qint64 UsbTmcDevice::readData(char *data, qint64 maxlen)
{
    //Read raw data from bulk in endpoint.
    return d->readFromBulkInEndpoint_sys(data, maxlen);
}

qint64 UsbTmcDevice::writeData(const char *data, qint64 len)
{
    //Write raw data to bulk out endpoint.
    int pad = (4 - len % 4) % 4;
    QByteArray buffer(12 + len + pad, Qt::Uninitialized);
    d->fillBulkOutHeader_DevDepMsgOut(buffer, len);
    memcpy(buffer.data()+12, data, len);
    for (int i=0; i<pad; ++i)
        buffer[int(12 + len + i)] = 0x00;
    return d->writeToBulkOutEndpoint_sys(data, len);
}
