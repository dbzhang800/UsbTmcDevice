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
#include "usbtmcdevice.h"
#include "usbtmcdevice_p.h"

#include <QUuid>
#include <QDebug>
#include <QStringList>
#include <setupapi.h>

static QStringList findWinUsbDevicePaths(ushort vId, ushort pId, const QString &serialNumber)
{
    //GUID winusbDeviceSetupClassGuid = QUuid("88BAE032-5A81-49f0-BC3D-A4FF138216D6");
    //GUID_DEVINTERFACE_USB_DEVICE
    GUID usbInterfaceGuid = QUuid("A5DCBF10-6530-11D2-901F-00C04FB951ED");
    QStringList devicePathList;

    HDEVINFO devInfoSet= ::SetupDiGetClassDevs(&usbInterfaceGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (devInfoSet != INVALID_HANDLE_VALUE) {

        SP_DEVICE_INTERFACE_DATA interfaceData;
        ::ZeroMemory(&interfaceData, sizeof(SP_DEVICE_INTERFACE_DATA));
        interfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

        int interfaceIndex=0;
        while(::SetupDiEnumDeviceInterfaces(devInfoSet, NULL, &usbInterfaceGuid, interfaceIndex, &interfaceData)) {

            DWORD detDataSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) + 256;
            PSP_DEVICE_INTERFACE_DETAIL_DATA pDetData = (SP_DEVICE_INTERFACE_DETAIL_DATA*) new char[detDataSize];
            pDetData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

            SP_DEVINFO_DATA devInfoData;
            ::ZeroMemory(&devInfoData, sizeof(SP_DEVINFO_DATA));
            devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

            ::SetupDiGetDeviceInterfaceDetail(devInfoSet, &interfaceData, pDetData, detDataSize, NULL, &devInfoData);

            QString devicePath = QString::fromUtf16(reinterpret_cast<ushort *>(pDetData->DevicePath));
            if ((vId == 0 || devicePath.contains(QString::fromLatin1("vid_%1").arg(vId, 4, 16, QLatin1Char('0')), Qt::CaseInsensitive))
                    && (pId == 0 || devicePath.contains(QString::fromLatin1("pid_%1").arg(pId, 4, 16, QLatin1Char('0')), Qt::CaseInsensitive))
                    && (serialNumber.isEmpty() || devicePath.contains(serialNumber, Qt::CaseInsensitive))) {
                devicePathList.append(devicePath);
            }

            delete [] (char*)pDetData;
            interfaceIndex++;
        }
        ::SetupDiDestroyDeviceInfoList(devInfoSet);
    }
    return devicePathList;
}

bool UsbTmcDevicePrivate::open_sys()
{
    QStringList devicePaths = findWinUsbDevicePaths(venderId, productId, serialNumber);
    if (devicePaths.isEmpty())
        return false;

    foreach (QString devicePath, devicePaths) {
        winusbDeviceHandle = CreateFile(reinterpret_cast<const wchar_t*>(devicePath.utf16()),
                                              GENERIC_WRITE | GENERIC_READ,
                                              FILE_SHARE_WRITE | FILE_SHARE_READ,
                                              NULL,
                                              OPEN_EXISTING,
                                              FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                                              NULL);
        if (winusbDeviceHandle != INVALID_HANDLE_VALUE) {
            bool ret = WinUsb_Initialize(winusbDeviceHandle, &winusbInterfaceHandle);
            if (false == ret) {
                CloseHandle(winusbDeviceHandle);
                winusbDeviceHandle = INVALID_HANDLE_VALUE;
                continue;
            }

            USB_DEVICE_DESCRIPTOR deviceDesc;
            ULONG lengthReceived;
            ret = WinUsb_GetDescriptor(winusbInterfaceHandle,
                                           USB_DEVICE_DESCRIPTOR_TYPE,
                                           0,
                                           0,
                                           (PBYTE) &deviceDesc,
                                           sizeof(deviceDesc),
                                           &lengthReceived);
            if (false == ret || lengthReceived != sizeof(deviceDesc)) {
                close_sys();
                continue;
            }
            //qDebug()<<deviceDesc.idVendor<<deviceDesc.idProduct<<deviceDesc.iSerialNumber;

            USB_INTERFACE_DESCRIPTOR interDesc;
            ret = WinUsb_QueryInterfaceSettings(winusbInterfaceHandle, 0, &interDesc);
            if (false == ret) {
                close_sys();
                continue;
            }
            //qDebug()<<interDesc.bInterfaceClass << interDesc.bInterfaceSubClass;

            if (interDesc.bInterfaceClass != uchar(0xFE) || interDesc.bInterfaceSubClass != uchar(0x03)) {
                close_sys();
                continue;
            }

            //Interface found now.
            interfaceProtocol = interDesc.bInterfaceProtocol == 1 ? UsbTmcDevice::USB488Protocol : UsbTmcDevice::USBTMCProtocol;

            for (int k=0; k<static_cast<int>(interDesc.bNumEndpoints); k++) {
                WINUSB_PIPE_INFORMATION epDesc;
                WinUsb_QueryPipe(winusbInterfaceHandle, 0, k, &epDesc);

                int epNumber = epDesc.PipeId & 0x0F;
                if ((epDesc.PipeType & 0x03) == 0x02) {
                    //Bulk transfer ep
                    if (epDesc.PipeId & 0x80)
                        bulkInEndpointNumber = epNumber;
                    else
                        bulkOutEndpointNumber = epNumber;
                } else if ((epDesc.PipeType & 0x03) == 0x03) {
                    //Inter transfer ep
                    interruptInEndpointNumber = epNumber;
                }
            }
            break;
        }
    }

    return winusbDeviceHandle != INVALID_HANDLE_VALUE;
}

void UsbTmcDevicePrivate::close_sys()
{
    if (winusbDeviceHandle != INVALID_HANDLE_VALUE) {
        WinUsb_Free(winusbInterfaceHandle);
        CloseHandle(winusbDeviceHandle);
        winusbDeviceHandle = INVALID_HANDLE_VALUE;
    }
}

void UsbTmcDevicePrivate::init_sys()
{
    winusbDeviceHandle = INVALID_HANDLE_VALUE;
}

void UsbTmcDevicePrivate::exit_sys()
{
}

qint64 UsbTmcDevicePrivate::readFromBulkInEndpoint_sys(char *data, qint64 maxlen)
{
    ULONG cbRead = 0;

    bool ret = WinUsb_ReadPipe(winusbInterfaceHandle, bulkInEndpointNumber | 0x80, (uchar *)data, maxlen, &cbRead, 0);
    if(ret) {
        //OK
        return cbRead;
    }

    return -1;
}

qint64 UsbTmcDevicePrivate::writeToBulkOutEndpoint_sys(const char *data, qint64 len)
{
    ULONG cbSent = 0;
    bool ret = WinUsb_WritePipe(winusbInterfaceHandle, bulkOutEndpointNumber | 0x00, (uchar *)data, len, &cbSent, 0);
    if(ret) {
        //Ok
        return cbSent;
    }

    return -1;
}

qint64 UsbTmcDevicePrivate::readWriteDefaultControlEndpoint_sys(quint8 bmRequestType, quint8 bRequest, quint16 wValue, quint16 wIndex, char *data, quint16 wLength)
{
    WINUSB_SETUP_PACKET setupPacket;
    ZeroMemory(&setupPacket, sizeof(WINUSB_SETUP_PACKET));

    //Create the setup packet
    setupPacket.RequestType = bmRequestType;
    setupPacket.Request = bRequest;
    setupPacket.Value = wValue;
    setupPacket.Index = wIndex;
    setupPacket.Length = wLength;

    ULONG cbSent = 0;
    bool ret = WinUsb_ControlTransfer(winusbInterfaceHandle, setupPacket, (uchar *)data, wLength, &cbSent, 0);
    if(ret) {
        //OK
        //qDebug()<<cbSent;
        return cbSent;
    }

    return -1;
}


