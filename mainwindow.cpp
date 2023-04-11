#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDebug>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    thread(new WorkThread),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qDebug()<<"main PID:"<<QThread::currentThreadId();
    // MV_CC_DEVICE_INFO_LIST（ nDeviceNum, MV_CC_DEVICE_INFO * pDeviceInfo ）
    //                          在线设备数量，设备信息 * 支持最多256个设备
    memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    m_pcMyCamera = NULL;    // 相机类指针
    m_bGrabbing = false;    // 是否开始抓图
    m_hWnd = (void*)ui->widgetDisplay->winId();
    g_bExit = false;
    g_cExit = false;
    a = 0;
    d = 0;
}
MainWindow::~MainWindow()
{
    delete ui;
}

void __stdcall MainWindow::ImageCallBack(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser)
{
    if(pUser)
    {
        MainWindow *pMainWindow = (MainWindow*)pUser;
        pMainWindow->ImageCallBackInner(pData, pFrameInfo);
        pMainWindow->ImageCallBackInner1(pData, pFrameInfo);
    }
}

void MainWindow::ImageCallBackInner(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo)
{
    MV_DISPLAY_FRAME_INFO stDisplayInfo;
    memset(&stDisplayInfo, 0, sizeof(MV_DISPLAY_FRAME_INFO));

    stDisplayInfo.hWnd = m_hWnd;    // 窗口句柄
    stDisplayInfo.pData = pData;    // 显示的数据
    stDisplayInfo.nDataLen = pFrameInfo->nFrameLen;     // 数据长度
    stDisplayInfo.nWidth = pFrameInfo->nWidth;      // 图像宽
    stDisplayInfo.nHeight = pFrameInfo->nHeight;    // 图像高
    stDisplayInfo.enPixelType = pFrameInfo->enPixelType;    // 像素格式
    m_pcMyCamera->DisplayOneFrame(&stDisplayInfo);      // 设置显示窗口句柄：成功返回 MV_OK，失败返回错误码
}

void MainWindow::ImageCallBackInner1(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo)
{
    if(g_bExit)
    {
        MV_FRAME_OUT stImageInfo;
        memset(&stImageInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX));
        MV_CC_INPUT_FRAME_INFO stInputFrameInfo;
        memset(&stInputFrameInfo, 0, sizeof(MV_CC_INPUT_FRAME_INFO));
        stImageInfo.stFrameInfo.enPixelType = pFrameInfo->enPixelType;
        stImageInfo.stFrameInfo.nWidth = pFrameInfo->nWidth;
        stImageInfo.stFrameInfo.nHeight = pFrameInfo->nHeight;
        stImageInfo.stFrameInfo.nFrameNum = pFrameInfo->nFrameNum;
        qDebug("[%d]",stImageInfo.stFrameInfo.nFrameNum);
        stInputFrameInfo.pData = pData;
        stInputFrameInfo.nDataLen = pFrameInfo->nFrameLen;

        Mat img(stImageInfo.stFrameInfo.nHeight, stImageInfo.stFrameInfo.nWidth,
                CV_8UC3, stInputFrameInfo.pData);
        svimag = Mat2QImage(img);
        svimag = svimag.scaled(ui->lbDisplayFrame->width(), ui->lbDisplayFrame->height(),
                               Qt::IgnoreAspectRatio, Qt::SmoothTransformation);//设置图片大小和label的长宽一致
        ui->lbDisplayFrame->setPixmap(QPixmap::fromImage(svimag));
        if(g_cExit){
            ui->lbShowFrame->setPixmap(QPixmap::fromImage(svimag));
            QString imageName = QString("/home/nvidia/Desktop/OpenHik_0308_2/Videoimag%1.bmp").arg(d);
            bool saved = imwrite(imageName.toStdString(), img);
            d++;
            if (saved){
                cout<<"保存图像成功！"<<endl;
            }
            g_cExit = false;
        }
    }
}

bool MainWindow::IsColor(MvGvspPixelType enType)
{
    switch(enType)
    {
    case PixelType_Gvsp_BGR8_Packed:
    case PixelType_Gvsp_YUV422_Packed:
    case PixelType_Gvsp_YUV422_YUYV_Packed:
    case PixelType_Gvsp_BayerGR8:
    case PixelType_Gvsp_BayerRG8:
    case PixelType_Gvsp_BayerGB8:
    case PixelType_Gvsp_BayerBG8:
    case PixelType_Gvsp_BayerGB10:
    case PixelType_Gvsp_BayerGB10_Packed:
    case PixelType_Gvsp_BayerBG10:
    case PixelType_Gvsp_BayerBG10_Packed:
    case PixelType_Gvsp_BayerRG10:
    case PixelType_Gvsp_BayerRG10_Packed:
    case PixelType_Gvsp_BayerGR10:
    case PixelType_Gvsp_BayerGR10_Packed:
    case PixelType_Gvsp_BayerGB12:
    case PixelType_Gvsp_BayerGB12_Packed:
    case PixelType_Gvsp_BayerBG12:
    case PixelType_Gvsp_BayerBG12_Packed:
    case PixelType_Gvsp_BayerRG12:
    case PixelType_Gvsp_BayerRG12_Packed:
    case PixelType_Gvsp_BayerGR12:
    case PixelType_Gvsp_BayerGR12_Packed:
        return true;
    default:
        return false;
    }
}

bool MainWindow::IsMono(MvGvspPixelType enType)
{
    switch(enType)
    {
    case PixelType_Gvsp_Mono10:
    case PixelType_Gvsp_Mono10_Packed:
    case PixelType_Gvsp_Mono12:
    case PixelType_Gvsp_Mono12_Packed:
        return true;
    default:
        return false;
    }
}

// 查找、打开设备，采集图像
void MainWindow::on_bnOpen_clicked()
{
    ui->PixelFormatComboBox->clear();

    // ch:枚举子网内所有设备 | en:Enumerate all devices within subnet
    memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    int nRet = CMvCamera::EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &m_stDevList);
    // 枚举设备：成功返回 MV_OK，失败返回错误码
    // nTLayerType = MV_GIGE_DEVICE | MV_USB_DEVICE，表示查找 GigE 和 USB3.0 设备
    if (MV_OK != nRet)
    {
        return;
    }
    int nIndex = 0;

    if(m_pcMyCamera == NULL)
    {
        m_pcMyCamera = new CMvCamera;
        if (NULL == m_pcMyCamera)
        {
            return;
        }
    }

    nRet = m_pcMyCamera->Open(m_stDevList.pDeviceInfo[nIndex]);
    if (MV_OK != nRet)
    {
        delete m_pcMyCamera;
        m_pcMyCamera = NULL;
        return;
    }

    // ch:获取数据包大小 | en:Get payload size
//    MVCC_INTVALUE_EX stParam;
    memset(&stParam, 0, sizeof(MVCC_INTVALUE_EX));
    nRet = m_pcMyCamera->GetIntValue("PayloadSize", &stParam);
    if (MV_OK != nRet)
    {
    printf("Get PayloadSize fail! nRet [0x%x]\n", nRet);
    }

    m_pcMyCamera->SetEnumValue("AcquisitionMode", MV_ACQ_MODE_CONTINUOUS);  // 设置为连续采集模式
    m_pcMyCamera->SetEnumValue("TriggerMode", MV_TRIGGER_MODE_OFF);     // 关闭触发模式
    m_pcMyCamera->SetEnumValue("ExposureAuto",2);   // 自动曝光连续
    m_pcMyCamera->SetEnumValue("GainAuto",2);   // 自动增益连续

    on_bnGetParam_clicked(); // ch:获取参数 | en:Get Parameter

    ui->bnOpen->setEnabled(false);
    ui->bnClose->setEnabled(true);

    ui->tbExposure->setEnabled(true);
    ui->tbGain->setEnabled(true);
    ui->tbFrameRate->setEnabled(true);
    ui->bnSetParam->setEnabled(true);
    ui->bnGetParam->setEnabled(true);

//    ui->bnStartGetFrame->setEnabled(true);
    nRet = m_pcMyCamera->SetImageNodeNum(3);
    qDebug("SetImageNodeNum %x",nRet);
//    m_pcMyCamera->RegisterImageCallBack(ImageCallBack, this);

//    nRet = m_pcMyCamera->StartGrabbing();
//    m_bGrabbing = true;

//    if (MV_OK != nRet)
//    {
//        printf("Start Grabbing fail! nRet [0x%x]\n", nRet);
//    }

//    nRet = m_pcMyCamera->StartGrabbing();
//    m_bGrabbing = true;
//    m_widgethWnd = (void*)ui->widgetDisplay->winId();
//    m_pcMyCamera->Display(m_widgethWnd);
    //录像参数
//    MVCC_ENUMVALUE stEnumValue;
//    memset(&stEnumValue, 0, sizeof(MVCC_ENUMVALUE));
//    MV_CC_RECORD_PARAM stRecordPar;
//    MVCC_INTVALUE stParam;
//    memset(&stParam, 0, sizeof(MVCC_INTVALUE));
//    nRet = m_pcMyCamera->GetIntValue2("Width", &stParam);
//    qDebug("getWidth %x",nRet);
//    stRecordPar.nWidth = stParam.nCurValue;
//    nRet = m_pcMyCamera->GetIntValue2("Height", &stParam);
//    qDebug("getHeight %x",nRet);
//    stRecordPar.nHeight = stParam.nCurValue;
//    nRet = m_pcMyCamera->GetEnumValue("PixelFormat", &stEnumValue);
//    qDebug("getPixelFormat %x",nRet);
//    stRecordPar.enPixelType = MvGvspPixelType(stEnumValue.nCurValue);
//    MVCC_FLOATVALUE stFloatValue;
//    nRet = m_pcMyCamera->GetFloatValue("ResultingFrameRate", &stFloatValue);
//    qDebug("getResultingFrameRate %x",nRet);
//    // Frame Rate (>1/16)fps
//    stRecordPar.fFrameRate = 29;
//    // Bitrate kbps(128kbps-16Mbps)
//    stRecordPar.nBitRate = 1000;
//    // Record Format(AVI is only supported)
//    stRecordPar.enRecordFmtType = MV_FormatType_AVI;
//    stRecordPar.strFilePath= "/home/nvidia/Desktop/Computer_vision/OpenHik_1125/Recording.avi";
//    nRet = m_pcMyCamera->StartRecord(&stRecordPar);
//    qDebug("StartRecord %x",nRet);

    nRet = m_pcMyCamera->RegisterImageCallBackForRGB(ImageCallBack, this);
    qDebug("RegisterImageCallBack %x",nRet);

//    nRet = m_pcMyCamera->SetBayerCvtQuality(2);
//    qDebug("SetBayerCvtQuality %x",nRet);

    nRet = m_pcMyCamera->StartGrabbing();
    qDebug("StartGrabbing %x",nRet);

}

// 停止采集图像，关闭设备
void MainWindow::on_bnClose_clicked()
{
    m_pcMyCamera->StopGrabbing();
    m_bGrabbing = false;    // 设置是否抓图 bool 参数为 false

    if (m_pcMyCamera)
    {
        m_pcMyCamera->Close();
        delete m_pcMyCamera;
        m_pcMyCamera = NULL;
    }

    ui->bnOpen->setEnabled(true);
    ui->bnClose->setEnabled(false);

    ui->tbExposure->setEnabled(false);
    ui->tbGain->setEnabled(false);
    ui->tbFrameRate->setEnabled(false);
    ui->bnSetParam->setEnabled(false);
    ui->bnGetParam->setEnabled(false);
}

// 获取参数
void MainWindow::on_bnGetParam_clicked()
{
    MVCC_FLOATVALUE stFloatValue;   // 定义 Float 类型节点值
    memset(&stFloatValue, 0, sizeof(MVCC_FLOATVALUE));

    int nRet = m_pcMyCamera->GetFloatValue("ExposureTime", &stFloatValue);
    if (MV_OK == nRet)
    {
        ui->tbExposure->setText(QString("%1").arg(stFloatValue.fCurValue));   // 曝光
    }

    nRet = m_pcMyCamera->GetFloatValue("Gain", &stFloatValue);
    if (MV_OK == nRet)
    {
        ui->tbGain->setText(QString("%1").arg(stFloatValue.fCurValue));   // 增益
    }

    nRet = m_pcMyCamera->GetFloatValue("ResultingFrameRate", &stFloatValue);
    if (MV_OK == nRet)
    {
        ui->tbFrameRate->setText(QString("%1").arg(stFloatValue.fCurValue));   // 帧率
    }

//    int nIndex = ui->PixelFormatComboBox->currentIndex();
//    nRet = m_pcMyCamera->GetFloatValue("ResultingFrameRate", &stFloatValue);
//    if (MV_OK == nRet)
//    {
//        ui->tbFrameRate->setText(QString("%1").arg(stFloatValue.fCurValue));   // 帧率
//    }
}

// 设置参数
void MainWindow::on_bnSetParam_clicked()
{
    m_pcMyCamera->SetEnumValue("ExposureAuto", 0);
    m_pcMyCamera->SetFloatValue("ExposureTime", ui->tbExposure->text().toFloat());   // 读取并设置曝光参数

    m_pcMyCamera->SetEnumValue("GainAuto", 0);
    m_pcMyCamera->SetFloatValue("Gain", ui->tbGain->text().toFloat());   // 读取并设置增益参数

    m_pcMyCamera->SetFloatValue("AcquisitionFrameRate", ui->tbFrameRate->text().toFloat());   // 读取并设置帧率参数
}

// 图片转换：Qlabel上只能显示QImage的图像，因此将QImage转化为Mat
QImage  MainWindow::Mat2QImage(Mat cvImg)
{
    QImage qImg;
    if(cvImg.channels()==3)     //3 channels color image
    {

        cv::cvtColor(cvImg,cvImg,CV_BGR2RGB);
        qImg =QImage((const unsigned char*)(cvImg.data),
                    cvImg.cols, cvImg.rows,
                    cvImg.cols*cvImg.channels(),
                    QImage::Format_RGB888);
    }
    else if(cvImg.channels()==1)                    //grayscale image
    {
        qImg =QImage((const unsigned char*)(cvImg.data),
                    cvImg.cols,cvImg.rows,
                    cvImg.cols*cvImg.channels(),
                    QImage::Format_Indexed8);
    }
    else
    {
        qImg =QImage((const unsigned char*)(cvImg.data),
                    cvImg.cols,cvImg.rows,
                    cvImg.cols*cvImg.channels(),
                    QImage::Format_RGB888);
    }
    return qImg;
}

// 获取视频
void MainWindow::on_bnInputOneFrame_clicked()
{
    g_bExit = true;
}

// 停止录制
void MainWindow::on_bnStopRecord_clicked()
{
    g_bExit = false;
    int nRet = m_pcMyCamera->StopGrabbing();
    qDebug("StopGrabbing %x",nRet);
    nRet = m_pcMyCamera->StopRecord();
    qDebug("StopRecord %x",nRet);
    m_pcMyCamera->StartGrabbing();
}

void MainWindow::on_bnGetOneFrame_clicked()
{
    g_cExit = true;
}
