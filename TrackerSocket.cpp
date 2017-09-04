#include "TrackerSocket.h"
#include "TrackerMessageTranslator.h"
#include "Tracker.h"

#include <QFile>
#include <QTimer>
#include <QThread>

TrackerSocket::TrackerSocket(QObject *parent)
	: QTcpSocket(parent),
	m_IsUploadingFile(false),
	m_IsAppendingFile(false)
{
	setSocketOption(QTcpSocket::LowDelayOption, 1);
	setSocketOption(QTcpSocket::KeepAliveOption, 1);
	connect(this, SIGNAL(readyRead()), this, SLOT(readData()));
	QTimer *timer = new QTimer;
	connect(timer, SIGNAL(timeout()), this, SLOT(checkSocketJam()), Qt::DirectConnection);
	connect(this, SIGNAL(startTimer(int)), timer, SLOT(start(int)));
	connect(this, SIGNAL(stopTimer()), timer, SLOT(stop()));
	connect(this, SIGNAL(destroyed()), timer, SLOT(deleteLater()));
	QThread *thread = new QThread;
	connect(timer, SIGNAL(destroyed()), thread, SLOT(quit()));
	connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
	timer->moveToThread(thread);
	thread->start();
}

TrackerSocket::~TrackerSocket()
{
	
}

void TrackerSocket::readData()
{
	if (bytesAvailable())
	{
		QByteArray request = readAll();
		if (m_IsUploadingFile)
		{
			handleUploadFile(request);
			return ;
		}
		QDataStream stream(request);
		QString order;
		stream >> order;
		if (order.toLower() == "uploadfile")
		{
			m_IsUploadingFile = true;
			handleUploadFile(request);
			return ;
		}
		switch(TrackerMessageTranslator::translate(request))
		{
		case Tracker::START_TRACKING:
			{
				emit startTracking();
			}
			break;
		case Tracker::STOP_TRACKING:
			{
				emit stopTracking();
			}
			break;
		case Tracker::START_MANUFACTURING:
			{
				emit startManufacturing();
			}
			break;
		case Tracker::MANUFACTURE:
			{
				emit manufacture();
			}
			break;
		case Tracker::STOP_MANUFACTURING:
			{
				emit stopManufacturing();
			}
			break;
		case Tracker::START_DIAGNOSING:
			{
				emit startDiagnosing();
			}
			break;
		case Tracker::STOP_DIAGNOSING:
			{
				emit stopDiagnosing();
			}
			break;
		}
	}
}
//向上位机反馈信息(目标点坐标、错误信息)
void TrackerSocket::sendMessage(const QByteArray &bytes)
{
	// 1M buffer
	if (bytesToWrite() > 1024*1024)
	{
		emit startTimer(20);
		return ;
	}
	write(bytes);
	flush();
}

//上传配置文件
void TrackerSocket::handleUploadFile(const QByteArray& data)
{
	if (!m_IsAppendingFile)
	{
		// 文件的第一部分
		// "UPLOADFILE" | FILE_NAME | FILE_LEGTH(qint64) | FILE_DATA
		m_FileData.clear();
		QDataStream stream(data);
		QString order;
		stream >> order;
		stream >> m_Filename;
		stream >> m_FileLength;
		int pos = stream.device()->pos();
		m_FileData = data.right(data.size()-pos);
		if (m_FileData.length() >= m_FileLength)
		{
			saveFile();
			m_IsUploadingFile = false;
		}
		else
		{
			m_IsAppendingFile = true;
		}
	}
	else
	{
		// 文件后续部分
		m_FileData.append(data);
		if (m_FileData.length() >= m_FileLength)
		{
			// 保存file
			saveFile();
			m_IsAppendingFile = m_IsUploadingFile = false;
		}
	}
}

void TrackerSocket::saveFile()
{
	QFile file(m_Filename);
	file.open(QIODevice::WriteOnly);
	file.write(m_FileData);
	file.close();
}

void TrackerSocket::checkSocketJam()
{
	if (bytesToWrite() > 0)
	{
		emit socketJam();
	}
	else
	{
		emit stopTimer();
		emit socketIdle();
	}
}