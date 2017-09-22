#include "includes.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>

SerialWorker::SerialWorker(QObject *parent):
    QSerialPort(parent),
    m_timer(new QTimer(this)),
    m_IsUploadingFile(false),
    m_IsAppendingFile(false),
    m_interpreter(new SerialInterpreter)

{
    m_interpreter->setSerialPort(this);

    connect(this, SIGNAL(startTimer(int)), m_timer, SLOT(start(int)));
    connect(this, SIGNAL(stopTimer()), m_timer, SLOT(stop()));
    connect(m_timer, SIGNAL(timeout()), this, SLOT(checkBusJam()));

    connect(this, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(this, SIGNAL(error(QSerialPort::SerialPortError)),
            this, SLOT(handleFatalError(QSerialPort::SerialPortError)));

    connect(m_interpreter, SIGNAL(startTracking()), this, SIGNAL(startTracking()));
    connect(m_interpreter, SIGNAL(stopTracking()), this, SIGNAL(stopTracking()));
}

SerialWorker::~SerialWorker()
{
    closeSerialPort();
}

int SerialWorker::OpenConnection()
{
    if(this->open(QIODevice::ReadWrite)){
#ifdef USE_DEBUG
        qDebug()<<"open serial port successfully...";
#endif
        return 0;
    }else{
#ifdef USE_DEBUG
        qDebug()<<"open serial port fail...";
#endif
        return -1;
    }
}

int SerialWorker::CloseConnection()
{
    this->close();
    return 0;
}

void SerialWorker::SetDeviceName(QString &portName)
{
    this->setPortName(portName);
}

bool SerialWorker::SetBaudRate(char baudRate)
{
    BaudRate rate;
    switch(baudRate){
    case '1':
        rate = BaudRate::Baud4800;
        break;
    case '2':
        rate = BaudRate::Baud19200;
        break;
    case '3':
        rate = BaudRate::Baud38400;
        break;
    case '4':
        rate = BaudRate::Baud57600;
        break;
    case '5':
        rate = BaudRate::Baud115200;
        break;
    default:
        rate = BaudRate::Baud9600;
        break;
    }
    return  this->setBaudRate(rate);
}

bool SerialWorker::SetDataBits(char dataBits)
{
    DataBits bits;
    switch(dataBits){
    case '1':
        bits = DataBits::Data7;
        break;
    default :
        bits = DataBits::Data8;
        break;
    }
    return this->setDataBits(bits);
}

bool SerialWorker::SetParity(char parity)
{
    Parity p;
    switch (parity) {
    case '1':
        p = Parity::OddParity;
        break;
    case '2':
        p = Parity::EvenParity;
        break;
    default:
        p = Parity::NoParity;
        break;
    }
    return this->setParity(p);
}

bool SerialWorker::SetStopBits(char stopBits)
{
    StopBits bits;
    switch(stopBits){
    case '1':
        bits = StopBits::TwoStop;
        break;
    default :
        bits = StopBits::OneStop;
        break;
    }
   return this->setStopBits(bits);
}

bool SerialWorker::SetFlowControl(char flowControl)
{
    FlowControl fc;
    switch (flowControl) {
    case '1':
        fc = FlowControl::HardwareControl;
        break;
    default:
        fc = FlowControl::NoFlowControl;
        break;
    }
    return this->setFlowControl(fc);
}


void SerialWorker::readData()
{
    if (this->bytesAvailable())
    {
        QByteArray data = this->readAll();
#ifdef USE_DEBUG
        qDebug() << "read data from serialport...";
        qDebug() << data;
#endif
        m_interpreter->cmdInterpreter(data);

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

}

bool SerialWorker::setUpDefault()
{
    this->setPortName("ttyS0");
    this->setBaudRate(QSerialPort::Baud9600);
    this->setDataBits(QSerialPort::Data8);
    this->setParity(QSerialPort::NoParity);
    this->setStopBits(QSerialPort::OneStop);
    this->setFlowControl(QSerialPort::NoFlowControl);

    return true;

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

bool  SerialWorker::handleUploadFile(const QByteArray& data)
{
    // PortHandle(2bytes) | StartAddress(4bytes) | FILE_DATA(128bytes)
    m_Filename = data.left(2);
    int sequence;
    sequence = data.mid(2,4).toInt();

    if (m_IsAppendingFile){

        m_FileData += data.mid(4, 128);

        if (m_FileData.length() >= m_FileLength){
            if( !saveFile() )
                return false;
        }
    }else{
        if(sequence > 0)
            m_IsAppendingFile = true;

        m_FileData = data.mid(4, 128);
        if (m_FileData.length() >= m_FileLength)
        {
            if(!saveFile())
                return false;

            m_IsAppendingFile = false;
        }
        if(m_FileData.endsWith(QByteArray("0000")))//后补0结尾的说明dataLength<128
        {
            if(!saveFile())
                return false;

            m_IsAppendingFile = false;
        }
    }
    // check crc value

}

bool SerialWorker::saveFile()
{
    QFile file(m_Filename);
    if(file.open(QIODevice::WriteOnly) == true){
        qint64 counts = file.write(m_FileData);
        if(counts < 0)
            return false;
        file.close();
    }else
        return false;
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

bool SerialWorker::sendMessage(const QByteArray &msg)
{
    if(this->bytesToWrite() > 1024*1024){
        emit startTimer(20);
        return false;
    }

    if(this->isOpen()){
        this->write(msg);
        this->flush();
        return true;
    }else
        return false;

}
