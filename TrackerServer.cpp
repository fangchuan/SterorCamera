#include "TrackerServer.h"
#include "TrackerSocket.h"
#include "TrackerImageCapture.h"
#include "TrackerImageProcessor.h"

#include <QThread>
#include <QCoreApplication>

const int MAX_CONNECTIONS = 1;

TrackerServer::TrackerServer(QObject *parent)
	: QTcpServer(parent),
	m_Capture(new TrackerImageCapture),
	m_Processor(new TrackerImageProcessor),
	m_ClientNumber(0)
{
	connect(m_Capture, SIGNAL(captured()), this, SIGNAL(captured()));
	connect(this, SIGNAL(processed()), m_Capture, SLOT(capture()));
	connect(m_Capture, SIGNAL(errorOccured(const QByteArray&)), this, SLOT(captureError(const QByteArray&)));
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
	connect(this, SIGNAL(socketJam()), m_Processor, SLOT(socketJam()));
	connect(this, SIGNAL(socketIdle()), m_Processor, SLOT(socketIdle()));
	connect(this, SIGNAL(captured()), m_Processor, SLOT(process()));
	connect(m_Processor, SIGNAL(processed()), this, SIGNAL(processed()));
	connect(m_Processor, SIGNAL(result(const QByteArray&)), this, SIGNAL(sendMessage(const QByteArray&)));

	QThread *captureThread = new QThread(qApp);
	m_Capture->moveToThread(captureThread);
	captureThread->start();

	QThread *processThread = new QThread(qApp);
	m_Processor->moveToThread(processThread);
	processThread->start();

	//emit startTracking();
}

TrackerServer::~TrackerServer()
{

}

void TrackerServer::incomingConnection(int socketDescriptor)
{
	++m_ClientNumber;
	TrackerSocket *socket = new TrackerSocket;
	socket->setSocketDescriptor(socketDescriptor);
	QThread *thread = new QThread;
	connect(socket, SIGNAL(disconnected()), this, SLOT(disconnectFromClient()));
	connect(socket, SIGNAL(disconnected()), thread, SLOT(quit()));
	connect(thread, SIGNAL(finished()), socket, SLOT(deleteLater()));
	connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
	connect(socket, SIGNAL(startTracking()), this, SIGNAL(startTracking()));
	connect(socket, SIGNAL(stopTracking()), this, SIGNAL(stopTracking()));
	connect(socket, SIGNAL(startManufacturing()), this, SIGNAL(startManufacturing()));
	connect(socket, SIGNAL(manufacture()), this, SIGNAL(processed()));
	connect(socket, SIGNAL(stopManufacturing()), this, SIGNAL(stopManufacturing()));
	connect(socket, SIGNAL(startDiagnosing()), this, SIGNAL(startDiagnosing()));
	connect(socket, SIGNAL(stopDiagnosing()), this, SIGNAL(stopDiagnosing()));
	connect(socket, SIGNAL(socketJam()), this, SIGNAL(socketJam()));
	connect(socket, SIGNAL(socketIdle()), this, SIGNAL(socketIdle()));
	connect(this, SIGNAL(sendMessage(const QByteArray&)), socket, SLOT(sendMessage(const QByteArray&)));
	socket->moveToThread(thread);
	thread->start();

	if (m_ClientNumber > MAX_CONNECTIONS)
	{
		QByteArray errorMsg;
		QDataStream stream(&errorMsg, QIODevice::WriteOnly);
		stream << QString("ERROR");
		int lengthPos = stream.device()->pos();
		stream << qint64(0) << QString("Connect");
		stream << QString("Too many client received.");
		int length = errorMsg.length();
		stream.device()->seek(lengthPos);
		stream << qint64(length);
		emit sendMessage(errorMsg);
		socket->disconnectFromHost();
	}
}

void TrackerServer::disconnectFromClient()
{
	--m_ClientNumber;
	if (m_ClientNumber <= 0)
	{
		emit stopTracking();
		m_ClientNumber = 0;
	}
	emit socketIdle();
}

void TrackerServer::captureError(const QByteArray& error)
{
	emit sendMessage(error);
}