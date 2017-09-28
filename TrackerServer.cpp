#include "includes.h"

#include <QCoreApplication>



TrackerServer::TrackerServer(QObject *parent)
    : QObject(parent)

{
    m_serial = new SerialWorker();
    serialThread = new QThread(this);
    m_serial->moveToThread(serialThread);

    m_Capture = new TrackerImageCapture();
    m_Processor = new TrackerImageProcessor();
    imgThread = new QThread(this);
    m_Capture->moveToThread(imgThread);
    m_Processor->moveToThread(imgThread);

    connect(m_serial, &SerialWorker::serialWorkerStop, serialThread, &QThread::quit);
    connect(serialThread, &QThread::started, m_serial, &SerialWorker::setUpDefault);
    connect(serialThread, &QThread::finished,  m_serial, &SerialWorker::deleteLater);
    connect(serialThread, &QThread::finished, serialThread, &QThread::deleteLater);

    connect(serialThread, &QThread::finished, imgThread, &QThread::quit);
    connect(imgThread, &QThread::finished,  m_Capture, &SerialWorker::deleteLater);
    connect(imgThread, &QThread::finished,  m_Processor, &SerialWorker::deleteLater);
    connect(imgThread, &QThread::finished, imgThread, &QThread::deleteLater);

//    connect(m_Capture, SIGNAL(captured()), m_Processor, SLOT(process()));
//    connect(m_Processor, SIGNAL(processed()), m_Capture, SLOT(capture()));
    connect(m_Capture, SIGNAL(errorOccured(const QByteArray&)),
            m_serial, SLOT(handleCameraError(const QByteArray&)), Qt::DirectConnection);
    connect(m_serial, SIGNAL(startTracking()), m_Capture, SLOT(startCapture()), Qt::DirectConnection);
    connect(m_serial, SIGNAL(stopTracking()), m_Capture, SLOT(stopCapture()), Qt::DirectConnection);
    /*connect(m_serial, SIGNAL(startManufacturing()), m_Capture, SLOT(startCapture()));
    connect(m_serial, SIGNAL(stopManufacturing()), m_Capture, SLOT(stopCapture()));
    connect(m_serial, SIGNAL(startDiagnosing()), m_Capture, SLOT(startCapture()));
    connect(m_serial, SIGNAL(stopDiagnosing()), m_Capture, SLOT(stopCapture()));*/

    connect(m_serial, SIGNAL(startTracking()), m_Processor, SLOT(startTracking()), Qt::DirectConnection);
    connect(m_serial, SIGNAL(stopTracking()), m_Processor, SLOT(stopTracking()), Qt::DirectConnection);
    connect(m_serial, SIGNAL(startProcess3D()), m_Processor, SLOT(process3D()), Qt::DirectConnection);
    connect(m_serial, SIGNAL(startProcess6D()), m_Processor, SLOT(process6D()), Qt::DirectConnection);
    /*connect(m_serial, SIGNAL(startManufacturing()), m_Processor, SLOT(startManufacturing()));
    connect(m_serial, SIGNAL(stopManufacturing()), m_Processor, SLOT(stopManufacturing()));
    connect(m_serial, SIGNAL(startDiagnosing()), m_Processor, SLOT(startDiagnosing()));
    connect(m_serial, SIGNAL(stopDiagnosing()), m_Processor, SLOT(stopDiagnosing()));*/
    connect(m_serial, SIGNAL(busJam()), m_Processor, SLOT(busJam()), Qt::DirectConnection);
    connect(m_serial, SIGNAL(busIdle()), m_Processor, SLOT(busIdle()), Qt::DirectConnection);
    connect(m_Processor, SIGNAL(result(const QByteArray&)),
            m_serial, SLOT(sendPosData(QByteArray)), Qt::DirectConnection);

    serialThread->start(QThread::TimeCriticalPriority);
    imgThread->start(QThread::HighPriority);


}

TrackerServer::~TrackerServer()
{
    if(serialThread->isRunning()){
        serialThread->quit();
        serialThread->wait();
    }

    if(imgThread->isRunning()){
        imgThread->quit();
        imgThread->wait();
    }
}


