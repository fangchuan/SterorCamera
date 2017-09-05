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


private:
	TrackerImageCapture *m_Capture;
	TrackerImageProcessor *m_Processor;
    SerialWorker *m_serial;
    QThread *serialThread;
    QThread *imgThread;

};

#endif // TRACKERSERVER_H
