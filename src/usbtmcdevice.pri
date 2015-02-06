#win32:CONFIG += use_winusb

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/usbtmcdevice.h \
    $$PWD/usbtmcdevice_p.h

SOURCES += $$PWD/usbtmcdevice.cpp

win32:use_winusb {
    DEFINES += USBTMCDEVICE_USE_WINUSB
    SOURCES += $$PWD/usbtmcdevice_winusb.cpp
    LIBS += -lwinusb -lsetupapi
} else {
    #Make use of libusb if no other backend is used.
    DEFINES += USBTMCDEVICE_USE_LIBUSB
    SOURCES += $$PWD/usbtmcdevice_libusb.cpp
    include($$PWD/3rdParty/libusb.pri)
}
