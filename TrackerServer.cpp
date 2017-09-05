#include "TrackerServer.h"
#include "serialworker.h"
#include "TrackerImageCapture.h"
#include "TrackerImageProcessor.h"
#include "TrackerMessageTranslator.h"
#include "Tracker.h"

#include <QCoreApplication>


const int MAX_CONNECTIONS = 1;

TrackerServer::TrackerServer(QObject *parent)
    : QObject(parent),
	m_Capture(new TrackerImageCapture),
	m_Processor(new TrackerImageProcessor),
    m_serial(new QSerialPort()),

{

    serialThread = new Thread(this);


	connect(m_Capture, SIGNAL(captured()), this, SIGNAL(captured()));
	connect(this, SIGNAL(processed()), m_Capture, SLOT(capture()));
    connect(m_Capture, SIGNAL(errorOccured(const QByteArray&)), this, SLOT(handleError(const QByteArray&)));
	connect(this, SIGNAL(startTracking()), m_Capture, SLOT(startCapture()));
	connect(this, SIGNAL(stopTracking()), m_Capture, SLOT(stopCapture()));
	connect(this, SIGNAL(startManufacturing()), m_Capture, SLOT(startCapture()));
	connect(this, SIGNAL(stopManufacturing()), m_Capture, SLOT(stopCapture()));
	connect(this, SIGNAL(startDiagnosing()), m_Capture, SLOT(startCapture()));
	connect(this, SIGNAL(stopDiagnosing()), m_Capture, SLOT(stopCapture()));

	connect(this, SIGNAL(startTracking()), m_Processor, SLOT(startTracking()));
	connect(this, SIGNAL(stopTracking()), m_Processor, SLOT(stopTracking()));
	connect(this, SIGNAL(startManufacturing()), m_Processor, SLOT(startManufacturing()));
	connect(this, SIGNAL(stopManufacturing()), m_Processor, SLOT(stopManufacturing()));
	connect(this, SIGNAL(startDiagnosing()), m_Processor, SLOT(startDiagnosing()));
	connect(this, SIGNAL(stopDiagnosing()), m_Processor, SLOT(stopDiagnosing()));
    connect(this, SIGNAL(busJam()), m_Processor, SLOT(busJam()));
    connect(this, SIGNAL(busIdle()), m_Processor, SLOT(busIdle()));
	connect(this, SIGNAL(captured()), m_Processor, SLOT(process()));
	connect(m_Processor, SIGNAL(processed()), this, SIGNAL(processed()));
    connect(m_Processor, SIGNAL(result(const QByteArray&)), this, SLOT(sendMessage(const QByteArray&)));


    QThread *imgThread = new QThread(qApp);
    m_Capture->moveToThread(imgThread);
    m_Processor->moveToThread(imgThread);
    imgThread->start();

}

TrackerServer::~TrackerServer()
{

}


