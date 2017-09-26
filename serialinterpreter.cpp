#include "includes.h"
#include <QDebug>
#include <QFile>
#include <signal.h>

SerialInterpreter::SerialInterpreter(QObject* parent):QObject(parent),
    m_IsAppendingFile(false),
    m_commad(NULL),
    m_serialCommunication(NULL),
    m_passiveTool(NULL)

{

}

SerialInterpreter::~SerialInterpreter()
{
    if(m_commad)
        delete m_commad;

    m_passiveTool->deconstructor();
}

int SerialInterpreter::OpenConnection()
{
    if(m_serialCommunication->open(QIODevice::ReadWrite)){
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

int SerialInterpreter::CloseConnection()
{
    if(m_serialCommunication->isOpen())
    {
        m_serialCommunication->close();
    }
#ifdef USE_DEBUG
    qDebug() << "close serial port...";
#endif
    return 0;
}

void SerialInterpreter::SetDeviceName(QString &portName)
{
    m_serialCommunication->setPortName(portName);
}

bool SerialInterpreter::SetBaudRate(char baudRate)
{
    QSerialPort::BaudRate rate;
    switch(baudRate){
    case '1':
        rate = QSerialPort::BaudRate::Baud4800;
        break;
    case '2':
        rate = QSerialPort::BaudRate::Baud19200;
        break;
    case '3':
        rate = QSerialPort::BaudRate::Baud38400;
        break;
    case '4':
        rate = QSerialPort::BaudRate::Baud57600;
        break;
    case '5':
        rate = QSerialPort::BaudRate::Baud115200;
        break;
    default:
        rate = QSerialPort::BaudRate::Baud9600;
        break;
    }
    return  m_serialCommunication->setBaudRate(rate);
}

bool SerialInterpreter::SetDataBits(char dataBits)
{
    QSerialPort::DataBits bits;
    switch(dataBits){
    case '1':
        bits = QSerialPort::DataBits::Data7;
        break;
    default :
        bits = QSerialPort::DataBits::Data8;
        break;
    }
    return m_serialCommunication->setDataBits(bits);
}

bool SerialInterpreter::SetParity(char parity)
{
    QSerialPort::Parity p;
    switch (parity) {
    case '1':
        p = QSerialPort::Parity::OddParity;
        break;
    case '2':
        p = QSerialPort::Parity::EvenParity;
        break;
    default:
        p = QSerialPort::Parity::NoParity;
        break;
    }
    return m_serialCommunication->setParity(p);
}

bool SerialInterpreter::SetStopBits(char stopBits)
{
    QSerialPort::StopBits bits;
    switch(stopBits){
    case '1':
        bits = QSerialPort::StopBits::TwoStop;
        break;
    default :
        bits = QSerialPort::StopBits::OneStop;
        break;
    }
   return m_serialCommunication->setStopBits(bits);
}

bool SerialInterpreter::SetFlowControl(char flowControl)
{
    QSerialPort::FlowControl fc;
    switch (flowControl) {
    case '1':
        fc = QSerialPort::FlowControl::HardwareControl;
        break;
    default:
        fc = QSerialPort::FlowControl::NoFlowControl;
        break;
    }
    return m_serialCommunication->setFlowControl(fc);
}

bool  SerialInterpreter::handleUploadFile(const QByteArray& data)
{
    // PortHandle(2bytes) | StartAddress(4bytes) | FILE_DATA(128bytes)
    m_Filename = data.left(2);
    int sequence;
    sequence = data.mid(2,4).toInt();

    if (m_IsAppendingFile){

        m_FileData += data.mid(4, 128);

    }else{
        if(sequence > 0)
            m_IsAppendingFile = true;

        m_FileData = data.mid(4, 128);
    }

    if(m_FileData.endsWith(QByteArray("0000")))//后补0结尾的说明dataLength<128
    {
        if(!saveFile())
            return false;

        m_IsAppendingFile = false;
    }
    // check crc value
    return true;
}

bool SerialInterpreter::saveFile()
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

void SerialInterpreter::replaySerialBreak(bool signo)
{
    //Reset the system, reset serial communication parameter
    if( signo == true){
#ifdef USE_DEBUG
    qDebug() << "Reset the system";
#endif
        CloseConnection();

        SetBaudRate('0');
        SetDataBits('0');
        SetParity('0');
        SetFlowControl('0');

        OpenConnection();

        replay(REPLAY_RESET);
    }

}
//
//
//
bool SerialInterpreter::replay( const std::string &data)
{
    if(m_serialCommunication == NULL)
        return false;

//REPLAY_PHRQ:
        //<PortHandle><CRC16><CR>
        //  2bytes
//REPLAY_PHSR:
        //<Numbers of PortHandle><nth PortHandle><nth PortHandleStatus><CRC16><CR>
        //            2bytes           2bytes           3bytes     
//REPLAY_3D:
        //<MarkerNumber><LF><ReplayOptionData 5><CRC16><CR>
        //ReplyOptionData 5: <Txn><Tyn><Tzn><LineSeparation><OutOfVolume><LF>
        //Txn, Tyn, Tzn: 9 bytes each(a sign, 8 digits meaning xxxx.xxxx).
        //LineSeparation: 4 bytes(a sign, 3 digits meaning x.xx).
        //OutOfVolume: 1 byte. 0 for inside, and 1 for outside.
//REPLAY_TX:
//        <PortHandleNumber><PortHandle n><ReplyOptionData 0001>…<ReplyOptionData
//        0008><LF><ReplyOptionData 1000><SystemStatus><CRC16><CR>
//        PortHandleNumber: 2 bytes.
//        PortHandle: 2 bytes.
//        ReplyOptionData 0001:
//        <Q0><Qx><Qy><Qz><Tx><Ty><Tz><Error><PortStatus><FrameNumber>
//        Q0, Qx, Qy, Qz: 6 bytes each(a sign, 5 digits meaning x.xxxx)
//        Tx, Ty, Tz: 7 bytes each(a sign, 6 digits meaning xxxx.xx)
//        Error: 6 bytes(a sign, 5 digits meaning x.xxxx).RMS error in mm.
//        PortStatus: 8 bytes.
//        FrameNumber: 8 bytes.
//        Or MISSING<PortStatus><FrameNumber>
//        ReplyOptionData 0002 – 1000 could be null by default option.
//        SystemStatus: 4 bytes.

    QByteArray fullReplay;
    m_crcValue = QByteArray(calcCRC(&data).c_str());
    fullReplay = QByteArray(data.c_str()) + m_crcValue;
    fullReplay.append('\r');

#ifdef USE_DEBUG
    qDebug() << fullReplay;
#endif

    m_serialCommunication->write(fullReplay);
    bool ret = m_serialCommunication->flush();
    return ret;
}
//
//
//
NDIErrorCode SerialInterpreter::cmdInterpreter(const QByteArray &data)
{

    int colonPos = data.indexOf(':');
    int crPos = data.indexOf(LF);
    if(colonPos > 0 && crPos > 0)
    {
        m_commad = data.left(colonPos).data();

        if(strcmp(m_commad, "APIREV") == 0)
        {
            return NDIOKAY;
        }
        if(strcmp(m_commad, "PHINF") == 0)
        {
            return NDIOKAY;
        }
//        if(strcmp(cmd, "PHF") == 0)//Port Handle Free
//        if(strcmp(cmd, "PDIS") == 0)//Port disable
//        if(strcmp(cmd, "IRATE") == 0)//Setting the illuminator rate
//        if(strcmp(cmd, "BEEP") == 0)//Sounding the beeper
        if(strcmp(m_commad, "COMM") == 0)//Change Serial Communication Parameters
        {
            //COMM:<Baudrate><Databits><Parity><Stopbits><HardwareHandShaking><CRC16>
            if(m_serialCommunication == NULL)
                return SERIALINTERFACENOTSET;
            CloseConnection();

            if(SetBaudRate(data.at(colonPos+1))
               && SetDataBits(data.at(colonPos+2))
               && SetParity(data.at(colonPos+3))
               && SetFlowControl(data.at(colonPos+4)) )
            {
                OpenConnection();
                if(replay( REPLAY_OKAY ))
                    return NDIOKAY;
                else
                    return SERIALSENDERROR;
            }else
            {
                if(replay( ERROR_INCORRECT_NUMBER_OF_PARAMETERS ))
                    return NDIOKAY;
                else
                    return SERIALSENDERROR;
            }
        }
        if(strcmp(m_commad, "INIT") == 0)//Initialize the Measurement System
        {
            //INIT:<CRC16><CR>
            //....systemInit()
            if(true){
                if(replay( REPLAY_OKAY ))
                    return NDIOKAY;
                else
                    return SERIALSENDERROR;
            }else{
                if(replay( ERROR_SYSTEM_NOT_INITIALIZED ))//
                    return NDIOKAY;
                else
                    return SERIALSENDERROR;
            }
        }
        if(strcmp(m_commad, "DSTART") == 0)//Start the Diagnostic Mode
        {
            //DSTART:<ReplayOption><CRC16><CR>
            emit startDiagnosing();
            if(replay( REPLAY_OKAY ))
                return NDIOKAY;
            else
                return SERIALSENDERROR;
        }
        if(strcmp(m_commad, "DSTOP") == 0)//Stop the Diagnostic Mode
        {
            emit stopDiagnosing();
            if(replay( REPLAY_OKAY ))
                return NDIOKAY;
            else
                return SERIALSENDERROR;
        }
        if(strcmp(m_commad, "IRINIT") == 0)//Initialize the System to Check for Infrared
        {
            //IRATE:<IlluminatorRate><CRC16><CR>
            //           1 byte:0 for 20HZ, 1 for 30HZ, 2 for 60HZ
            //... do nothing
            if(replay( REPLAY_OKAY ))
                return NDIOKAY;
            else
                return SERIALSENDERROR;
        }
//        char *cmdphsr = "PHSR";
        if(strcmp(m_commad, "PHSR") == 0)//Port Handle Search
        {
            //PHSR:<ReplayOption><CRC16><CR>
            //ReplayOption: 2bytes : 01---报告空闲的端口   02---报告已被占用但是没有初始化的端口
            m_passiveTool = vpsToolManager::getInstance();
            char option = data.at(6);
            if(option == PHSR_REPORT_FREE_HANDLE){
                if(replay( m_passiveTool->getFreePortHandle() ))
                    return NDIOKAY;
                else
                    return SERIALSENDERROR;
            }
            if(option == PHSR_REPORT_OCCUPIED_PORT){
                if(replay( m_passiveTool->getOccupiedPortHandle() ))
                    return NDIOKAY;
                else
                    return SERIALSENDERROR;
            }else{
                if(replay( ERROR_INVALID_COMMAND ))
                    return NDIOKAY;
                else
                    return SERIALSENDERROR;
            }
        }
        if(strcmp(m_commad, "PHRQ") == 0)//Port Handle Request
        {
            //PHRQ:*********1****
            m_passiveTool = vpsToolManager::getInstance();
            if(replay( m_passiveTool->getPortHandle() ))
                return NDIOKAY;
            else
                return SERIALSENDERROR;
        }
        if(strcmp(m_commad, "PVWR") == 0)//Port Virtual Write. Writes an SROM Image data to a tool
        {
            //PVWR:<PortHandle><StartAddress><FileData><CRC16><CR>
            //         2bytes        4bytes     128bytes
            if( handleUploadFile(data.mid(5)))
            {
                if(replay( REPLAY_OKAY ))
                    return NDIOKAY;
                else
                    return SERIALSENDERROR;
            }
            else
            {
                if(replay( ERROR_FAILURE_TO_WRITESROM ))
                    return NDIOKAY;
                else
                    return SERIALSENDERROR;
            }
        }
        if(strcmp(m_commad, "PINIT") == 0)//Port Initialize
        {
            //PINIT:<PortHandle><CRC16><CR>
            std::string ph = data.mid(6, 2).toStdString();
            m_passiveTool = vpsToolManager::getInstance();
            if(m_passiveTool->initPortHandle(ph))
            {
                if(replay( REPLAY_OKAY))
                    return NDIOKAY;
                else
                    return SERIALSENDERROR;
            }else
            {
                if(replay( ERROR_INITIALIZATION_FAILED))
                    return NDIOKAY;
                else
                    return SERIALSENDERROR;
            }
        }
        if(strcmp(m_commad, "PENA") == 0)//Port Enable.enable a port that has been initialized with PINIT
        {
            //PENA:<PortHandle><TrackingPriority><CRC16><CR>
            std::string port;
            port = data.mid(5, 2).toStdString();
            char priority = data.at(7);
            m_passiveTool = vpsToolManager::getInstance();
            if(m_passiveTool->setTrackingPriority(port, priority))
            {
                if(replay( REPLAY_OKAY))
                    return NDIOKAY;
                else
                    return SERIALSENDERROR;
            }else{
                if(replay( ERROR_INVALID_TRACKING_PRIORITY))//ERROR_PORT_NOT_INITIALZED, ERROR_INVALID_PORTHANDLE
                    return NDIOKAY;
                else
                    return SERIALSENDERROR;
            }
        }
        if(strcmp(m_commad, "SFLIST") == 0)//查询trackDevice特征
        {
            //SFLIST:<ReplayOption><CRC16><CR>
            //00: 所有支持的特征值概要
            //01: 活动状态的tool port个数
            //02: wireless tool port个数
            //03: 所有的支持视场的参数
            std::string replay03 =
                    "5-240000-153200-095000+057200+039800+056946+024303+029773+999999+99999924\n";
            if(replay( replay03))
                return NDIOKAY;
            else
                return SERIALSENDERROR;
        }
        if(strcmp(m_commad, "TSTART") == 0)//Start Tracking Mode
        {
            emit startTracking();
            if(replay( REPLAY_OKAY))
                return NDIOKAY;
            else
                return SERIALSENDERROR;

        }
        if(strcmp(m_commad, "TSTOP") == 0)//Stop Tracking Mode
        {
            emit stopTracking();
            if(replay( REPLAY_OKAY))
                return NDIOKAY;
            else
                return SERIALSENDERROR;

        }
        if(strcmp(m_commad, "TX") == 0)//Report transformations in text mode
        {
            //TX:<ReplyOption><CRC16><CR>
            //ReplyOption: Optional. 4 bytes.
            //0001 for transformation data by default,
            //0002 for tool and marker information,
            //0004 for 3D position of a single stray active marker,
            //0008 for 3D positions of markers on tools,
            //0800 for transformations not normally reported, and
            //1000 for 3D positions of stray passive markers.
        }
        if(strcmp(m_commad, "3D") == 0)
        {
            //3D:<PortHandle><ReplayOption><CRC16><CR>
            //        2bytes      1byte: 1-4 for single marker  5 for upto 50 markers
        }
        if(strcmp(m_commad, "VSEL") == 0)//设置测量视场体积
        {
            //VSEL:<VolumeNumber><CRC16><CR>
            if(replay( REPLAY_OKAY))
                return NDIOKAY;
            else
                return SERIALSENDERROR;
        }
        m_commad = NULL;//prevent repeate operation
        replay( ERROR_INVALID_COMMAND);
        return NDIINVALIDCOMMAND;
    }
}

//
//
//
bool SerialInterpreter::setSerialPort(QSerialPort *serial)
{
    if(serial != NULL)
    {
        m_serialCommunication = serial;
        return true;
    }
    else
        return false;
}

//
//
//
const std::string SerialInterpreter::calcCRC(const std::string* input)
{

    if (input == NULL)
        return "";
    /* the crc16 calculation code is taken from the NDI API guide example code section */
    static int oddparity[16] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};
    unsigned int data;  // copy of the input string's current character
    unsigned int crcValue = 0;  // the crc value is stored here
    // the algorithm uses a pointer to crcValue, so it's easier to provide that than to
    //change the algorithm
    unsigned int* puCRC16 = &crcValue;
    for (unsigned int i = 0; i < input->length(); i++)
    {
        data = (*input)[i];
        data = (data ^ (*(puCRC16) & 0xff)) & 0xff;
        *puCRC16 >>= 8;
        if (oddparity[data & 0x0f] ^ oddparity[data >> 4])
        {
            *(puCRC16) ^= 0xc001;
        }
        data <<= 6;
        *puCRC16 ^= data;
        data <<= 1;
        *puCRC16 ^= data;
    }
    // crcValue contains now the CRC16 value. Convert it to a string and return it
    char returnvalue[13];
    sprintf(returnvalue,"%04X", crcValue);  // 4 hexadecimal digit with uppercase format
    return std::string(returnvalue);
}
