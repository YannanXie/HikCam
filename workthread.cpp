#include "workthread.h"
#include <QDebug>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include "MvCameraControl.h"

WorkThread::WorkThread()
{
   qDebug()<<"start thread: "<<QThread::currentThreadId();

}


void WorkThread::run()
{





}

void WorkThread::stop()
{
    g_bExit = false;

}
