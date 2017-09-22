#include "includes.h"
#include <QDebug>


SerialInterpreter::SerialInterpreter(QObject* parent):QObject(parent),
    m_commad(NULL),
    m_serialCommunication(NULL),
    m_passiveTool(NULL)

{

}

SerialInterpreter::~SerialInterpreter()
{

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

    bool ret = m_serialCommunication->sendMessage(fullReplay);
    return ret;
}

//
//
//
NDIErrorCode SerialInterpreter::cmdInterpreter(const QByteArray &data)
{
    if(data.at(0) == '0' ){  //Reset the system, reset serial communication parameter
        replay(REPLAY_RESET);
        m_serialCommunication->setUpDefault();
        return NDIOKAY;
    }

    int colonPos = data.indexOf(':');
    int crPos = data.indexOf(CR);
    if(colonPos > 0 && crPos > 0)
    {
        m_commad = data.left(colonPos - 1).data();

        if(strcmp(m_commad, "APIREV:") == 0)
        {
            return NDIOKAY;
        }
        if(strcmp(m_commad, "PHINF:") == 0)
        {
            return NDIOKAY;
        }
//        if(strcmp(cmd, "PHF:") == 0)//Port Handle Free
//        if(strcmp(cmd, "PDIS:") == 0)//Port disable
//        if(strcmp(cmd, "IRATE:") == 0)//Setting the illuminator rate
//        if(strcmp(cmd, "BEEP:") == 0)//Sounding the beeper
        if(strcmp(m_commad, "COMM:") == 0)//Change Serial Communication Parameters
        {
            //COMM:<Baudrate><Databits><Parity><Stopbits><HardwareHandShaking><CRC16>
            if(m_serialCommunication == NULL)
                return SERIALINTERFACENOTSET;
            if(m_serialCommunication->SetBaudRate(data.at(colonPos+1))
               && m_serialCommunication->SetDataBits(data.at(colonPos+2))
               && m_serialCommunication->SetParity(data.at(colonPos+3))
               && m_serialCommunication->SetFlowControl(data.at(colonPos+4)) )
            {
                m_serialCommunication->OpenConnection();
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
        if(strcmp(m_commad, "INIT:") == 0)//Initialize the Measurement System
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
        if(strcmp(m_commad, "DSTART:") == 0)//Start the Diagnostic Mode
        {
            //DSTART:<ReplayOption><CRC16><CR>
            emit startDiagnosing();
            if(replay( REPLAY_OKAY ))
                return NDIOKAY;
            else
                return SERIALSENDERROR;
        }
        if(strcmp(m_commad, "DSTOP:") == 0)//Stop the Diagnostic Mode
        {
            emit stopDiagnosing();
            if(replay( REPLAY_OKAY ))
                return NDIOKAY;
            else
                return SERIALSENDERROR;
        }
        if(strcmp(m_commad, "IRINIT:") == 0)//Initialize the System to Check for Infrared
        {
            //IRATE:<IlluminatorRate><CRC16><CR>
            //           1 byte:0 for 20HZ, 1 for 30HZ, 2 for 60HZ
            //... do nothing
            if(replay( REPLAY_OKAY ))
                return NDIOKAY;
            else
                return SERIALSENDERROR;
        }
        if(strcmp(m_commad, "PHSR:") == 0)//Port Handle Search
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
        if(strcmp(m_commad, "PHRQ:") == 0)//Port Handle Request
        {
            //PHRQ:*********1****
            m_passiveTool = vpsToolManager::getInstance();
            if(replay( m_passiveTool->getPortHandle() ))
                return NDIOKAY;
            else
                return SERIALSENDERROR;
        }
        if(strcmp(m_commad, "PVWR:") == 0)//Port Virtual Write. Writes an SROM Image data to a tool
        {
            //PVWR:<PortHandle><StartAddress><FileData><CRC16><CR>
            //         2bytes        4bytes     128bytes
            if( m_serialCommunication->handleUploadFile(data.mid(5)))
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
        if(strcmp(m_commad, "PINIT:") == 0)//Port Initialize
        {
            //PINIT:<PortHandle><CRC16><CR>
            std::string ph = data.mid(6, 2).toStdString();
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
        if(strcmp(m_commad, "PENA:") == 0)//Port Enable.enable a port that has been initialized with PINIT
        {
            //PENA:<PortHandle><TrackingPriority><CRC16><CR>
            std::string port( data.mid((5, 2)).toStdString());
            char priority = data.at(7);
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
        if(strcmp(m_commad, "SFLIST:") == 0)//查询trackDevice特征
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
        if(strcmp(m_commad, "TSTART:") == 0)//Start Tracking Mode
        {
            emit startTracking();
            if(replay( REPLAY_OKAY))
                return NDIOKAY;
            else
                return SERIALSENDERROR;

        }
        if(strcmp(m_commad, "TSTOP:") == 0)//Stop Tracking Mode
        {
            emit stopTracking();
            if(replay( REPLAY_OKAY))
                return NDIOKAY;
            else
                return SERIALSENDERROR;

        }
        if(strcmp(m_commad, "TX:") == 0)//Report transformations in text mode
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
        if(strcmp(m_commad, "3D:") == 0)
        {
            //3D:<PortHandle><ReplayOption><CRC16><CR>
            //        2bytes      1byte: 1-4 for single marker  5 for upto 50 markers
        }
        if(strcmp(m_commad, "VSEL:") == 0)//设置测量视场体积
        {
            //VSEL:<VolumeNumber><CRC16><CR>
            if(replay( REPLAY_OKAY))
                return NDIOKAY;
            else
                return SERIALSENDERROR;
        }

        replay( ERROR_INVALID_COMMAND); //01 represent for invalid command
        return NDIINVALIDCOMMAND;
    }
}

//
//
//
bool SerialInterpreter::setSerialPort(SerialWorker *serial)
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
