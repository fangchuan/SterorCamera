#ifndef TRACKERLIGHTSCONTROLLER_H
#define TRACKERLIGHTSCONTROLLER_H

#include <Camera.h>

using namespace FlyCapture2;

class TrackerLightsController
{
public:
	explicit TrackerLightsController();
	virtual ~TrackerLightsController();

	void setLeftCamera(Camera *camera);
	void setRightCamera(Camera *camera);
	void turnOnLeftLight();
	void turnOnRightLight();
	void turnOffLeftLight();
	void turnOffRightLight();

private:
	Camera *m_LeftCamera;
	Camera *m_RightCamera;
};

#endif // TRACKERLIGHTSCONTROLLER_H
