#include "usbtmcdevice.h"
#include <QDebug>

int main()
{
    //Take IPD3303SLU as an example.
    //UsbTmcDevice *tmcDevice = new UsbTmcDevice(0x8246, 0x1117, "IPD3303SLU000A9B");
    UsbTmcDevice *tmcDevice = new UsbTmcDevice();
    if (tmcDevice->open()) {
        tmcDevice->write("*IDN?\n");
        qDebug()<<tmcDevice->read();

        tmcDevice->close();
    } else {
        qDebug() << "Device open failed.";
    }
    delete tmcDevice;
    return 0;
}
