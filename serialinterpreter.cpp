#include "includes.h"
#include <QDebug>


SerialInterpreter::SerialInterpreter(QObject* parent):QObject(parent),
    m_serialCommunication(NULL),
    m_passiveTool(NULL),
    m_commad("")
{

}

SerialInterpreter::~SerialInterpreter()
{

}

//
//
//
bool SerialInterpreter::replay(int replayType, const std::string &data)
{
    if(m_serialCommunication == NULL)
        return false;

    QByteArray fullReplay;

    switch(replayType){
    case REPLAY_BREAK:
        m_crcValue = QByteArray(calcCRC(REPLAY_RESET).c_str());
        fullReplay = QByteArray(REPLAY_RESET) + m_crcValue;
        break;
    case REPLAY_OPERATION:
        m_crcValue = QByteArray(calcCRC(&data).c_str());
        fullReplay = QByteArray(data.c_str()) + m_crcValue;
        break;
    case REPLAY_3D:
        //<MarkerNumber><LF><ReplayOptionData 5><CRC16><CR>
        //ReplyOptionData 5: <Txn><Tyn><Tzn><LineSeparation><OutOfVolume><LF>
        //Txn, Tyn, Tzn: 9 bytes each(a sign, 8 digits meaning xxxx.xxxx).
        //LineSeparation: 4 bytes(a sign, 3 digits meaning x.xx).
        //OutOfVolume: 1 byte. 0 for inside, and 1 for outside.
        break;
    case REPLAY_PHRQ:
        //<PortHandle><CRC16><CR>
        //  2bytes
        m_crcValue = QByteArray(calcCRC(&data).c_str());
        fullReplay = QByteArray(data.c_str()) + m_crcValue;
        break;
    case REPLAY_PHSR:
        //<Numbers of PortHandle><nth PortHandle><nth PortHandleStatus><CRC16><CR>
        //            2bytes           2bytes           3bytes

        break;
    case REPLAY_SFLIST:
        m_crcValue = QByteArray(calcCRC(&data).c_str());
        fullReplay = QByteArray(data.c_str()) + m_crcValue;
        break;
    case REPLAY_TX:
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
        break;
    case REPLAY_ERROR:
        m_crcValue = QByteArray(calcCRC(&data).c_str());
        fullReplay = QByteArray(data.c_str()) + m_crcValue;
        break;
    }
    fullReplay.append('\r');

#ifdef USE_DEBUG
    qDebug() << fullReplay;
#endif

    bool ret = m_serialCommunication->sendMessage(fullReplay);
    return ret;
}

//
//
//
NDIErrorCode SerialInterpreter::cmdInterpreter(const QByteArray &data)
{
    if(data.at(0) == '0' ){  //Reset the system, reset serial communication parameter
        replay(REPLAY_BREAK);
        m_serialCommunication->setUpDefault();
        return NDIOKAY;
    }

    int colonPos = data.indexOf(':');
    int crPos = data.indexOf(CR);
    if(colonPos > 0 && crPos > 0){
        m_commad = data.left(colonPos - 1).toStdString();

        switch(m_commad){
        case COMMAD_COMM: //set serial communication parameter
            //COMM:<Baudrate><Databits><Parity><Stopbits><HardwareHandShaking><CRC16>
            if(m_serialCommunication == NULL)
                return SERIALINTERFACENOTSET;
            if(m_serialCommunication->SetBaudRate(data.at(colonPos+1))
               && m_serialCommunication->SetDataBits(data.at(colonPos+2))
               && m_serialCommunication->SetParity(data.at(colonPos+3))
               && m_serialCommunication->SetFlowControl(data.at(colonPos+4)) )
            {
                replay(REPLAY_OPERATION, REPLAY_OKAY);
            }else
            {
                m_serialMutex->unlock();
                replay(REPLAY_OPERATION, ERROR_INCORRECT_NUMBER_OF_PARAMETERS);
            }
            break;
        case COMMAD_INIT: //initialize serial port, beep, led
            //INIT:<CRC16><CR>
            //....systemInit()
            if(true){
                replay(REPLAY_OPERATION, REPLAY_OKAY);
            }else{
                replay(REPLAY_OPERATION, ERROR_SYSTEM_NOT_INITIALIZED);//
            }
            break;
        case COMMAD_APIREV:
            break;
        case COMMAD_PHRQ:// Port Handle Request. Will request a Port Handle for a wireless tool and return it
            //PHRQ:*********1****
            m_passiveTool = vpsToolManager::getInstance();
            replay(REPLAY_PHRQ, m_passiveTool->getPortHandle());
            break;
        case COMMAD_3D: //Report 3D Positions of single markers
            //3D:<PortHandle><ReplayOption><CRC16><CR>
            //        2bytes      1byte: 1-4 for single marker  5 for upto 50 markers

            break;
        case COMMAD_BEEP:
            break;
        case COMMAD_BX:
            break;
        case COMMAD_DSTART://开启诊断模式  弃用
            //DSTART:<ReplayOption><CRC16><CR>
            //...do nothing
            replay(REPLAY_OPERATION, REPLAY_OKAY);
            emit startDiagnosing();
            break;
        case COMMAD_DSTOP://
            //...DO NOTHING
            replay(REPLAY_OPERATION, REPLAY_OKAY);
            emit stopDiagnosing();
            break;
        case COMMAD_IRATE://设置红外灯板闪烁频率. 弃用
            //IRATE:<IlluminatorRate><CRC16><CR>
            //           1 byte:0 for 20HZ, 1 for 30HZ, 2 for 60HZ
            //... do nothing
            replay(REPLAY_OPERATION, REPLAY_OKAY);
            break;
        case COMMAD_IRINIT:  // Initialize the System to Check for Infrared
            break;
        case COMMAD_PDIS:
            break;
        case COMMAD_PENA://enable the reporting of transformation for a particular porthandle
            //PENA:<PortHandle><TrackingPriority><CRC16><CR>
            std::string port = data.mid((5, 2)).toStdString();
            char priority = data.at(7);
            if(m_passiveTool->setTrackingPriority(port, priority))
            {
                replay(REPLAY_OPERATION, REPLAY_OKAY);
            }else{
                replay(REPLAY_OPERATION, ERROR_INVALID_TRACKING_PRIORITY);//ERROR_PORT_NOT_INITIALZED, ERROR_INVALID_PORTHANDLE
            }
            break;
        case COMMAD_PHF:
            break;
        case COMMAD_PHINF:
            break;
        case COMMAD_PHSR: // Port Handle Search. Will returned port handles
            //PHSR:<ReplayOption><CRC16><CR>
            //ReplayOption: 2bytes : 01---报告空闲的端口   02---报告已被占用但是没有初始化的端口
            m_passiveTool = vpsToolManager::getInstance();
            char option = data.at(6);
            if(option == PHSR_REPORT_FREE_HANDLE){
                replay(REPLAY_PHSR, m_passiveTool->getFreePortHandle());
            }
            if(option == PHSR_REPORT_OCCUPIED_PORT){
                replay( REPLAY_PHSR, m_passiveTool->getOccupiedPortHandle() );
            }else{
                replay( REPLAY_ERROR, ERROR_INVALID_COMMAND );
            }
            break;
        case COMMAD_PINIT://Initialize a port handle
            //PINIT:<PortHandle><CRC16><CR>
            if(m_passiveTool->initPortHandle(data.mid(6, 2).toStdString()))
            {
                replay(REPLAY_OPERATION, REPLAY_OKAY);
            }else
            {
                replay(REPLAY_OPERATION, ERROR_INITIALIZATION_FAILED);
            }
            break;
        case COMMAD_PVWR://Assign a definition file to a tool
            //PVWR:<PortHandle><StartAddress><FileData><CRC16><CR>
            //         2bytes        4bytes     128bytes
            if( m_serialCommunication->handleUploadFile(data.mid(5)))
            {
                replay(REPLAY_OPERATION, REPLAY_OKAY);
            }
            else
            {
                replay(REPLAY_OPERATION, ERROR_FAILURE_TO_WRITESROM);
            }

            break;
        case COMMAD_SFLIST://查询trackDevice特征
            //SFLIST:<ReplayOption><CRC16><CR>
            //00: 所有支持的特征值概要
            //01: 活动状态的tool port个数
            //02: wireless tool port个数
            //03: 所有的支持视场的参数
            std::string replay03 = DeviceDataPolarisSpectra.HardwareCode + std::string(LF);
            replay(REPLAY_SFLIST, replay03);
            break;
        case COMMAD_TSTART://开始跟踪
            //...
            replay(REPLAY_OPERATION, REPLAY_OKAY);
            emit startTracking();
            break;
        case COMMAD_TSTOP://停止跟踪
            //...
            replay(REPLAY_OPERATION, REPLAY_OKAY);
            emit stopTracking();
            break;
        case COMMAD_TX:
//TX:<ReplyOption><CRC16><CR>
//ReplyOption: Optional. 4 bytes.
//0001 for transformation data by default,
//0002 for tool and marker information,
//0004 for 3D position of a single stray active marker,
//0008 for 3D positions of markers on tools,
//0800 for transformations not normally reported, and
//1000 for 3D positions of stray passive markers.
            break;
        case COMMAD_VSEL://设置测量视场体积
            //VSEL:<VolumeNumber><CRC16><CR>
            replay(REPLAY_OPERATION, REPLAY_OKAY);
            break;
        default:
            replay(REPLAY_ERROR, ERROR_INVALID_COMMAND); //01 represent for invalid command
            return NDIINVALIDCOMMAND;
        }
    }else{
        return NDICOMMANDTOOSHORT;
    }

}

//
//
//
bool SerialInterpreter::setSerialPort(SerialWorker *serial)
{
    QMutexLocker lock(m_serialMutex);
    m_serialCommunication = serial;
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
