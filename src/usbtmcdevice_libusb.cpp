#include "usbtmcdevice.h"
#include "usbtmcdevice_p.h"

#include <libusb.h>

bool UsbTmcDevicePrivate::open_sys(QIODevice::OpenMode /*mode*/)
{
    if (!libusbContext)
        return false;

    libusb_device **devsList;
    ssize_t cnt = libusb_get_device_list(libusbContext, &devsList);
    if (cnt <= 0)
        return false;

    libusb_device *found = 0;
    for (int i = 0; i < cnt; i++) {
        struct libusb_device_descriptor desc;
        if (libusb_get_device_descriptor(devsList[i], &desc) < 0)
            break;
        if (desc.idVendor == venderId && desc.idProduct == productId) {
            if (!serialNumber.isEmpty()) {
                if (desc.iSerialNumber == 0)
                    continue;

                libusb_device_handle *handle;
                if (!libusb_open(devsList[i], &handle)) {
                    char data[128];
                    libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, (uchar *)data, 128);
                    libusb_close(handle);

                    if (serialNumber != QString::fromLatin1(data))
                        continue;
                }
            }
            found = devsList[i];
            break;
        }
    }

    if (found)
        libusb_open(found, &libusbHandle);
    libusb_free_device_list(devsList, 1);
    return libusbHandle;
}

void UsbTmcDevicePrivate::close_sys()
{
    libusb_close(libusbHandle);
}

void UsbTmcDevicePrivate::init_sys()
{
    libusbContext = 0;
    libusbHandle = 0;
    int r = libusb_init(&libusbContext);
    if (r < 0)
        qWarning("libusb init failed.");
}

void UsbTmcDevicePrivate::exit_sys()
{
    if (libusbContext)
        libusb_exit(libusbContext);
}
