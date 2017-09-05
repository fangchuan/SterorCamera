#include "TrackerServer.h"
#include "serialworker.h"
#include "TrackerImageCapture.h"
#include "TrackerImageProcessor.h"
#include "TrackerMessageTranslator.h"
#include "Tracker.h"

#include <QCoreApplication>



TrackerServer::TrackerServer(QObject *parent)
    : QObject(parent)

{
    m_serial = new SerialWorker();
    serialThread = new Thread(this);
    m_serial->moveToThread(serialThread);

    m_Capture = new TrackerImageCapture();
    m_Processor = new TrackerImageProcessor();
    imgThread = new Thread(this);
    m_Capture->moveToThread(imgThread);
    m_Processor->moveToThread(imgThread);

    connect(serialThread, &QThread::started, m_serial, &SerialWorker::setup);
    connect(serialThread, &QThread::finished,  m_serial, &SerialWorker::deleteLater);
    connect(serialThread, &QThread::finished, serialThread, &SerialWorker::deleteLater);

    connect(imgThread, &QThread::finished,  m_Capture, &SerialWorker::deleteLater);
    connect(imgThread, &QThread::finished,  m_Processor, &SerialWorker::deleteLater);
    connect(imgThread, &QThread::finished, imgThread, &SerialWorker::deleteLater);

    connect(m_Capture, SIGNAL(captured()), m_Processor, SLOT(process());
    connect(m_Processor, SIGNAL(processed()), m_Capture, SLOT(capture()));
    connect(m_Capture, SIGNAL(errorOccured(const QByteArray&)), m_serial, SLOT(handleError(const QByteArray&)));
    connect(m_serial, SIGNAL(startTracking()), m_Capture, SLOT(startCapture()));
    connect(m_serial, SIGNAL(stopTracking()), m_Capture, SLOT(stopCapture()));
    connect(m_serial, SIGNAL(startManufacturing()), m_Capture, SLOT(startCapture()));
    connect(m_serial, SIGNAL(stopManufacturing()), m_Capture, SLOT(stopCapture()));
    connect(m_serial, SIGNAL(startDiagnosing()), m_Capture, SLOT(startCapture()));
    connect(m_serial, SIGNAL(stopDiagnosing()), m_Capture, SLOT(stopCapture()));

    connect(m_serial, SIGNAL(startTracking()), m_Processor, SLOT(startTracking()));
    connect(m_serial, SIGNAL(stopTracking()), m_Processor, SLOT(stopTracking()));
    connect(m_serial, SIGNAL(startManufacturing()), m_Processor, SLOT(startManufacturing()));
    connect(m_serial, SIGNAL(stopManufacturing()), m_Processor, SLOT(stopManufacturing()));
    connect(m_serial, SIGNAL(startDiagnosing()), m_Processor, SLOT(startDiagnosing()));
    connect(m_serial, SIGNAL(stopDiagnosing()), m_Processor, SLOT(stopDiagnosing()));
    connect(m_serial, SIGNAL(busJam()), m_Processor, SLOT(busJam()));
    connect(m_serial, SIGNAL(busIdle()), m_Processor, SLOT(busIdle()));
    connect(m_serial, SIGNAL(captured()), m_Processor, SLOT(process()));
    connect(m_Processor, SIGNAL(result(const QByteArray&)), m_serial, SLOT(sendMessage(const QByteArray&)));

    serialThread->start(QThread::TimeCriticalPriority);
    imgThread->start(QThread::HighPriority);

}

TrackerServer::~TrackerServer()
{

}


