#ifndef SERIALWORKER_H
#define SERIALWORKER_H

#include <QSerialPort>
#include <QTimer>


class SerialWorker : public QSerialPort
{
    Q_OBJECT
public:
    SerialWorker(QObject* parent);
    ~SerialWorker();

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

public slots:
    bool setup();
    void closeSerialPort();
    void readData();
    void handleError(const QByteArray& error);
    void sendMessage(const QByteArray& msg);
private slots:
    void checkBusJam();
    void handleError(QSerialPort::SerialPortError &err);
    void handleUploadFile(const QByteArray& data);
    void saveFile();
private:

    QTimer *m_timer;
    bool m_IsUploadingFile;
    bool m_IsAppendingFile;
    QString m_Filename;
    QByteArray m_FileData;
    qint64 m_FileLength;

};

#endif // SERIALWORKER_H
