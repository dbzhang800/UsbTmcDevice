#include "usbtmcdevice.h"
#include <QDebug>

int main()
{
    //Take IPD3303SLU as an example.
    //UsbTmcDevice *tmcDevice = new UsbTmcDevice(0x8246, 0x1117, "IPD3303SLU000A9B");
    UsbTmcDevice *tmcDevice = new UsbTmcDevice();

    if (!tmcDevice->open()) {
        qDebug() << "Device open failed.";
        delete tmcDevice;
        return -1;
    }

    qDebug()<<tmcDevice->query("*IDN?\n").trimmed();
    qDebug()<<tmcDevice->query("*IDN?\n").trimmed();

    tmcDevice->write("CURR 1.4\n");
    qDebug()<<tmcDevice->query("MEAS:CURR?\n").trimmed();
    qDebug()<<tmcDevice->query("MEAS:CURR?\n").trimmed();

    tmcDevice->close();
    delete tmcDevice;
    return 0;
}
