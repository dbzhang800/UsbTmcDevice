#ifndef USBTMCDEVICE_H
#define USBTMCDEVICE_H

#include <QtCore/qobject.h>

class UsbTmcDevicePrivate;
class UsbTmcDevice : public QObject
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

    bool isOpen() const;
    bool open();
    void close();
    qint64 write(const QByteArray &data);
    QByteArray read(quint64 maxLen = -1);

signals:
    void readyRead();

public slots:

private:
    friend class UsbTmcDevicePrivate;
    UsbTmcDevicePrivate *d;
};

#endif // USBTMCDEVICE_H
