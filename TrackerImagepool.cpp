#include "TrackerImagepool.h"

#include <QMutexLocker>

TrackerImagepool *TrackerImagepool::m_Instance = NULL;

static QMutex Mutex;

TrackerImagepool::TrackerImagepool()
{
	m_Instance = this;
}

TrackerImagepool::~TrackerImagepool()
{

}

TrackerImagepool *TrackerImagepool::getInstance()
{
	QMutexLocker locker(&Mutex);
	return m_Instance == NULL ? new TrackerImagepool : m_Instance;
}

FlyCapture2::Image *TrackerImagepool::getImagePtr()
{
	return m_Image;
}
