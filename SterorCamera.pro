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

LIBS += /usr/lib/libflycapture.so \
        /usr/local/lib/libopencv_highgui.so \
        /usr/local/lib/libopencv_imgproc.so \
        /usr/local/lib/libopencv_core.so


SOURCES += main.cpp \
    TrackerImageCapture.cpp \
    TrackerImagepool.cpp \
    TrackerImageProcessor.cpp \
    TrackerLightsController.cpp \
    TrackerMessageTranslator.cpp \
    TrackerServer.cpp \
    serialworker.cpp \
#    trackingdevice.cpp \
#    trackingtool.cpp \
#    vpsvector.cpp \
#    vpsndiprotocol.cpp \
#    vpsserialcommunication.cpp \
#    vpsnditrackingdevice.cpp \
#    vpsinternaltrackingtool.cpp \
#    vpsigttimestamp.cpp \
#    vpsrealtimeclock.cpp \
#    vpsndipassivetool.cpp \
    serialinterpreter.cpp \
    vpstoolmanager.cpp

HEADERS += \
    Tracker.h \
    TrackerCommon.h \
    TrackerImageCapture.h \
    TrackerImagepool.h \
    TrackerImageProcessor.h \
    TrackerLightsController.h \
    TrackerMessageTranslator.h \
    TrackerServer.h \
    serialworker.h \
#    trackingdevice.h \
#    trackingtool.h \
#    vpscommon.h \
#    vpsvector.h \
#    vpsndiprotocol.h \
#    vpsserialcommunication.h \
#    vpsnditrackingdevice.h \
#    vpsinternaltrackingtool.h \
#    vpsigttimestamp.h \
#    vpsrealtimeclock.h \
#    trackingtypes.h \
#    vpsndipassivetool.h \
#    vps_quaternion.h \
    serialinterpreter.h \
    vpstoolmanager.h \
    trackingtypes.h \
    includes.h

DISTFILES += \
    README.md
