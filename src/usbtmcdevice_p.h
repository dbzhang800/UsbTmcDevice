/****************************************************************************
** Copyright (c) 2015 Debao Zhang <hello@debao.me>
**
** Permission is hereby granted, free of charge, to any person obtaining
** a copy of this software and associated documentation files (the
** "Software"), to deal in the Software without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Software, and to
** permit persons to whom the Software is furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be
** included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
** LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
** OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
** WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**
****************************************************************************/
#ifndef USBTMCDEVICE_P_H
#define USBTMCDEVICE_P_H

#include "usbtmcdevice.h"

#ifdef USBTMCDEVICE_USE_LIBUSB
#include <libusb.h>
#endif

const uchar USBTMC_MSGID_DEV_DEP_MSG_OUT = 1;
const uchar USBTMC_MSGID_REQUEST_DEV_DEP_MSG_IN = 2;
const uchar USBTMC_MSGID_DEV_DEP_MSG_IN = 2;
const uchar USBTMC_MSGID_VENDOR_SPECIFIC_OUT = 126;
const uchar USBTMC_MSGID_REQUEST_VENDOR_SPECIFIC_IN = 127;
const uchar USBTMC_MSGID_VENDOR_SPECIFIC_IN = 127;
const uchar USB488_MSGID_TRIGGER = 128;
const uchar USBTMC_STATUS_SUCCESS = 0x01;
const uchar USBTMC_STATUS_PENDING = 0x02;
const uchar USBTMC_STATUS_FAILED = 0x80;
const uchar USBTMC_STATUS_TRANSFER_NOT_IN_PROGRESS = 0x81;
const uchar USBTMC_STATUS_SPLIT_NOT_IN_PROGRESS = 0x82;
const uchar USBTMC_STATUS_SPLIT_IN_PROGRESS = 0x83;
const uchar USB488_STATUS_INTERRUPT_IN_BUSY = 0x20;
const uchar USBTMC_REQUEST_INITIATE_ABORT_BULK_OUT = 1;
const uchar USBTMC_REQUEST_CHECK_ABORT_BULK_OUT_STATUS = 2;
const uchar USBTMC_REQUEST_INITIATE_ABORT_BULK_IN = 3;
const uchar USBTMC_REQUEST_CHECK_ABORT_BULK_IN_STATUS = 4;
const uchar USBTMC_REQUEST_INITIATE_CLEAR = 5;
const uchar USBTMC_REQUEST_CHECK_CLEAR_STATUS = 6;
const uchar USBTMC_REQUEST_GET_CAPABILITIES = 7;
const uchar USBTMC_REQUEST_INDICATOR_PULSE = 64;
const uchar USB488_READ_STATUS_BYTE = 128;
const uchar USB488_REN_CONTROL = 160;
const uchar USB488_GOTO_LOCAL = 161;
const uchar USB488_LOCAL_LOCKOUT = 162;

struct DevDepMsgInHeader
{
    quint8 msgID;
    quint8 bTag;
    quint8 bTagInverse;
    quint8 reserved;
    quint32 transferSize;
    quint8 bmTransferAttributes;
    quint8 termChar;
    quint16 reserved2;
};

class UsbTmcDevicePrivate
{
    friend class UsbTmcDevice;
public:
    UsbTmcDevicePrivate(ushort vId, ushort pId, const QString &serialNumber, UsbTmcDevice *q);
    ~UsbTmcDevicePrivate();

    void fillBulkOutHeader(QByteArray &data, uchar msgId);
    QByteArray packDevDepMsgOutData(const QByteArray &data, bool eom=true);
    QByteArray packRequestDevDepMsgInData(int transferSize, int termChar=-1);
    QByteArray unpackDevDepMsgInData(const QByteArray &data, DevDepMsgInHeader *header = 0);
    void getCapabilities();

    bool open_sys();
    void close_sys();
    void init_sys();
    void exit_sys();
    qint64 readFromBulkInEndpoint_sys(char *data, qint64 maxlen);
    qint64 writeToBulkOutEndpoint_sys(const char *data, qint64 len);
    qint64 readWriteDefaultControlEndpoint_sys(quint8 bmRequestType, quint8 bRequest, quint16 wValue, quint16 wIndex, char *data, quint16 wLength);

#ifdef USBTMCDEVICE_USE_LIBUSB
    libusb_context *libusbContext;
    libusb_device_handle *libusbHandle;
#endif

    bool isOpen;

    ushort venderId;
    ushort productId;
    QString serialNumber;

    int interfaceNumber;
    int bulkInEndpointNumber;
    int bulkOutEndpointNumber;
    int interruptInEndpointNumber;
    UsbTmcDevice::InterfaceProtocol interfaceProtocol;

    //USBTMC Capabilities
    bool supportIndicatorPulse;
    bool isTalkOnly;
    bool isListenOnly;
    bool supportTermChar;
    //USB488 Capabilities
    bool supportUsb488IEEE488_2;
    bool supportRemLocal;
    bool supportTrigger;
    bool supportScpi;
    bool supportSR1;
    bool supportRL1;
    bool supportDT1;

    uchar last_bTag; //Transfer identifier. [1, 255]

    int timeout;

    UsbTmcDevice *q;
};

#endif // USBTMCDEVICE_P_H
