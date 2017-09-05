#ifndef TRACKERIMAGEPROCESSOR_H
#define TRACKERIMAGEPROCESSOR_H

#include <QObject>

class TrackerImagepool;

class TrackerImageProcessor : public QObject
{
	Q_OBJECT

public:
	explicit TrackerImageProcessor();
	virtual ~TrackerImageProcessor();

public slots:
	void startTracking();
	void stopTracking();
	void startManufacturing();
	void stopManufacturing();
	void startDiagnosing();
	void stopDiagnosing();
	void process();
    void busJam();
    void busIdle();

signals:
	void processed();
	void result(const QByteArray& r);

private:
	TrackerImagepool *m_Imagepool;
	int m_State;
	bool m_SocketOverload;
};

#endif // TRACKERIMAGEPROCESSOR_H
