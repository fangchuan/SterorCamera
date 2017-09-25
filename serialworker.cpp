#include "includes.h"
#include <signal.h>
#include <QDebug>

SerialWorker::SerialWorker(QObject *parent):
    QObject(parent),
    m_timer(new QTimer(this)),
    m_interpreter(new SerialInterpreter(this)),
    m_serialPort(new QSerialPort(this))

{
    m_interpreter->setSerialPort(m_serialPort);

    connect(this, SIGNAL(startTimer(int)), m_timer, SLOT(start(int)));
    connect(this, SIGNAL(stopTimer()), m_timer, SLOT(stop()));
    connect(m_timer, SIGNAL(timeout()), this, SLOT(checkBusJam()));

    connect(m_serialPort, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(m_serialPort, SIGNAL(error(QSerialPort::SerialPortError)),
            this, SLOT(handleFatalError(QSerialPort::SerialPortError)));

    connect(m_interpreter, SIGNAL(startTracking()), this, SIGNAL(startTracking()));
    connect(m_interpreter, SIGNAL(stopTracking()), this, SIGNAL(stopTracking()));

    if (signal(SIGINT, m_interpreter->replaySerialBreak) == SIG_ERR)
        qDebug() << "cant catch SIGINT";
}

SerialWorker::~SerialWorker()
{
    if(m_serialPort->isOpen())
        m_serialPort->close();
}

void SerialWorker::readData()
{

    if( m_serialPort->canReadLine() ){
        QByteArray data = m_serialPort->readLine();
#ifdef USE_DEBUG
        qDebug() << "read data from serialport...";
        qDebug() << data;
#endif
        m_interpreter->cmdInterpreter(data);
    }

//        switch(TrackerMessageTranslator::translate(data))
//        {
//        case Tracker::START_TRACKING:
//            {
//                emit startTracking();
//            }
//            break;
//        case Tracker::STOP_TRACKING:
//            {
//                emit stopTracking();
//            }
//            break;
//        case Tracker::START_MANUFACTURING:{
//                emit startManufacturing();
//            }
//            break;
//        case Tracker::MANUFACTURE:
//            {
//                emit manufacting();
//        }
//            break;
//        case Tracker::STOP_MANUFACTURING:
//            {
//                emit stopManufacturing();
//            }
//            break;
//        case Tracker::START_DIAGNOSING:
//            {
//                emit startDiagnosing();
//            }
//            break;
//        case Tracker::STOP_DIAGNOSING:
//            {
//                emit stopDiagnosing();
//            }
//            break;
//        }


}

//
//Open Serial port in default parameter
//
bool SerialWorker::setUpDefault()
{
    m_serialPort->setPortName("ttyUSB0");
    m_serialPort->setBaudRate(QSerialPort::Baud9600);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if(m_serialPort->open(QIODevice::ReadWrite))
    {
        qDebug() << "Open Serial port 0 success...";
        return true;
    }
    else
    {
        qDebug() << "open Serial port 0 failed...";
        return false;
    }

}

//error message handler
void SerialWorker::handleError(const QByteArray& error)
{
    if(m_serialPort->bytesToWrite() > 1024*1024){
        emit startTimer(20);
    }

    if(m_serialPort->isOpen())
        m_interpreter->replay(error.toStdString());
}
//
void SerialWorker::handleFatalError(QSerialPort::SerialPortError error)
{
    Q_UNUSED(error);
    if(m_serialPort->isOpen()){
        m_serialPort->close();
        emit serialWorkerStop();
    }
}


void SerialWorker::checkBusJam()
{
    if (m_serialPort->bytesToWrite() > 0)
    {
        emit busJam();
    }else{
        emit stopTimer();
        emit busIdle();
    }

}

bool SerialWorker::sendPosData(const QByteArray &msg)
{
    if(m_serialPort->bytesToWrite() > 1024*1024){
        emit startTimer(20);
        return false;
    }

    if(m_serialPort->isOpen()){
        m_interpreter->replay(msg.toStdString());
        return true;
    }else
        return false;

}
