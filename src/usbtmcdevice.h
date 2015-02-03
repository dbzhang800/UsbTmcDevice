#ifndef USBTMCDEVICE_H
#define USBTMCDEVICE_H

#include <QtCore/qiodevice.h>

class UsbTmcDevicePrivate;
class UsbTmcDevice : public QIODevice
{
    Q_OBJECT
public:
    enum InterfaceProtocol
    {
        USBTMCProtocol,
        USB488Protocol
    };

    explicit UsbTmcDevice(QObject *parent = 0);
    UsbTmcDevice(ushort verderId, ushort productId, const QString &serialNumber=QString(), QObject *parent = 0);
    ~UsbTmcDevice();

    bool open(OpenMode mode);
    bool isSequential() const;
    void close();
signals:

public slots:

protected:
    qint64 readData(char * data, qint64 maxlen);
    qint64 writeData(const char * data, qint64 len);

private:
    friend class UsbTmcDevicePrivate;
    UsbTmcDevicePrivate *d;
};

#endif // USBTMCDEVICE_H
