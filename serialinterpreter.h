#ifndef SERIALINTERPRETER_H
#define SERIALINTERPRETER_H

#include <QObject>
#include "serialworker.h"
#include "trackingtypes.h"
#include <QMutex>


class SerialWorker;

class SerialInterpreter
{

#define LF 0x0A
#define CR 0x0D
#define BREAK_REPLAY_INFO  "RESETBE6F\r"

enum COMMAD_TYPE{
    COMMAD_APIREV = "APIREV:",
    COMMAD_PHINF = "PHINF:",
    COMMAD_COMM = "COMM:",   //Change Serial Communication Parameters
    COMMAD_INIT = "INIT:",  //Initialize the Measurement System
    COMMAD_DSTART = "DSTART:",//Start the Diagnostic Mode
    COMMAD_DSTOP = "DSTOP:",   //Stop the Diagnostic Mode
    COMMAD_IRINIT = "IRINIT:", //Initialize the System to Check for Infrared
    CAOMMAD_IRCHK = "IRCHK:", //This version of IRCHK uses only the simple "presence of infrared light" call, that returns a binary "IR detected/no IR detected" answer
    COMMAD_PHSR = "PHSR:",   //Port Handle Search. Will write returned port handles to the string portHandles
    COMMAD_PHF = "PHF:", //Port Handle Free. Frees the port handle.
    COMMAD_PHRQ = "PHRQ:", //Port Handle Request. Will request a Port Handle for a wireless tool and return it in the string portHandle
    COMMAD_PVWR = "PVWR:",//Port Virtual Write. Writes an SROM Image data to a tool
    COMMAD_PINIT = "PINIT:",//Port Initialize. Will initialize a Port that has been acquired with PHRQ and has been assigned a SROM File with PVWR
    COMMAD_PENA = "PENA:", //Port Enable. Will enable a port that has been initialized with PINIT
    COMMAD_PDIS = "PDIS:", //Port Disable. Will disable a port that has been enabled with PENA
    COMMAD_IRATE = "IRATE:",//Setting the illuminator rate. Will set the refresh rate for the illuminator for wireless tools
    COMMAD_BEEP = "BEEP:",//Sounding the beeper. The tracking system will beep one to nine times
    COMMAD_SFLIST = "SFLIST:",//Returns information about the supported feature of the tracking system
    COMMAD_TSTART = "TSTART:",//Start Tracking Mode. The tracking system must be in setup mode and must be initialized.
    COMMAD_TSTOP = "TSTOP:",//Stop Tracking Mode. The tracking system must be in Tracking mode.
    COMMAD_TX = "TX:",//Report transformations in text mode. Optionally, individual markers can be tracked
    COMMAD_BX = "BX:",//Report transformations in binary mode.
    COMMAD_3D = "3D:",//eport 3D Positions of single markers.
    COMMAD_VSEL = "VSEL:",//ets the tracking volume to the given type. Check available tracking volumes with SFLIST first
};

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

public:
    serialInterpreter();

    bool setSerialPort(SerialWorker *serial);
    const std::string calcCRC(const std::string* input);
    bool replay(int replayType, const std::__cxx11::string &data = QByteArray());
    NDIErrorCode cmdInterpreter(const QByteArray &data);

private:
    QByteArray m_crcValue;
    COMMAD_TYPE m_commad;

    QMutex *m_serialMutex;
    SerialWorker *m_serialCommunication;
};

#endif // SERIALINTERPRETER_H
