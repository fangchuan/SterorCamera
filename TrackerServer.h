#ifndef TRACKERSERVER_H
#define TRACKERSERVER_H

#include <QThread>
#include <QTimer>
#include <QSerialPort>

class SerialWorker;
class TrackerImageCapture;
class TrackerImageProcessor;

class TrackerServer: public QObject
{
	Q_OBJECT

public:
	explicit TrackerServer(QObject *parent = NULL);
	virtual ~TrackerServer();

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
	void processed();
    void startTimer(int time);
    void stopTimer();

private:
    bool openSerialPort();
    void closeSerialPort();
    void readData();

private slots:
    void checkBusJam();
    void sendMessage(const QByteArray& msg);
    void handleError(const QByteArray& error);
    void handleError(QSerialPort::SerialPortError &err);
    void handleUploadFile(const QByteArray& data);
    void saveFile();

private:
	TrackerImageCapture *m_Capture;
	TrackerImageProcessor *m_Processor;
    SerialWorker *m_serial;
    QThread *serialThread;
    QThread *imgThread;

};

#endif // TRACKERSERVER_H
