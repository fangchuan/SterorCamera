#ifndef TRACKERSOCKET_H
#define TRACKERSOCKET_H

#include <QTcpSocket>

class TrackerSocket : public QTcpSocket
{
	Q_OBJECT

public:
	TrackerSocket(QObject *parent = NULL);
	~TrackerSocket();

public slots:
	void sendMessage(const QByteArray &bytes);

signals:
	void startTracking();
	void stopTracking();
	void startManufacturing();
	void manufacture();
	void stopManufacturing();
	void startDiagnosing();
	void stopDiagnosing();
	void startTimer(int msec);
	void stopTimer();
	void socketJam();
	void socketIdle();

private:
	void handleUploadFile(const QByteArray &data);
	void saveFile();

private slots:
	void readData();
	void checkSocketJam();

private:
	bool m_IsUploadingFile;
	bool m_IsAppendingFile;
	QByteArray m_FileData;
	QString m_Filename;
	qint64 m_FileLength;
};

#endif // TRACKERSOCKET_H
