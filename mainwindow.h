#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "MvCamera.h"
#include "MvCameraControl.h"

#include <QDebug>
#include <QTimer>
#include <QImage>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgproc.hpp>
#include "cuda_runtime.h"
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudabgsegm.hpp>
#include <opencv2/cudacodec.hpp>
#include <opencv2/cudafeatures2d.hpp>
#include <opencv2/cudafilters.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudalegacy.hpp>
#include <opencv2/cudaobjdetect.hpp>
#include <opencv2/cudaoptflow.hpp>
#include <opencv2/cudastereo.hpp>
#include <opencv2/cudawarping.hpp>
#include "workthread.h"
#include <QThread>
#include <QTextCodec>
using namespace cv;
using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    void startthread();
    QWidget *newWidget;
    ~MainWindow();

    void static __stdcall ImageCallBack(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser);
    void ImageCallBackInner(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo);
    void ImageCallBackInner1(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo);

    bool IsColor(MvGvspPixelType enType);
    bool IsMono(MvGvspPixelType enType);
    QImage  Mat2QImage(Mat cvImg);

//private:
//    void ShowErrorMsg(QString csMessage, int nErrorNum);
private slots:

//    void on_bnEnum_clicked();

    void on_bnOpen_clicked();

    void on_bnClose_clicked();

//    void on_bnContinuesMode_clicked();

//    void on_bnTriggerMode_clicked();

//    void on_bnStart_clicked();

//    void on_bnStop_clicked();

//   void on_cbSoftTrigger_clicked();

//    void on_bnTriggerExec_clicked();

    void on_bnGetParam_clicked();

    void on_bnSetParam_clicked();

    void on_bnInputOneFrame_clicked();

    void on_bnStopRecord_clicked();

//    void on_bnStartGetFrame_clicked();

//    void on_bnOverGetFrame_clicked();

//    void on_bnInputVideo_clicked();

    void on_bnGetOneFrame_clicked();

private:
    Ui::MainWindow *ui;

    void *m_hWnd;
    void *m_widgethWnd;

    MV_CC_DEVICE_INFO_LIST  m_stDevList; // 定义设备结构体
    CMvCamera*              m_pcMyCamera;  // 相机类指针
    bool                    m_bGrabbing;   // 是否开始抓图
    bool                    g_bExit;    //
    bool                    g_cExit;    //
//    unsigned char *pDataForSaveImage = NULL;    // 定义保存图片
    unsigned int nPacketSize = 0;
    MVCC_INTVALUE_EX stParam;

    QImage                  imag, svimag, svimag2;
    Mat                     cap;
    int a,d;
//    VideoCapture    capture; //声明视频读入类
    WorkThread * thread;

};

#endif // MAINWINDOW_H
