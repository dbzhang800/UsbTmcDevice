QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = simple
TEMPLATE = app

include(../../src/usbtmcdevice.pri)

SOURCES += main.cpp
