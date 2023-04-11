#-------------------------------------------------
#
# Project created by QtCreator 2021-02-22T09:44:38
#
#-------------------------------------------------

QT       += core gui
CONFIG += c++11 console
CONFIG -= app_bundle
CONFIG += C++11  # 添加对C++11的支持

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BasicDemo
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
# Based on Qt5 development.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# opencv 路径
INCLUDEPATH += /usr/local/include/opencv4 #添加头文件路径

#LIBS += -L/usr/lib/aarch64-linux-gnu -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_highgui -lopencv_objdetect -lopencv_imgproc -lopencv_videoio #添加需要链接的库
LIBS += -L/usr/local/lib -lopencv_gapi -lopencv_stitching -lopencv_alphamat -lopencv_aruco -lopencv_bgsegm -lopencv_bioinspired -lopencv_ccalib -lopencv_cudabgsegm -lopencv_cudafeatures2d -lopencv_cudaobjdetect -lopencv_cudastereo -lopencv_dnn_objdetect -lopencv_dnn_superres -lopencv_dpm -lopencv_highgui -lopencv_face -lopencv_freetype -lopencv_fuzzy -lopencv_hfs -lopencv_img_hash -lopencv_intensity_transform -lopencv_line_descriptor -lopencv_mcc -lopencv_quality -lopencv_rapid -lopencv_reg -lopencv_rgbd -lopencv_saliency -lopencv_stereo -lopencv_structured_light -lopencv_phase_unwrapping -lopencv_superres -lopencv_cudacodec -lopencv_surface_matching -lopencv_tracking -lopencv_datasets -lopencv_text -lopencv_dnn -lopencv_plot -lopencv_videostab -lopencv_cudaoptflow -lopencv_optflow -lopencv_cudalegacy -lopencv_videoio -lopencv_cudawarping -lopencv_xfeatures2d -lopencv_shape -lopencv_ml -lopencv_ximgproc -lopencv_video -lopencv_xobjdetect -lopencv_objdetect -lopencv_calib3d -lopencv_imgcodecs -lopencv_features2d -lopencv_flann -lopencv_xphoto -lopencv_photo -lopencv_cudaimgproc -lopencv_cudafilters -lopencv_imgproc -lopencv_cudaarithm -lopencv_core -lopencv_cudev


SOURCES += \
        main.cpp \
        mainwindow.cpp \
        MvCamera.cpp \
    workthread.cpp

HEADERS += \
        ui_mainwindow.h \
        mainwindow.h \
        MvCamera.h \
    workthread.h

FORMS += \
        mainwindow.ui

# 海康摄像头配置
INCLUDEPATH += ${MVCAM_SDK_PATH}/include

LIBS += -L${MVCAM_SDK_PATH}/bin -L${MVCAM_COMMON_RUNENV}/aarch64 -lMvCameraControl

unix:!macx: LIBS += -L$$PWD/../../../../../opt/MVS/lib/aarch64/ -lMvCameraControl

INCLUDEPATH += $$PWD/../../../../../opt/MVS/include
DEPENDPATH += $$PWD/../../../../../opt/MVS/include

# opencv & cuda 配置
CUDA_DIR = /usr/local/cuda-10.2
CUDA_LIBS = -lcudart -lcuda

INCLUDEPATH  += $$CUDA_DIR/include
QMAKE_LIBDIR += $$CUDA_DIR/lib64
LIBS += $$CUDA_LIBS

#CUDA编译器配置
#GPU架构
SYSTEM_TYPE = 64
CUDA_ARCH = sm_61
NVCCOPTIONS = -use_fast_math -O2

#调试模式下，-g 表示将CPU代码(host)编译成可调式版本，-G表示将GPU代码(kernel)编译成可调式版本。
debug
{
    NVCCOPTIONS += -g -G
}

CUDA_INC = $$join(INCLUDEPATH,' -I','-I',' ')

cuda.input = CUDA_SOURCES
cuda.output = ${OBJECTS_DIR}${QMAKE_FILE_BASE}_cuda.o
cuda.commands = $$CUDA_DIR/bin/nvcc $$NVCCOPTIONS $$CUDA_INC $$CUDA_LIBS --machine $$SYSTEM_TYPE -arch=$$CUDA_ARCH -c -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_NAME} 2>&1 | sed -r \"s/\\(([0-9]+)\\)/:\\1/g\" 1>&2
cuda.dependency_type = TYPE_C

QMAKE_EXTRA_COMPILERS += cuda
