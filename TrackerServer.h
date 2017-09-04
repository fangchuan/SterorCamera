#ifndef TRACKERSERVER_H
#define TRACKERSERVER_H

#include <QTcpServer>

class TrackerSocket;
class TrackerImageCapture;
class TrackerImageProcessor;

class TrackerServer : public QTcpServer
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
	void socketJam();
	void socketIdle();
	void captured();
	void processed();
	void sendMessage(const QByteArray& msg);

protected:
	virtual void incomingConnection(int socketDescriptor);

private slots:
	void disconnectFromClient();
	void captureError(const QByteArray& error);

private:
	TrackerImageCapture *m_Capture;
	TrackerImageProcessor *m_Processor;
	int m_ClientNumber;
};

#endif // TRACKERSERVER_H
