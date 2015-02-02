win32 {
    LIBUSBPATH = $$PWD/libusb-1.0.19-rc1-win

    INCLUDEPATH += $$LIBUSBPATH/include/libusb-1.0

    ##To make thing simple, we use static lib here.
    !contains(QMAKE_TARGET.arch, x86_64) {
        win32-msvc*:LIBS += -L$$LIBUSBPATH/MS32/static -llibusb-1.0
        win32-g++:LIBS += -L$$LIBUSBPATH/MinGW32/static -lusb-1.0
    } else {
        win32-msvc*:LIBS += -L$$LIBUSBPATH/MS64/static -llibusb-1.0
        win32-g++:LIBS += -L$$LIBUSBPATH/MinGW64/static -lusb-1.0
    }
}

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += libusb-1.0
}
