#include "TrackerImageCapture.h"
#include "TrackerLightsController.h"
#include "TrackerImagepool.h"

#include <BusManager.h>
#include <Error.h>
#include <Image.h>

#include <QDataStream>
#include <QDir>

#include <Windows.h>

TrackerImageCapture::TrackerImageCapture()
	: m_IsCapturing(false),
	m_LeftCamera(NULL),
	m_RightCamera(NULL),
	m_Imagepool(NULL),
	m_LightsController(NULL)
{

}

TrackerImageCapture::~TrackerImageCapture()
{
	stopCapture();
	if (m_LeftCamera)
	{
		m_LeftCamera->Disconnect();
	}
	if (m_RightCamera)
	{
		m_RightCamera->Disconnect();
	}
}

void TrackerImageCapture::startCapture()
{
	if (!initCameras())
	{
		return ;
	}
	initLightsController();

	m_LeftCamera->StartCapture();
	m_RightCamera->StartCapture();

	if (!m_Imagepool)
	{
		m_Imagepool = TrackerImagepool::getInstance();
	}

	m_IsCapturing = true;

	capture();
}

void TrackerImageCapture::capture()
{
	if (m_IsCapturing)
	{
		Image *ptr = m_Imagepool->getImagePtr();
		m_LightsController->turnOnLeftLight();
		m_LeftCamera->WriteRegister(0x62C, 0x80000000);//set softwaretrigger bit 0
		m_LeftCamera->RetrieveBuffer(ptr);
		m_LightsController->turnOffLeftLight();

		m_LightsController->turnOnRightLight();
		m_RightCamera->WriteRegister(0x62C, 0x80000000);
		m_RightCamera->RetrieveBuffer(ptr+1);
		m_LightsController->turnOffRightLight();

		emit captured();
	}
}

void TrackerImageCapture::stopCapture()
{
	m_IsCapturing = false;
	if (m_LeftCamera && m_RightCamera)
	{
		m_LeftCamera->StopCapture();
		m_RightCamera->StopCapture();
	}
}

bool TrackerImageCapture::initCameras()
{
	if (m_LeftCamera && m_RightCamera)
	{
		return true;
	}

	QByteArray errorMsg;
	QDataStream stream(&errorMsg, QIODevice::WriteOnly);
	stream << QString("ERROR");
	int lengthPos = stream.device()->pos();
	stream << qint64(0) << QString("Capture");
	const PixelFormat pixelFormat = PIXEL_FORMAT_MONO8;
	const Mode mode = MODE_0;
	Error error;
	BusManager busMgr;
	unsigned int numCameras;
	error = busMgr.GetNumOfCameras(&numCameras);
	if (error != PGRERROR_OK)
	{
		stream << QString(error.GetDescription());
		int length = errorMsg.length();
		stream.device()->seek(lengthPos);
		stream << qint64(length);
		emit errorOccured(errorMsg);
		return false;
	}
	if (numCameras != 2)
	{
		stream << QString("The camera number is not correct %1").arg(numCameras);
		int length = errorMsg.length();
		stream.device()->seek(lengthPos);
		stream << qint64(length);
		emit errorOccured(errorMsg);
		return false;
	}

	Camera **ppCameras = new Camera *[numCameras];

	// Connect to all detected cameras and attempt to set them to
	// a common video mode and frame rate
	for (unsigned i = 0; i < numCameras; i++)
	{
		ppCameras[i] = new Camera;
		PGRGuid guid;
		error = busMgr.GetCameraFromIndex(i, &guid);
		if (error != PGRERROR_OK)
		{
			stream << QString(error.GetDescription());
			int length = errorMsg.length();
			stream.device()->seek(lengthPos);
			stream << qint64(length);
			emit errorOccured(errorMsg);
			for (int j = 0; j <= i; ++j)
			{
				delete ppCameras[j];
			}
			delete [] ppCameras;
			return false;
		}

		// Connect to a camera
		error = ppCameras[i]->Connect(&guid);
		if (error != PGRERROR_OK)
		{
			stream << QString(error.GetDescription());
			int length = errorMsg.length();
			stream.device()->seek(lengthPos);
			stream << qint64(length);
			emit errorOccured(errorMsg);
			for (int j = 0; j <= i; ++j)
			{
				delete ppCameras[j];
			}
			delete [] ppCameras;
			return false;
		}

		// Get the camera information
		CameraInfo camInfo;
		error = ppCameras[i]->GetCameraInfo(&camInfo);
		if (error != PGRERROR_OK)
		{
			stream << QString(error.GetDescription());
			int length = errorMsg.length();
			stream.device()->seek(lengthPos);
			stream << qint64(length);
			emit errorOccured(errorMsg);
			for (int j = 0; j <= i; ++j)
			{
				delete ppCameras[j];
			}
			delete [] ppCameras;
			return false;
		}
		//format the camera information to send
		/*QString info;
		info = "Camera Info: Serial number - " + QString::number(camInfo.serialNumber) + ",Camera model - " +
		camInfo.modelName + ",Camera vendor - " + camInfo.vendorName + ",Sensor - " + camInfo.sensorInfo +
		",Resolution - " + camInfo.sensorResolution;
		emit sendData(info);*/

		// Set all cameras to a specific mode and frame rate so they
		// can be synchronized
		Format7Info fmt7Info;
		bool ok = false;
		fmt7Info.mode = mode;
		error = ppCameras[i]->GetFormat7Info(&fmt7Info, &ok);
		if (error != PGRERROR_OK)
		{
			stream << QString(error.GetDescription());
			int length = errorMsg.length();
			stream.device()->seek(lengthPos);
			stream << qint64(length);
			emit errorOccured(errorMsg);
			for (int j = 0; j <= i; ++j)
			{
				delete ppCameras[j];
			}
			delete [] ppCameras;
			return false;
		}
		if ( (pixelFormat & fmt7Info.pixelFormatBitField) == 0 )
		{
			stream << QString(error.GetDescription());
			int length = errorMsg.length();
			stream.device()->seek(lengthPos);
			stream << qint64(length);
			emit errorOccured(errorMsg);
			for (int j = 0; j <= i; ++j)
			{
				delete ppCameras[j];
			}
			delete [] ppCameras;
			return false;
		}

		Format7ImageSettings fmt7ImageSettings;
		fmt7ImageSettings.mode = mode;
		fmt7ImageSettings.offsetX = 0;
		fmt7ImageSettings.offsetY = 0;
		fmt7ImageSettings.width =  1920;
		fmt7ImageSettings.height =  1080;
		fmt7ImageSettings.pixelFormat = pixelFormat;

		bool valid;
		Format7PacketInfo fmt7PacketInfo;
		// Validate the settings to make sure that they are valid
		error = ppCameras[i]->ValidateFormat7Settings(&fmt7ImageSettings, &valid, &fmt7PacketInfo );
		if (error != PGRERROR_OK)
		{
			stream << QString(error.GetDescription());
			int length = errorMsg.length();
			stream.device()->seek(lengthPos);
			stream << qint64(length);
			emit errorOccured(errorMsg);
			for (int j = 0; j <= i; ++j)
			{
				delete ppCameras[j];
			}
			delete [] ppCameras;
			return false;
		}

		if ( !valid )
		{
			// Settings are not valid
			stream << QString(error.GetDescription());
			int length = errorMsg.length();
			stream.device()->seek(lengthPos);
			stream << qint64(length);
			emit errorOccured(errorMsg);
			for (int j = 0; j <= i; ++j)
			{
				delete ppCameras[j];
			}
			delete [] ppCameras;
			return false;
		}

		// Set the settings to the camera
		error = ppCameras[i]->SetFormat7Configuration(&fmt7ImageSettings, fmt7PacketInfo.recommendedBytesPerPacket );
		if (error != PGRERROR_OK)
		{
			stream << QString(error.GetDescription());
			int length = errorMsg.length();
			stream.device()->seek(lengthPos);
			stream << qint64(length);
			emit errorOccured(errorMsg);
			for (int j = 0; j <= i; ++j)
			{
				delete ppCameras[j];
			}
			delete [] ppCameras;
			return false;
		}

		TriggerMode triggerMode;
		error = ppCameras[i]->GetTriggerMode(&triggerMode);
		triggerMode.onOff = true;
		triggerMode.mode = 0;
		triggerMode.parameter = 0;
		triggerMode.source = 7;
		error = ppCameras[i]->SetTriggerMode(&triggerMode);

		//FC2Config configuration;
		//ppCameras[i]->GetConfiguration(&configuration);
		//configuration.grabMode = DROP_FRAMES;
		//configuration.numBuffers = 2;
		////configuration.grabTimeout = 10;
		//error = ppCameras[i]->SetConfiguration(&configuration);
		if (error != PGRERROR_OK)
		{
			stream << QString(error.GetDescription());
			int length = errorMsg.length();
			stream.device()->seek(lengthPos);
			stream << qint64(length);
			emit errorOccured(errorMsg);
			for (int j = 0; j <= i; ++j)
			{
				delete ppCameras[j];
			}
			delete [] ppCameras;
			return false;
		}
	}
	m_LeftCamera = ppCameras[0];
	m_RightCamera = ppCameras[1];

	determineCameraOrder();

	return true;
}

void TrackerImageCapture::determineCameraOrder()
{
	QDir dir(".");
	QStringList leftCameraId = dir.entryList(QStringList() << "*.left");
	QStringList rightCameraId = dir.entryList(QStringList() << "*.right");
	if (!leftCameraId.isEmpty())
	{
		CameraInfo camInfo;
		m_LeftCamera->GetCameraInfo(&camInfo);
		if (leftCameraId[0].contains(QString::number(camInfo.serialNumber)))
		{

		}
		else
		{
			Camera *temp = m_RightCamera;
			m_RightCamera = m_LeftCamera;
			m_LeftCamera = temp;
		}
	}
	else if (!rightCameraId.isEmpty())
	{
		CameraInfo camInfo;
		m_RightCamera->GetCameraInfo(&camInfo);
		if (rightCameraId[0].contains(QString::number(camInfo.serialNumber)))
		{

		}
		else
		{
			Camera *temp = m_RightCamera;
			m_RightCamera = m_LeftCamera;
			m_LeftCamera = temp;
		}
	}
}

void TrackerImageCapture::initLightsController()
{
	if (!m_LightsController)
	{
		m_LightsController = new TrackerLightsController;
		m_LightsController->setLeftCamera(m_LeftCamera);
		m_LightsController->setRightCamera(m_RightCamera);
	}
}