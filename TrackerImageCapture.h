#ifndef TRACKERIMAGECAPTURE_H
#define TRACKERIMAGECAPTURE_H

#include <QObject>
#include <Camera.h>

using namespace FlyCapture2;

class TrackerLightsController;
class TrackerImagepool;

class TrackerImageCapture : public QObject
{
	Q_OBJECT

public:
	TrackerImageCapture();
	virtual ~TrackerImageCapture();

public slots:
	void startCapture();
	void capture();
	void stopCapture();

signals:
	void captured();
	void errorOccured(const QByteArray& msg);

private:
	bool initCameras();
	void determineCameraOrder();
	void initLightsController();

private:
	bool m_IsCapturing;
	Camera *m_LeftCamera;
	Camera *m_RightCamera;
	TrackerLightsController *m_LightsController;
	TrackerImagepool *m_Imagepool;
};

#endif // TRACKERIMAGECAPTURE_H
