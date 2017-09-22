#ifndef SERIALWORKER_H
#define SERIALWORKER_H

#include <QSerialPort>
#include <QTimer>

class SerialInterpreter;

class SerialWorker : public QSerialPort
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
    void closed();
public slots:
    bool setUpDefault();
    void closeSerialPort();
    void readData();
    void handleError(const QByteArray& error);
    bool sendMessage(const QByteArray& msg);
    bool handleUploadFile(const QByteArray& data);
private slots:
    void checkBusJam();
    void handleFatalError(QSerialPort::SerialPortError error);

    bool saveFile();
private:

    QTimer *m_timer;
    bool m_IsUploadingFile;
    bool m_IsAppendingFile;
    QString m_Filename;
    QByteArray m_FileData;
    qint64 m_FileLength;
    SerialInterpreter* m_interpreter;

};

#endif // SERIALWORKER_H
