#include "includes.h"
#include "Image.h"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <vector>

#include <QDataStream>
#include <QDebug>

using namespace FlyCapture2;

typedef std::vector<Point2D> MarkerCenters;

const int IRCAM_THRES_BIN = 190;

TrackerImageProcessor::TrackerImageProcessor()
	:m_Imagepool(NULL),
	m_State(Tracker::INVALID),
	m_SocketOverload(false)
{

}

TrackerImageProcessor::~TrackerImageProcessor()
{
    if( m_Imagepool != NULL)
        delete m_Imagepool;
    m_Imagepool = NULL;
}

void TrackerImageProcessor::startTracking()
{
	if (!m_Imagepool)
	{
		m_Imagepool = TrackerImagepool::getInstance();
	}

	m_State = Tracker::TRACKING_3D;
}

void TrackerImageProcessor::stopTracking()
{
	m_State = Tracker::INVALID;
}

void TrackerImageProcessor::startManufacturing()
{
	if (!m_Imagepool)
	{
		m_Imagepool = TrackerImagepool::getInstance();
	}

	m_State = Tracker::MANUFACTURING;
}

void TrackerImageProcessor::stopManufacturing()
{
	m_State = Tracker::INVALID;
}

void TrackerImageProcessor::startDiagnosing()
{
	if (!m_Imagepool)
	{
		m_Imagepool = TrackerImagepool::getInstance();
	}

	m_State = Tracker::DIAGNOSING;
}

void TrackerImageProcessor::stopDiagnosing()
{
	m_State = Tracker::INVALID;
}

void getMarkerCenters2D(Image *p, MarkerCenters& centers)
{
	centers.clear();
	cv::Mat cvImage(p->GetRows(), p->GetCols(), CV_8UC1, p->GetData());
	if (!cvImage.data)
	{
		return ;
	}

	std::vector<std::vector<cv::Point> > contours;
	cv::Mat binImage;
	cv::threshold(cvImage, binImage, IRCAM_THRES_BIN, 255, cv::THRESH_BINARY);
	cv::findContours(binImage, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE,
		cv::Point(0, 0));

    for (unsigned int i = 0; i < contours.size(); ++i)
	{
		double area = cv::contourArea(contours.at(i));
		double circum = cv::arcLength(contours.at(i), 1);
		double ratio = circum * circum / area;

		if (area > 50 && area < 1500 && ratio > 11.4 && ratio < 16)
		{
			double sum = 0;
			double sumx = 0;
			double sumy = 0;

			cv::Rect boundRect = cv::boundingRect(contours.at(i));
			uchar *p = NULL;
			for (int row = boundRect.y; row < boundRect.y + boundRect.height; ++row)
			{
				p = cvImage.ptr<uchar>(row);
				for (int col = boundRect.x; col < boundRect.x + boundRect.width; ++col)
				{
					if (cv::pointPolygonTest(contours.at(i), cv::Point2f(col, row), 0) >= 0)
					{
						sum += p[col];
						sumy += row * p[col];
						sumx += col * p[col];
					}
				}
			}

			Point2D center;
			center.x = sumx / sum;
			center.y = sumy / sum;
			centers.push_back(center);
		}
	}
}

void getMarkerCenters2DHough(Image *p, MarkerCenters& centers)
{
	centers.clear();
	cv::Mat cvImage(p->GetRows(), p->GetCols(), CV_8UC1, p->GetData());
	if (!cvImage.data)
	{
		return ;
	}

    cv::Mat binImage;
	cv::threshold(cvImage, binImage, IRCAM_THRES_BIN, 255, cv::THRESH_BINARY);
	std::vector<cv::Vec3f> circles;
	cv::HoughCircles(binImage, circles, CV_HOUGH_GRADIENT,
		2,
		10, 
		200, 
		60, 
		20, 100); 
	int size = circles.size();
	for (int i = 0; i < size; ++i)
	{
		Point2D center;
		center.x = circles.at(i)[0];
		center.y = circles.at(i)[1];
		centers.push_back(center);
	}
}

void getMarkerCenters3D(MarkerCenters& leftCenters, MarkerCenters& rightCenters)
{

}

void TrackerImageProcessor::process()
{

	Image *p = m_Imagepool->getImagePtr();

	QByteArray r;
	switch(m_State)
	{
	case Tracker::TRACKING_3D:
		{
			MarkerCenters leftCenters, rightCenters;
			getMarkerCenters2D(p, leftCenters);
			getMarkerCenters2D(p+1, rightCenters);
			getMarkerCenters3D(leftCenters, rightCenters);
			emit processed();
		}
		break;
	case Tracker::MANUFACTURING:
		{
			MarkerCenters leftCenters, rightCenters;
			getMarkerCenters2D(p, leftCenters);
			getMarkerCenters2D(p+1, rightCenters);
			QDataStream stream(&r, QIODevice::WriteOnly);
			stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
			stream << QString("manufacturing");
			int lengthPos = stream.device()->pos();
			stream << qint64(0);
			int leftSize = leftCenters.size();
			int rightSize = rightCenters.size();
			stream << leftSize;
			for (int i = 0; i < leftSize; ++i)
			{
				stream << leftCenters.at(i).x;
				stream << leftCenters.at(i).y;
			}
			stream << rightSize;
			for (int i = 0; i < rightSize; ++i)
			{
				stream << rightCenters.at(i).x;
				stream << rightCenters.at(i).y;
			}
			qint64 length = r.length();
			stream.device()->seek(lengthPos);
			stream << length;
		}
		break;
	case Tracker::DIAGNOSING:
		{
			MarkerCenters leftCenters, rightCenters;
			getMarkerCenters2DHough(p, leftCenters);
			getMarkerCenters2DHough(p+1, rightCenters);
			QDataStream stream(&r, QIODevice::WriteOnly);
			stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
			stream << QString("diagnosing");
			int lengthPos = stream.device()->pos();
			stream << qint64(0);
			int rows = p->GetRows();
			int cols = p->GetCols();
			stream << rows;
			stream << cols;
			stream.writeRawData((char*)(p->GetData()), rows*cols);
			stream.writeRawData((char*)((p+1)->GetData()), rows*cols);
			int leftSize = leftCenters.size();
			int rightSize = rightCenters.size();
			stream << leftSize;
			for (int i = 0; i < leftSize; ++i)
			{
				stream << leftCenters.at(i).x;
				stream << leftCenters.at(i).y;
			}
			stream << rightSize;
			for (int i = 0; i < rightSize; ++i)
			{
				stream << rightCenters.at(i).x;
				stream << rightCenters.at(i).y;
			}
			qint64 length = r.length();
			stream.device()->seek(lengthPos);
			stream << length;
			emit processed();
		}
		break;
	}
#ifdef USE_DEBUG
    qDebug()<<"image process starting...";
#endif
	if (!m_SocketOverload)
	{
		emit result(r);
	}

}

void TrackerImageProcessor::busJam()
{
	m_SocketOverload = true;
}

void TrackerImageProcessor::busIdle()
{
	m_SocketOverload = false;
}
