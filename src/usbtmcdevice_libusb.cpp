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

#include <libusb.h>

/*
 * All devices with a USBTMC interface must implement string descriptors
 * with LANGID = 0x0409 (English, United States)
 */
static QString getStringDescriptor(libusb_device_handle *handle, quint8 descIndex)
{
    uchar data[128];
    int r = libusb_get_string_descriptor(handle, descIndex, 0x0409, data, 128);
    if (r < 4) {
        if (libusb_get_string_descriptor_ascii(handle, descIndex, data, 128) > 0)
            return QString::fromLatin1(reinterpret_cast<char *>(data));
        return QString();
    }

    if (data[0] > r || data[1] != LIBUSB_DT_STRING)
        return QString();

    int length = (r - 2) / 2;
    QString descriptor(length, Qt::Uninitialized);
    for (int i=0; i<length; i++)
        descriptor[i] = data[i*2+2];
    return descriptor;
}

bool UsbTmcDevicePrivate::open_sys()
{
    if (!libusbContext)
        return false;

    interfaceNumber = -1;
    bulkInEndpointNumber = -1;
    bulkOutEndpointNumber = -1;
    interruptInEndpointNumber = -1;

    libusb_device **devsList;
    ssize_t cnt = libusb_get_device_list(libusbContext, &devsList);
    if (cnt <= 0)
        return false;

    //Find usb device based on vid and pid.
    libusb_device *deviceFound = 0;
    for (int i = 0; i < cnt; i++) {
        struct libusb_device_descriptor desc;
        if (libusb_get_device_descriptor(devsList[i], &desc) < 0) {
            //break or continue?
            break;
        }
        if ((venderId == 0 || venderId == desc.idVendor)
                && (productId == 0 || productId == desc.idProduct)) {
            //Compare serialNumber if it's not empty.
            if (!serialNumber.isEmpty()) {
                libusb_device_handle *handle;
                if (!libusb_open(devsList[i], &handle)) {
                    QString sn = getStringDescriptor(handle, desc.iSerialNumber);
                    libusb_close(handle);

                    if (serialNumber != sn)
                        continue;
                }
            }

            //Find usbtmc interface.
            libusb_config_descriptor *config;
            if (libusb_get_config_descriptor(devsList[i], 0, &config) < 0)
                break;
            for(int j=0; j<static_cast<int>(config->bNumInterfaces); j++) {
                const libusb_interface_descriptor * const interDesc = &config->interface[j].altsetting[0];
                if (interDesc->bInterfaceClass == (uint8_t)0xFE
                        && interDesc->bInterfaceSubClass == (uint8_t)0x03) {
                    //OK, interface found.
                    deviceFound = devsList[i];
                    if (interDesc->iInterface == 1)
                        interfaceProtocol = UsbTmcDevice::USB488Protocol;
                    interfaceNumber = j;
                    for (int k=0; k<static_cast<int>(interDesc->bNumEndpoints); k++) {
                        const libusb_endpoint_descriptor *epDesc = &interDesc->endpoint[k];
                        int epNumber = epDesc->bEndpointAddress & 0x0F;
                        if ((epDesc->bmAttributes & 0x03) == 0x02) {
                            //Bulk transfer ep
                            if (epDesc->bEndpointAddress & 0x80)
                                bulkInEndpointNumber = epNumber;
                            else
                                bulkOutEndpointNumber = epNumber;
                        } else if ((epDesc->bmAttributes & 0x03) == 0x03) {
                            //Inter transfer ep
                            interruptInEndpointNumber = epNumber;
                        }
                    }
                    break;
                }
            }
            libusb_free_config_descriptor(config);
            if (deviceFound)
                break;
        }
    }

    if (deviceFound && interfaceNumber != -1) {
        //Open the device if .
        int r = libusb_open(deviceFound, &libusbHandle);
        if (r < 0) {
            qWarning("Can not open the device.");
        } else {
            //find out if kernel driver is attached
            if(libusb_kernel_driver_active(libusbHandle, 0) == 1)
                libusb_detach_kernel_driver(libusbHandle, 0);

            //claim the interface
            if (libusb_claim_interface(libusbHandle, interfaceNumber) < 0) {
                qWarning("Can not claim the interface.");
                libusb_close(libusbHandle);
                libusbHandle = 0;
            }

//            if (libusbHandle && libusb_reset_device(libusbHandle) < 0)
//                qWarning("Cann't reset the device.");
        }
    }

    libusb_free_device_list(devsList, 1);
    return libusbHandle;
}

void UsbTmcDevicePrivate::close_sys()
{
    if (libusbHandle) {
        libusb_release_interface(libusbHandle, interfaceNumber);
        libusb_close(libusbHandle);
        libusbHandle = 0;
    }
}

void UsbTmcDevicePrivate::init_sys()
{
    libusbContext = 0;
    libusbHandle = 0;
    int r = libusb_init(&libusbContext);
    if (r < 0) {
        qWarning("libusb init failed.");
        return;
    }
    //libusb_set_debug(libusbContext, 3);
}

void UsbTmcDevicePrivate::exit_sys()
{
    if (libusbContext)
        libusb_exit(libusbContext);
}

qint64 UsbTmcDevicePrivate::readFromBulkInEndpoint_sys(char *data, qint64 maxlen)
{
    int bytesRead = 0;
    if (libusb_bulk_transfer(libusbHandle, bulkInEndpointNumber | LIBUSB_ENDPOINT_IN, (uchar *)data, maxlen, &bytesRead, timeout) < 0)
        return -1;
    return bytesRead;
}

qint64 UsbTmcDevicePrivate::writeToBulkOutEndpoint_sys(const char *data, qint64 len)
{
    int bytesWritten = 0;
    int r = libusb_bulk_transfer(libusbHandle, bulkOutEndpointNumber | LIBUSB_ENDPOINT_OUT, (uchar *)data, len, &bytesWritten, timeout);
    if (r < 0)
        return -1;
    return bytesWritten;
}

qint64 UsbTmcDevicePrivate::readWriteDefaultControlEndpoint_sys(quint8 bmRequestType, quint8 bRequest, quint16 wValue, quint16 wIndex, char *data, quint16 wLength)
{
    int r = libusb_control_transfer(libusbHandle, bmRequestType, bRequest, wValue, wIndex, (uchar *)data, wLength, timeout);
    if (r < 0)
        return -1;

    return wLength;
}
