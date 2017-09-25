#ifndef SERIALINTERPRETER_H
#define SERIALINTERPRETER_H

#include <QObject>
#include <QSerialPort>
#include "trackingtypes.h"
#include "vpstoolmanager.h"
#include <string>
#include <QMutex>



#define LF 0x0A
#define CR 0x0D

#define REPLAY_OKAY    std::string("OKAY")
#define REPLAY_RESET   std::string("RESET")

#define ERROR_INVALID_COMMAND  std::string("ERROR01")
#define ERROR_COMMAND_TOOLONG  std::string("ERROR02")
#define ERROR_COMMAND_TOOSHORT std::string("ERROR03")
#define ERROR_CRCDOES_NOTMATCH std::string("ERROR04")
#define ERROR_TIMEOUT          std::string("ERROR05")
#define ERROR_UNABLE_TO_SET_NEW_COMM_PARAMETERS std::string("ERROR06")
#define ERROR_INCORRECT_NUMBER_OF_PARAMETERS  std::string("ERROR07")
#define ERROR_INVALID_PORTHANDLE         std::string("ERROR08")
#define ERROR_INVALID_TRACKING_PRIORITY  std::string("ERROR09")
#define ERROR_INVALID_LED                std::string("ERROR0A")
#define ERROR_INVALID_LED_STATE          std::string("ERROR0B")
#define ERROR_COMMAND_INVALID_IN_CURRENTMODE std::string("ERROR0C")
#define ERROR_NO_TOOL_FOR_PORT     std::string("ERROR0D")
#define ERROR_PORT_NOT_INITIALIZED std::string("ERROR0E")
#define ERROR_SYSTEM_NOT_INITIALIZED std::string("ERROR10")
#define ERROR_UNABLE_TO_STOP_TRACKING std::string("ERROR11")
#define ERROR_UNABLE_TO_START_TRACKING std::string("ERROR12")
#define ERROR_INITIALIZATION_FAILED     std::string("ERROR13")
#define ERROR_INVALID_VOLUME_PARAMETERS std::string("ERROR14")
#define ERROR_CANT_START_DIAGNOSTIC_MODE std::string("ERROR16")
#define ERROR_CANT_INIT_IR_DIAGNOSTICS   std::string("ERROR1B")
#define ERROR_FAILURE_TO_WRITESROM       std::string("ERROR1F")
#define ERROR_ENABLED_TOOLS_NOT_SUPPORTED  std::string("ERROR22")
#define ERROR_COMMAND_PARAMETER_OUTOFRANGE  std::string("ERROR23")
#define ERROR_INO_MEMORY_AVAILABLE  std::string("ERROR2A")
#define ERROR_PORT_HANDLE_NOTALLOCATED  std::string("ERROR2B")
#define ERROR_PORT_HAS_BECOME_UNOCCUPIED  std::string("ERROR2C")
#define ERROR_OUT_OF_HANDLES  std::string("ERROR2D")
#define ERROR_INCOMPATIBLE_FIRMWARE_VERSIONS  std::string("ERROR2E")
#define ERROR_INVALID_PORT_DESCRIPTION  std::string("ERROR2F")
#define ERROR_INVALID_OPERATION_FORDEVICE  std::string("ERROR32")
 // ...
#define PHSR_REPORT_FREE_HANDLE '1'
#define PHSR_REPORT_OCCUPIED_PORT '2'




class SerialInterpreter: public QObject
{
    Q_OBJECT
public:
    SerialInterpreter(QObject* parent = NULL);
    ~SerialInterpreter();

    bool setSerialPort(QSerialPort *serial);
    void replaySerialBreak(int signo);
    const std::string calcCRC(const std::string* input);
    bool replay(const std::__cxx11::string &data );
    NDIErrorCode cmdInterpreter(const QByteArray &data);
private:
    int OpenConnection();
    int CloseConnection();
    void SetDeviceName(QString &portName);
    bool SetBaudRate(char baudRate);
    bool SetDataBits(char dataBits);
    bool SetParity(char parity);
    bool SetStopBits(char stopBits);
    bool SetFlowControl(char flowControl);
    bool handleUploadFile(const QByteArray& data);
    bool saveFile();

signals:
    void startTracking();
    void stopTracking();
    void startDiagnosing();
    void stopDiagnosing();
private:
    bool m_IsAppendingFile;
    QString m_Filename;
    QByteArray m_FileData;
    QByteArray m_crcValue;
    char *m_commad;
    QSerialPort *m_serialCommunication;
    vpsToolManager *m_passiveTool;

};

#endif // SERIALINTERPRETER_H
