QT += core  serialport
QT -= gui

CONFIG += c++11

TARGET = SterorCamera
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app


INCLUDEPATH += /usr/local/include \
                /usr/local/include/opencv \
                /usr/local/include/opencv2  \
                /usr/include/flycapture

LIBS += /usr/local/lib/libopencv_highgui.so \
        /usr/local/lib/libopencv_imgproc.so \
        /usr/local/lib/libopencv_core.so    \
        /usr/lib/libflycapture.so


SOURCES += main.cpp \
    TrackerImageCapture.cpp \
    TrackerImagepool.cpp \
    TrackerImageProcessor.cpp \
    TrackerLightsController.cpp \
    TrackerMessageTranslator.cpp \
    TrackerServer.cpp \
    serialworker.cpp

HEADERS += \
    Tracker.h \
    TrackerCommon.h \
    TrackerImageCapture.h \
    TrackerImagepool.h \
    TrackerImageProcessor.h \
    TrackerLightsController.h \
    TrackerMessageTranslator.h \
    TrackerServer.h \
    serialworker.h
