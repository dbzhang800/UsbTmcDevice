#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "usbtmcdevice.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Take IPD3303SLU as an example.
    //tmcDevice = new UsbTmcDevice(0x8246, 0x1117, "IPD3303SLU000A9B", this);
    tmcDevice = new UsbTmcDevice(this);
    tmcDevice->open();
}

MainWindow::~MainWindow()
{
    delete ui;
}
