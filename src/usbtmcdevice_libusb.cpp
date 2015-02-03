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

bool UsbTmcDevicePrivate::open_sys(QIODevice::OpenMode /*mode*/)
{
    if (!libusbContext)
        return false;

    libusb_device **devsList;
    ssize_t cnt = libusb_get_device_list(libusbContext, &devsList);
    if (cnt <= 0)
        return false;

    //Find usb device based on vid and pid.
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
                    QString sn = getStringDescriptor(handle, desc.iSerialNumber);
                    libusb_close(handle);

                    if (serialNumber != sn)
                        continue;
                }
            }
            found = devsList[i];
            break;
        }
    }

    //Find the interface.
    const libusb_interface *libusbInterface = 0;
    libusb_config_descriptor *config;
    libusb_get_config_descriptor(found, 0, &config);
    for(int i=0; i<(int)config->bNumInterfaces; i++) {
        if (config->interface[i].altsetting->bInterfaceClass == (uint8_t)0xFE
                && config->interface->altsetting->bInterfaceSubClass == (uint8_t)0x03) {
            if (config->interface[i].altsetting->iInterface == 1)
                interfaceProtocol = UsbTmcDevice::USB488Protocol;
            interfaceNumber = i;
            libusbInterface = &config->interface[i];
            break;
        }
    }
    libusb_free_config_descriptor(config);

    //Open the device.
    if (libusbInterface) {
        if (libusb_open(found, &libusbHandle) < 0) {
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
    interfaceNumber = 0;
    int r = libusb_init(&libusbContext);
    if (r < 0)
        qWarning("libusb init failed.");
}

void UsbTmcDevicePrivate::exit_sys()
{
    if (libusbContext)
        libusb_exit(libusbContext);
}
