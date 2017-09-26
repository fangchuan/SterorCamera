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
    if( m_Instance )
        delete m_Instance;
    m_Instance = NULL;
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
