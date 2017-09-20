#ifndef SERIALINTERPRETER_H
#define SERIALINTERPRETER_H

#include <QObject>
#include "serialworker.h"
#include "trackingtypes.h"
#include "vpstoolmanager.h"
#include <QMutex>


class SerialWorker;



class SerialInterpreter: public QObject
{

#define LF 0x0A
#define CR 0x0D

#define REPLAY_OKAY    "OKAY"
#define REPLAY_RESET  "RESET"


#define ERROR_INVALID_COMMAND  "ERROR01"
#define ERROR_COMMAND_TOOLONG  "ERROR02"
#define ERROR_COMMAND_TOOSHORT "ERROR03"
#define ERROR_CRCDOES_NOTMATCH "ERROR04"
#define ERROR_TIMEOUT         "ERROR05"
#define ERROR_UNABLE_TO_SET_NEW_COMM_PARAMETERS "ERROR06"
#define ERROR_INCORRECT_NUMBER_OF_PARAMETERS  "ERROR07"
#define ERROR_INVALID_PORTHANDLE   "ERROR08"
#define ERROR_INVALID_TRACKING_PRIORITY  "ERROR09"
#define ERROR_INVALID_LED   "ERROR0A"
#define ERROR_INVALID_LED_STATE  "ERROR0B"
#define ERROR_COMMAND_INVALID_IN_CURRENTMODE "ERROR0C"
#define ERROR_NO_TOOL_FOR_PORT "ERROR0D"
#define ERROR_PORT_NOT_INITIALIZED "ERROR0E"
#define ERROR_SYSTEM_NOT_INITIALIZED "ERROR10"
#define ERROR_UNABLE_TO_STOP_TRACKING "ERROR11"
#define ERROR_UNABLE_TO_START_TRACKING "ERROR12"
#define ERROR_INITIALIZATION_FAILED "ERROR13"
#define ERROR_INVALID_VOLUME_PARAMETERS "ERROR14"
#define ERROR_CANT_START_DIAGNOSTIC_MODE "ERROR16"
#define ERROR_CANT_INIT_IR_DIAGNOSTICS "ERROR1B"
#define ERROR_FAILURE_TO_WRITESROM  "ERROR1F"
#define ERROR_ENABLED_TOOLS_NOT_SUPPORTED  "ERROR22"
#define ERROR_COMMAND_PARAMETER_OUTOFRANGE  "ERROR23"
#define ERROR_INO_MEMORY_AVAILABLE  "ERROR2A"
#define ERROR_PORT_HANDLE_NOTALLOCATED  "ERROR2B"
#define ERROR_PORT_HAS_BECOME_UNOCCUPIED  "ERROR2C"
#define ERROR_OUT_OF_HANDLES  "ERROR2D"
#define ERROR_INCOMPATIBLE_FIRMWARE_VERSIONS  "ERROR2E"
#define ERROR_INVALID_PORT_DESCRIPTION  "ERROR2F"
#define ERROR_INVALID_OPERATION_FORDEVICE  "ERROR32"
 // ...

#define PHSR_REPORT_FREE_HANDLE '1'
#define PHSR_REPORT_OCCUPIED_PORT '2'


#define    COMMAD_APIREV  "APIREV:"
#define    COMMAD_PHINF   "PHINF:"
#define    COMMAD_COMM   "COMM:"    //Change Serial Communication Parameters
#define    COMMAD_INIT   "INIT:"   //Initialize the Measurement System
#define    COMMAD_DSTART   "DSTART:" //Start the Diagnostic Mode
#define    COMMAD_DSTOP   "DSTOP:"   //Stop the Diagnostic Mode
#define    COMMAD_IRINIT   "IRINIT:"  //Initialize the System to Check for Infrared
#define    CAOMMAD_IRCHK   "IRCHK:"  //This version of IRCHK uses only the simple "presence of infrared light" call, that returns a binary "IR detected/no IR detected" answer
#define    COMMAD_PHSR   "PHSR:"   //Port Handle Search. Will write returned port handles to the string portHandles
#define    COMMAD_PHF   "PHF:"  //Port Handle Free. Frees the port handle.
#define    COMMAD_PHRQ   "PHRQ:"  //Port Handle Request. Will request a Port Handle for a wireless tool and return it in the string portHandle
#define    COMMAD_PVWR   "PVWR:" //Port Virtual Write. Writes an SROM Image data to a tool
#define    COMMAD_PINIT   "PINIT:" //Port Initialize. Will initialize a Port that has been acquired with PHRQ and has been assigned a SROM File with PVWR
#define    COMMAD_PENA   "PENA:"  //Port Enable. Will enable a port that has been initialized with PINIT
#define    COMMAD_PDIS   "PDIS:"  //Port Disable. Will disable a port that has been enabled with PENA
#define    COMMAD_IRATE   "IRATE:" //Setting the illuminator rate. Will set the refresh rate for the illuminator for wireless tools
#define    COMMAD_BEEP   "BEEP:" //Sounding the beeper. The tracking system will beep one to nine times
#define    COMMAD_SFLIST   "SFLIST:" //Returns information about the supported feature of the tracking system
#define    COMMAD_TSTART   "TSTART:" //Start Tracking Mode. The tracking system must be in setup mode and must be initialized.
#define    COMMAD_TSTOP   "TSTOP:" //Stop Tracking Mode. The tracking system must be in Tracking mode.
#define    COMMAD_TX   "TX:" //Report transformations in text mode. Optionally, individual markers can be tracked
#define    COMMAD_BX   "BX:" //Report transformations in binary mode.
#define    COMMAD_3D   "3D:" //eport 3D Positions of single markers.
#define    COMMAD_VSEL   "VSEL:" //ets the tracking volume to the given type. Check available tracking volumes with SFLIST first



enum REPLAY_TYPE{
    REPLAY_BREAK = 0,
    REPLAY_OPERATION,
    REPLAY_PHSR,
    REPLAY_PHRQ,
    REPLAY_SFLIST,
    REPLAY_3D,
    REPLAY_TX,
    REPLAY_ERROR

};

    Q_OBJECT
public:
    SerialInterpreter(QObject* parent = NULL);

    bool setSerialPort(SerialWorker *serial);
    const std::string calcCRC(const std::string* input);
    bool replay(int replayType, const std::__cxx11::string &data = std::string());
    NDIErrorCode cmdInterpreter(const QByteArray &data);

signals:
    void startTracking();
    void stopTracking();
    void startDiagnosing();
    void stopDiagnosing();
private:
    QByteArray m_crcValue;
    std::string m_commad;
    vpsToolManager *m_passiveTool;
    SerialWorker *m_serialCommunication;
};

#endif // SERIALINTERPRETER_H
