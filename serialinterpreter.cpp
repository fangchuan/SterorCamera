#include "serialinterpreter.h"

SerialInterpreter::SerialInterpreter():
    m_serialCommunication(NULL),
    m_commad(""),
    m_serialMutex( new QMutex)
{

}

bool SerialInterpreter::replay(int replayType, const std::string &data)
{
    QByteArray fullReplay;


    switch(replayType){
    case REPLAY_BREAK:
        std::string tmp = "RESET";
        m_crcValue = QByteArray(calcCRC(&tmp).c_str());
        fullReplay = QByteArray(tmp.c_str()) + m_crcValue;
        break;
    case REPLAY_OPERATION:
        m_crcValue = QByteArray(calcCRC(&data).c_str());
        fullReplay = QByteArray(data.c_str()) + m_crcValue;
        break;
    case REPLAY_3D:

        break;
    case REPLAY_PHRQ:
        break;
    case REPLAY_PHSR:
        break;
    case REPLAY_SFLIST:
        break;
    case REPLAY_TX:
        break;
    case REPLAY_ERROR:
        break;
    }
    fullReplay.append('\r');
    m_serialMutex->lock();
    bool ret = m_serialCommunication->sendMessage(fullReplay);
    m_serialMutex->unlock();
    return ret;
}

NDIErrorCode SerialInterpreter::cmdInterpreter(const QByteArray &data)
{
    int length = data.length();

    if(data.at(0) == '0' ){
        replay(REPLAY_BREAK);
        m_serialMutex->lock();
        m_serialCommunication->setUpDefault();
        m_serialMutex->unlock();
        return NDIOKAY;
    }

    int colonPos = data.indexOf(':');
    int crPos = data.indexOf('\r');
    if(colonPos > 0 && crPos > 0){
        QByteArray cmd = data.left(colonPos - 1);

        switch(cmd){
        case COMMAD_COMM: //set serial port parameter
            m_serialMutex->lock();
            bool retB = m_serialCommunication->SetBaudRate(data.at(colonPos+1));
            bool retD = m_serialCommunication->SetDataBits(data.at(colonPos+2));
            bool retP = m_serialCommunication->SetParity(data.at(colonPos+3));
            bool retF = m_serialCommunication->SetFlowControl(data.at(colonPos+4));
            m_serialMutex->unlock();
            if(retB && retD && retP && retF){
                replay(REPLAY_OPERATION, std::string("OKAY"));
            }else{
                replay(REPLAY_OPERATION, std::string("ERROR07")); //07 represent for incorrect number param

            }
            break;
        case COMMAD_INIT: //initialize serial port, beep, led

            replay(REPLAY_OPERATION, std::string("OKAY"));
            replay(REPLAY_OPERATION, std::string("ERROR10"));//10 represent for system not initialized
            break;
        case COMMAD_APIREV:
            break;
        case COMMAD_PHRQ:
            break;
        case COMMAD_3D:
            break;
        case COMMAD_BEEP:
            break;
        case COMMAD_BX:
            break;
        case COMMAD_DSTART:
            break;
        case COMMAD_DSTOP:
            break;
        case COMMAD_IRATE:
            break;
        case COMMAD_IRINIT:
            break;
        case COMMAD_PDIS:
            break;
        case COMMAD_PENA:
            break;
        case COMMAD_PHF:
            break;
        case COMMAD_PHINF:
            break;
        case COMMAD_PHSR:
            break;
        case COMMAD_PINIT:
            break;
        case COMMAD_PVWR:
            break;
        case COMMAD_SFLIST:
            break;
        case COMMAD_TSTART:
            break;
        case COMMAD_TSTOP:
            break;
        case COMMAD_TX:
            break;
        case COMMAD_VSEL:
            break;
        default:
            replay(REPLAY_ERROR, std::string("ERROR01")); //01 represent for invalid command
            return NDIINVALIDCOMMAND;
        }
    }else{
        return NDICOMMANDTOOSHORT;
    }

}

bool SerialInterpreter::setSerialPort(SerialWorker *serial)
{
    QMutexLocker lock(m_serialMutex);
    m_serialCommunication = serial;
}

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
