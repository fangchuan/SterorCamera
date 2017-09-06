#include "serialworker.h"
#include "TrackerMessageTranslator.h"
#include "Tracker.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>

SerialWorker::SerialWorker(QObject *parent):
    QSerialPort(parent),
    m_timer(new QTimer(this)),
    m_IsUploadingFile(false),
    m_IsAppendingFile(false)

{
    connect(this, SIGNAL(startTimer(int)), m_timer, SLOT(start(int)));
    connect(this, SIGNAL(stopTimer()), m_timer, SLOT(stop()));
    connect(m_timer, SIGNAL(timeout()), this, SLOT(checkBusJam()));

    connect(this, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(this, SIGNAL(error(QSerialPort::SerialPortError)),
            this, SLOT(handleFatalError(QSerialPort::SerialPortError)));
}

SerialWorker::~SerialWorker()
{
    closeSerialPort();
}

void SerialWorker::readData()
{
    if (this->bytesAvailable())
    {
        QByteArray request = this->readAll();
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
                emit manufacting();
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
        qDebug()<< request;
    }

}

bool SerialWorker::setup()
{
    this->setPortName("ttyS0");
    this->setBaudRate(QSerialPort::Baud115200);
    this->setDataBits(QSerialPort::Data8);
    this->setParity(QSerialPort::NoParity);
    this->setStopBits(QSerialPort::OneStop);
    this->setFlowControl(QSerialPort::NoFlowControl);

    if(this->open(QIODevice::ReadWrite)){
        qDebug()<<"open successfully";
        return true;
    }else
        return false;

}

void SerialWorker::closeSerialPort()
{
    if(this->isOpen())
        this->close();

}

//error message handler
void SerialWorker::handleError(const QByteArray& error)
{
        QByteArray errorMsg;
        QDataStream stream(&errorMsg, QIODevice::WriteOnly);
        stream << QString("ERROR:");
        stream << error;

        sendMessage(errorMsg);
}
//
void SerialWorker::handleFatalError(QSerialPort::SerialPortError error)
{
    Q_UNUSED(error);
    closeSerialPort();
    emit closed();
}

void SerialWorker::handleUploadFile(const QByteArray& data)
{
    if (!m_IsAppendingFile)
    {
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
        m_FileData.append(data);
        if (m_FileData.length() >= m_FileLength)
        {
            saveFile();
            m_IsAppendingFile = m_IsUploadingFile = false;
        }
    }
}

void SerialWorker::saveFile()
{
    QFile file(m_Filename);
    file.open(QIODevice::WriteOnly);
    file.write(m_FileData);
    file.close();
}

void SerialWorker::checkBusJam()
{
    if (this->bytesToWrite() > 0)
    {
        emit busJam();
    }else{
        emit stopTimer();
        emit busIdle();
    }

}

void SerialWorker::sendMessage(const QByteArray &msg)
{
    if(this->bytesToWrite() > 1024*1024){
        emit startTimer(20);
        return ;
    }

    if(this->isOpen()){
        this->write(msg);
        this->flush();
    }else
        return ;

}
