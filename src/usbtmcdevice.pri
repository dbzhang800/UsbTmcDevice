include($$PWD/3rdParty/libusb.pri)
DEFINES += USBTMCDEVICE_USE_LIBUSB

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/usbtmcdevice.h \
    $$PWD/usbtmcdevice_p.h

SOURCES += \
    $$PWD/usbtmcdevice.cpp \
    $$PWD/usbtmcdevice_libusb.cpp
