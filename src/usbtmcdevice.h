#ifndef USBTMCDEVICE_H
#define USBTMCDEVICE_H

#include <QtCore/qiodevice.h>

class UsbTmcDevicePrivate;
class UsbTmcDevice : public QIODevice
{
    Q_OBJECT
public:
    explicit UsbTmcDevice(QObject *parent = 0);
    UsbTmcDevice(ushort verderId, ushort productId, int index = 0, QObject *parent = 0);
    ~UsbTmcDevicePrivate();

signals:

public slots:

protected:
    qint64 readData(char * data, qint64 maxSize);
    qint64 writeData(const char * data, qint64 maxSize);

private:
    UsbTmcDevicePrivate *d;
};

#endif // USBTMCDEVICE_H
