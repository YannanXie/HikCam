#ifndef WORKTHREAD_H
#define WORKTHREAD_H

#include "MvCamera.h"
#include "MvCameraControl.h"


#include <QObject>
#include <QThread>


class WorkThread : public QThread
{
    Q_OBJECT
public:
    WorkThread();
    void run() override;
    void stop();
    void *m_hWnd;
    void *m_widgethWnd;
    bool                    g_bExit;
    MV_CC_DEVICE_INFO_LIST  m_stDevList;
    CMvCamera*              m_pcMyCamera;
    bool                    m_bGrabbing;
    MVCC_INTVALUE_EX        stParam;

//private:


};

#endif // WorkThread_H
