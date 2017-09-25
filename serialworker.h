#ifndef SERIALWORKER_H
#define SERIALWORKER_H

#include <QSerialPort>
#include <QTimer>

class SerialInterpreter;

class SerialWorker : public QObject
{
    Q_OBJECT
public:
    SerialWorker(QObject* parent = NULL);
    ~SerialWorker();

    void SetDeviceName(QString &portName);
    bool SetBaudRate(char buadRate);
    bool SetDataBits(char dataBits);
    bool SetParity(char parity);
    bool SetStopBits(char stopBits);
    bool SetFlowControl(char flowControl);
    int OpenConnection();
    int CloseConnection();

signals:
    void startTracking();
    void stopTracking();
    void startManufacturing();
    void stopManufacturing();
    void startDiagnosing();
    void stopDiagnosing();
    void busJam();
    void busIdle();
    void captured();
    void manufacting();
    void startTimer(int time);
    void stopTimer();
    void serialWorkerStop();
public slots:
    bool setUpDefault();
    void readData();
    void handleError(const QByteArray& error);
    bool sendPosData(const QByteArray& msg);

private slots:
    void checkBusJam();
    void handleFatalError(QSerialPort::SerialPortError error);

private:

    QTimer *m_timer;
    SerialInterpreter* m_interpreter;
    QSerialPort *m_serialPort;
};

#endif // SERIALWORKER_H
