#ifndef TRACKERIMAGEPOOL_H
#define TRACKERIMAGEPOOL_H

#include <Image.h>

class TrackerImagepool
{
public:
	~TrackerImagepool();

	static TrackerImagepool *getInstance();
	FlyCapture2::Image *getImagePtr();

private:
	TrackerImagepool();

private:
	static TrackerImagepool *m_Instance;
	FlyCapture2::Image m_Image[2];
};

#endif // TRACKERIMAGEPOOL_H
