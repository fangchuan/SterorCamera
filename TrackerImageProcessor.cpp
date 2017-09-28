#include "includes.h"
#include "Image.h"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <vector>

#include <QDataStream>
#include <QSharedMemory>
#include <QWaitCondition>
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

void TrackerImageProcessor::process3D()
{
    Image *p = m_Imagepool->getImagePtr();

    if(p->GetDataSize() != 0)
    {
        QByteArray r;

        MarkerCenters leftCenters, rightCenters;
        getMarkerCenters2D(p, leftCenters);
        getMarkerCenters2D(p+1, rightCenters);
        getMarkerCenters3D(leftCenters, rightCenters);

        emit processed();
        emit result(r);
    }
}

void TrackerImageProcessor::process6D()
{
    Image *p = m_Imagepool->getImagePtr();

    if(p->GetDataSize() != 0)
    {
        QByteArray r;

        MarkerCenters leftCenters, rightCenters;
        getMarkerCenters2D(p, leftCenters);
        getMarkerCenters2D(p+1, rightCenters);
        getMarkerCenters3D(leftCenters, rightCenters);

        emit processed();
        emit result(r);
    }
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


void loadData()
{
    QFile global("global.mat");
    QDataStream gs(&global);
    gs.setByteOrder(QDataStream::LittleEndian);
    if (global.open(QFile::ReadOnly)) {
        double value;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 4; ++j) {
                gs >> value;
                m_LeftMatrix(i, j) = value;
            }
        }
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 4; ++j) {
                gs >> value;
                m_RightMatrix(i, j) = value;
            }
        }
        global.close();
    }
    QFile map("map.dat");
    QDataStream ms(&map);
    ms.setByteOrder(QDataStream::LittleEndian);
    if (map.open(QFile::ReadOnly)) {
        int size;
        ms >> size;
        int key, value;
        for (int i = 0; i < size; ++i) {
            ms >> key >> value;
            m_BlockMap[key] = value;
        }
        map.close();
    }
    QFile matirxs("matrixs.mat");
    QDataStream ts(&matirxs);
    ts.setByteOrder(QDataStream::LittleEndian);
    if (matirxs.open(QFile::ReadOnly)) {
        int size;
        ts >> size;
        double value;
        for (int i = 0; i < size; ++i) {
            m_Matrixs.push_back(new Matrix<double, 4, 4>());
        }
        for (int i = 0; i < size; ++i) {
            auto matrix = m_Matrixs[i];
            matrix->setZero();
            for (int j = 0; j < 4; ++j) {
                for (int k = 0; k < 4; ++k) {
                    ts >> value;
                    (*matrix)(j, k) = value;
                }
            }
        }
        matirxs.close();
    }
    QFile plan("plan.dat");
    QDataStream ps(&plan);
    ps.setByteOrder(QDataStream::LittleEndian);
    if (plan.open(QFile::ReadOnly)) {
        m_Grid = vtkRectilinearGrid::New();
        int size;
        double value;
        ps >> size;
        vtkDoubleArray *xArray = vtkDoubleArray::New();
        vtkDoubleArray *yArray = vtkDoubleArray::New();
        vtkDoubleArray *zArray = vtkDoubleArray::New();
        for (int i = 0; i < size; ++i) {
            ps >> value;
            xArray->InsertNextValue(value);
        }
        ps >> size;
        for (int i = 0; i < size; ++i) {
            ps >> value;
            yArray->InsertNextValue(value);
        }
        ps >> size;
        for (int i = 0; i < size; ++i) {
            ps >> value;
            zArray->InsertNextValue(value);
        }
        m_Grid->SetDimensions(xArray->GetNumberOfValues(), yArray->GetNumberOfValues(), zArray->GetNumberOfValues());
        m_Grid->SetXCoordinates(xArray);
        xArray->Delete();
        m_Grid->SetYCoordinates(yArray);
        yArray->Delete();
        m_Grid->SetZCoordinates(zArray);
        zArray->Delete();
        plan.close();
    }
}



void correct(Matrix<double, 4, 4>* matrix, double& x, double& y, double& z) {
    VectorXd U(4);
    U(0) = x;
    U(1) = y;
    U(2) = z;
    U(3) = 1;
    VectorXd R(4);
    R = (*matrix) * U;
    x = R(0);
    y = R(1);
    z = R(2);
}

bool reconstruct(double lx, double ly, double rx, double ry, double& x, double& y, double& z) {
    MatrixXd A(4, 3);
    VectorXd U(4);
    VectorXd M(3); //要求的坐标
    A.setZero();
    U.setZero();
    M.setZero();

    A(0, 0) = m_LeftMatrix(2, 0) * lx - m_LeftMatrix(0, 0); //u1m31 - m11
    A(0, 1) = m_LeftMatrix(2, 1) * lx - m_LeftMatrix(0, 1); //u1m32 - m12
    A(0, 2) = m_LeftMatrix(2, 2) * lx - m_LeftMatrix(0, 2); //u1m33 - m13
    A(1, 0) = m_LeftMatrix(2, 0) * ly - m_LeftMatrix(1, 0); //v1m31 - m21
    A(1, 1) = m_LeftMatrix(2, 1) * ly - m_LeftMatrix(1, 1); //v1m32 - m22
    A(1, 2) = m_LeftMatrix(2, 2) * ly - m_LeftMatrix(1, 2); //v1m33 - m23

    A(2, 0) = m_RightMatrix(2, 0) * rx - m_RightMatrix(0, 0); //u1m31 - m11
    A(2, 1) = m_RightMatrix(2, 1) * rx - m_RightMatrix(0, 1); //u1m32 - m12
    A(2, 2) = m_RightMatrix(2, 2) * rx - m_RightMatrix(0, 2); //u1m33 - m13

    A(3, 0) = m_RightMatrix(2, 0) * ry - m_RightMatrix(1, 0); //v1m31 - m21
    A(3, 1) = m_RightMatrix(2, 1) * ry - m_RightMatrix(1, 1); //v1m32 - m22
    A(3, 2) = m_RightMatrix(2, 2) * ry - m_RightMatrix(1, 2); //v1m33 - m23

    U(0) = m_LeftMatrix(0, 3) - m_LeftMatrix(2, 3) * lx; //m14 - um34
    U(1) = m_LeftMatrix(1, 3) - m_LeftMatrix(2, 3) * ly; //m24 - vm34
    U(2) = m_RightMatrix(0, 3) - m_RightMatrix(2, 3) * rx; //m14 - um34
    U(3) = m_RightMatrix(1, 3) - m_RightMatrix(2, 3) * ry; //m24 - vm34

    M = A.jacobiSvd(ComputeThinU | ComputeThinV).solve(U); //使用最小二乘法解 M

    x = M(0);
    y = M(1);
    z = M(2);

    double point[3] = { x, y, z };
    vtkIdType cellid = 0;
    double tol2 = 0.0;
    int subid = 0;
    double pcoord[3];
    double weights[8];
    vtkIdType gridCellId = m_Grid->FindCell(point, nullptr, cellid, tol2, subid, pcoord, weights);
    if (gridCellId != -1 && m_BlockMap.find(gridCellId) != m_BlockMap.end()) {
        correct(m_Matrixs[m_BlockMap[gridCellId]], x, y, z);
        return true;
    }
    return false;
}
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

//record the camera coordinate and 3D coordinate every 30s
/*
void timeout() {
    if (!m_SharedMemory->isAttached()) {
        return;
    }
    QBuffer buffer;
    QDataStream in(&buffer);
    in.setByteOrder(QDataStream::LittleEndian);
    m_SharedMemory->lock();
    buffer.setData((char*)m_SharedMemory->constData(), m_SharedMemory->size());
    buffer.open(QBuffer::ReadOnly);
    int flag;
    in >> flag;
    if (flag == 1) {
        // new data
        double leftx, lefty, rightx, righty, x, y, z;
        in >> leftx >> lefty >> rightx >> righty >> x >> y >> z;

        char *to = (char*)m_SharedMemory->data();
        char *from = (char*)buffer.data().data();
        from[0] = 0;
        memcpy(to, from, m_SharedMemory->size());
        m_SharedMemory->unlock();

        std::ofstream camera("camera.txt", std::ios::app);
        camera << leftx << " " << lefty << " " << rightx << " " << righty << endl;
        camera.close();
        std::ofstream xyz("xyz.txt", std::ios::app);
        xyz << x << " " << y << " " << z << endl;
        xyz.close();
        double cx, cy, cz;
        if (reconstruct(leftx, lefty, rightx, righty, cx, cy, cz)) {
            std::ofstream rxyz("reconstruct.txt", std::ios::app);
            rxyz << cx << " " << cy << " " << cz << endl;
            rxyz.close();
        }
        else {
            std::ofstream rxyz("reconstruct.txt", std::ios::app);
            rxyz << -1 << " " << -1 << " " << -1 << endl;
            rxyz.close();
        }
    }
    else {
        m_SharedMemory->unlock();
    }
}
*/
