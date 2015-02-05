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

    QByteArray query(const QByteArray &data);
    qint64 write(const QByteArray &data);
    QByteArray read(quint64 maxLen = -1);

    InterfaceProtocol interfaceProtocol() const;
    bool isTalkOnly() const;
    bool isListenOnly() const;

public slots:
    bool open();
    void close();

private:
    friend class UsbTmcDevicePrivate;
    UsbTmcDevicePrivate *d;
};

#endif // USBTMCDEVICE_H
