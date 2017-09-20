#include "vpsnditrackingdevice.h"
#include "vpsigttimestamp.h"
#include "vpsigthardwareexception.h"
#include <stdio.h>


const unsigned char CR = 0xD; // == '\r' - carriage return
const unsigned char LF = 0xA; // == '\n' - line feed


NDITrackingDevice::NDITrackingDevice() :TrackingDevice(),
    m_DeviceName(""),
    m_BaudRate(BaudRate::Baud9600),
    m_DataBits(DataBits::Data8),
    m_Parity(Parity::NoParity),
    m_StopBits(StopBits::OneStop),
    m_HardwareHandshake(FlowControl::NoFlowControl),
    m_IlluminationActivationRate(Hz20),
    m_DataTransferMode(TX),
    m_6DTools(),
    m_ToolsMutex(NULL),
    m_SerialCommunication(NULL),
    m_SerialCommunicationMutex(NULL),
    m_DeviceProtocol(NULL),
    m_MultiThreader(NULL),
    m_ThreadID(0),
    m_OperationMode(ToolTracking6D),
    m_MarkerPointsMutex(NULL),
    m_MarkerPoints()
{
	m_Data = DeviceDataUnspecified;
	m_6DTools.clear();
    m_SerialCommunicationMutex = new QMutex();
    m_DeviceProtocol = new NDIProtocol();
    m_DeviceProtocol->SetTrackingDevice(this);
    m_DeviceProtocol->SetUseCRC(true);
    m_MultiThreader = new QThread();
    m_ToolsMutex = new QMutex();
    m_MarkerPointsMutex = new QMutex();
	m_MarkerPoints.reserve(50);   // a maximum of 50 marker positions can be reported by the tracking device
}


bool NDITrackingDevice::UpdateTool(TrackingTool* tool)
{
	if (this->GetState() != Setup)
	{
		NDIPassiveTool* ndiTool = dynamic_cast<NDIPassiveTool*>(tool);
		if (ndiTool == NULL)
			return false;

		std::string portHandle = ndiTool->GetPortHandle();

		//return false if the SROM Data has not been set
		if (ndiTool->GetSROMData() == NULL)
			return false;

		NDIErrorCode returnvalue;
		returnvalue = m_DeviceProtocol->PVWR(&portHandle, ndiTool->GetSROMData(), ndiTool->GetSROMDataLength());
		if (returnvalue != NDIOKAY)
			return false;
		returnvalue = m_DeviceProtocol->PINIT(&portHandle);
		if (returnvalue != NDIOKAY)
			return false;
		returnvalue = m_DeviceProtocol->PENA(&portHandle, ndiTool->GetTrackingPriority()); // Enable tool
		if (returnvalue != NDIOKAY)
			return false;

		return true;
    }else{
		return false;
	}
}

void NDITrackingDevice::SetRotationMode(RotationMode r)
{
	m_RotationMode = r;
}

NDITrackingDevice::~NDITrackingDevice()
{
	/* stop tracking and disconnect from tracking device */
	if (GetState() == Tracking)
	{
		this->StopTracking();
	}
	if (GetState() == Ready)
	{
		this->CloseConnection();
	}
	/* cleanup tracking thread */
    if ((m_ThreadID != 0) && (m_MultiThreader != NULL))
	{
		m_MultiThreader->TerminateThread(m_ThreadID);
	}
	m_MultiThreader = NULL;
	/* free serial communication interface */
    if (m_SerialCommunication != NULL)
	{
		m_SerialCommunication->ClearReceiveBuffer();
		m_SerialCommunication->ClearSendBuffer();
		m_SerialCommunication->CloseConnection();
		m_SerialCommunication = NULL;
	}
}

void NDITrackingDevice::SetDeviceName(std::string _arg)
{
	if (this->GetState() != Setup)
		return;
#ifdef USE_DEBUG
    qDebug()<<"setting eviceName to " << _arg;
#endif
	if (this->m_DeviceName != _arg){
		this->m_DeviceName = _arg;
	}
}


void NDITrackingDevice::SetBaudRate(const BaudRate _arg)
{
	if (this->GetState() != Setup)
		return;
#ifdef USE_DEBUG
    qDebug()<<"setting BaudRate to " << _arg;
#endif
	if (this->m_BaudRate != _arg){
		this->m_BaudRate = _arg;
	}
}


void NDITrackingDevice::SetDataBits(const DataBits _arg)
{
	if (this->GetState() != Setup)
		return;
#ifdef USE_DEBUG
    qDebug()<<"setting DataBits to " << _arg;
#endif
	if (this->m_DataBits != _arg){
		this->m_DataBits = _arg;
	}
}


void NDITrackingDevice::SetParity(const Parity _arg)
{
	if (this->GetState() != Setup)
		return;
#ifdef USE_DEBUG
    qDebug()<<"setting Parity to " << _arg;
#endif
	if (this->m_Parity != _arg){
		this->m_Parity = _arg;
	}
}


void NDITrackingDevice::SetStopBits(const StopBits _arg)
{
	if (this->GetState() != Setup)
		return;
#ifdef USE_DEBUG
    qDebug()<<"setting StopBits to " << _arg;
#endif
    if (this->m_StopBits != _arg){
		this->m_StopBits = _arg;
	}
}


void NDITrackingDevice::SetHardwareHandshake(const FlowControl _arg)
{
	if (this->GetState() != Setup)
		return;
#ifdef USE_DEBUG
    qDebug()<<"setting HardwareHandshake to " << _arg;
#endif
	if (this->m_HardwareHandshake != _arg){
		this->m_HardwareHandshake = _arg;
	}
}


void NDITrackingDevice::SetIlluminationActivationRate(const IlluminationActivationRate _arg)
{
	if (this->GetState() == Tracking)
		return;
#ifdef USE_DEBUG
    qDebug()<<"setting IlluminationActivationRate to " << _arg;
#endif
    if (this->m_IlluminationActivationRate != _arg){
    // if the connection to the tracking system is established,
    // send the new rate to the tracking device too
		this->m_IlluminationActivationRate = _arg;
        if (this->GetState() == Ready)
			m_DeviceProtocol->IRATE(this->m_IlluminationActivationRate);
	}
}


void NDITrackingDevice::SetDataTransferMode(const DataTransferMode _arg)
{
#ifdef USE_DEBUG
    qDebug()<<"setting DataTransferMode to " << _arg;
#endif
	if (this->m_DataTransferMode != _arg){
		this->m_DataTransferMode = _arg;
	}
}


NDIErrorCode NDITrackingDevice::Send(const std::string* input, bool addCRC)
{
	if (input == NULL)
		return SERIALSENDERROR;

	std::string message;

	if (addCRC == true)
		message = *input + CalcCRC(input) + std::string(1, CR);
	else
		message = *input + std::string(1, CR);

	//unsigned int messageLength = message.length() + 1; // +1 for CR

	// Clear send buffer
	this->ClearSendBuffer();
	// Send the date to the device
    QMutexLocker lock(m_SerialCommunicationMutex); // lock and unlock the mutex
    long returnvalue = m_SerialCommunication->write(message.data());

    if (returnvalue == -1)
		return SERIALSENDERROR;
	else
		return NDIOKAY;
}


NDIErrorCode NDITrackingDevice::Receive(std::string *answer, unsigned int numberOfBytes)
{
	if (answer == NULL)
		return SERIALRECEIVEERROR;

    char *data = answer->data();
    QMutexLocker lock(m_SerialCommunicationMutex); // lock and unlock the mutex
    // never read more bytes than the device has send,
    //the function will block until enough bytes are send...
    long returnvalue = m_SerialCommunication->read(data, numberOfBytes);
    if (returnvalue <= 0)
		return SERIALRECEIVEERROR;
	else
		return NDIOKAY;
}


NDIErrorCode NDITrackingDevice::ReceiveByte(char* answer)
{
	if (answer == NULL)
		return SERIALRECEIVEERROR;

    QMutexLocker lock(m_SerialCommunicationMutex); // lock and unlock the mutex
    long returnvalue = m_SerialCommunication->read(answer, 1);

    if ((returnvalue <= 0) )
		return SERIALRECEIVEERROR;

	return NDIOKAY;
}


NDIErrorCode NDITrackingDevice::ReceiveLine(std::string *answer)
{
	if (answer == NULL)
		return SERIALRECEIVEERROR;

    QMutexLocker lock(m_SerialCommunicationMutex); // lock and unlock the mutex
    char *data = answer->data();
    qint64 bytes = m_SerialCommunication->readLine(data);
    if(bytes > 0)
        return NDIOKAY;
    else
        return SERIALRECEIVEERROR;
}


void NDITrackingDevice::ClearSendBuffer()
{
	MutexLockHolder lock(*m_SerialCommunicationMutex); // lock and unlock the mutex
    m_SerialCommunication->clear(QSerialPort::Output);
}


void NDITrackingDevice::ClearReceiveBuffer()
{
    QMutexLocker lock(m_SerialCommunicationMutex); // lock and unlock the mutex
    m_SerialCommunication->clear(QSerialPort::Input);
}


const std::string NDITrackingDevice::CalcCRC(const std::string* input)
{

	if (input == NULL)
		return "";
	/* the crc16 calculation code is taken from the NDI API guide example code section */
	static int oddparity[16] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};
	unsigned int data;  // copy of the input string's current character
	unsigned int crcValue = 0;  // the crc value is stored here
    // the algorithm uses a pointer to crcValue, so it's easier to provide that than to
    //change the algorithm
    unsigned int* puCRC16 = &crcValue;
	for (unsigned int i = 0; i < input->length(); i++)
	{
		data = (*input)[i];
		data = (data ^ (*(puCRC16) & 0xff)) & 0xff;
		*puCRC16 >>= 8;
		if (oddparity[data & 0x0f] ^ oddparity[data >> 4])
		{
			*(puCRC16) ^= 0xc001;
		}
		data <<= 6;
		*puCRC16 ^= data;
		data <<= 1;
		*puCRC16 ^= data;
	}
	// crcValue contains now the CRC16 value. Convert it to a string and return it
	char returnvalue[13];
	sprintf(returnvalue,"%04X", crcValue);  // 4 hexadecimal digit with uppercase format
	return std::string(returnvalue);
}

bool NDITrackingDevice::OpenConnection()
{
    if (this->GetState() != Setup){
    //{vpsThrowException(IGTException) << "Can only try to open the connection if in setup mode";}
#ifdef USE_DEBUG
        qDebug()<<"Can only try to open the connection if in setup mode";
#endif
        return false;
    }

    m_SerialCommunication = new SerialWorker();

	/* init local com port to standard com settings for a NDI tracking device:
	9600 baud, 8 data bits, no parity, 1 stop bit, no hardware handshake */
	if (m_DeviceName.empty())
        m_DeviceName = QString("/dev/ttyS0");

    m_SerialCommunication->SetDeviceName(m_DeviceName);
    m_SerialCommunication->SetBaudRate(BaudRate::Baud9600);
    m_SerialCommunication->SetDataBits(DataBits::Data8);
    m_SerialCommunication->SetParity(Parity::NoParity);
	m_SerialCommunication->SetStopBits(SerialCommunication::StopBits1);

    if (m_SerialCommunication->OpenConnection() != 0) // 0 == ERROR_VALUE
	{
		m_SerialCommunication->CloseConnection();
        //vpsThrowException(IGTHardwareException) << "Can not open serial port";
	}

	/* Reset Tracking device by sending a serial break for 500ms */
	m_SerialCommunication->SendBreak(400);

	/* Read answer from tracking device (RESETBE6F) */
    QByteArray reset("RESETBE6F\r");
    QByteArray answer;
	this->Receive(&answer, reset.length());  // read answer (should be RESETBE6F)
    // flush the receive buffer of all remaining data (carriage return, strings other than reset
    this->ClearReceiveBuffer();
    if (!(reset == answer))  // check for RESETBE6F
	{
        if (m_SerialCommunication != NULL)		{
			m_SerialCommunication->CloseConnection();
		}
        //vpsThrowException(IGTHardwareException) << "Hardware Reset of tracking device did not work";
	}

	/* Now the tracking device isSetData reset, start initialization */
	NDIErrorCode returnvalue;

	/* set device com settings to new values and wait for the device to change them */
    returnvalue = m_DeviceProtocol->COMM(m_BaudRate,
                                         m_DataBits,
                                         m_Parity,
                                         m_StopBits,
                                         m_HardwareHandshake);

	if (returnvalue != NDIOKAY)
	{vpsThrowException(IGTHardwareException) << "Could not set comm settings in trackingdevice";}

	//after changing COMM wait at least 100ms according to NDI Api documentation page 31
	itksys::SystemTools::Delay(500);

	/* now change local com settings accordingly */
	m_SerialCommunication->CloseConnection();
	m_SerialCommunication->SetBaudRate(m_BaudRate);
	m_SerialCommunication->SetDataBits(m_DataBits);
	m_SerialCommunication->SetParity(m_Parity);
	m_SerialCommunication->SetStopBits(m_StopBits);
	m_SerialCommunication->SetHardwareHandshake(m_HardwareHandshake);
	m_SerialCommunication->OpenConnection();


	/* initialize the tracking device */
	returnvalue = m_DeviceProtocol->INIT();
	if (returnvalue != NDIOKAY)
	{vpsThrowException(IGTHardwareException) << "Could not initialize the tracking device";}

	if (this->GetType() == TrackingSystemNotSpecified)  // if the type of tracking device is not specified, try to query the connected device
	{
		TrackingDeviceType deviceType;
		returnvalue = m_DeviceProtocol->VER(deviceType);
		if ((returnvalue != NDIOKAY) || (deviceType == TrackingSystemNotSpecified))
		{vpsThrowException(IGTHardwareException) << "Could not determine tracking device type. Please set manually and try again.";}
		this->SetType(deviceType);
	}

	/****  Optional Polaris specific code, Work in progress
	// start diagnostic mode
	returnvalue = m_DeviceProtocol->DSTART();
	if (returnvalue != NDIOKAY)
	{
	this->SetErrorMessage("Could not start diagnostic mode");
	return false;
	}
	else    // we are in diagnostic mode
	{
	// initialize extensive IR checking
	returnvalue = m_DeviceProtocol->IRINIT();
	if (returnvalue != NDIOKAY)
	{
	this->SetErrorMessage("Could not initialize intense infrared light checking");
	return false;
	}
	bool intenseIR = false;
	returnvalue = m_DeviceProtocol->IRCHK(&intenseIR);
	if (returnvalue != NDIOKAY)
	{
	this->SetErrorMessage("Could not execute intense infrared light checking");
	return false;
	}
	if (intenseIR == true)
	// do something - warn the user, raise exception, write to protocol or similar
	std::cout << "Warning: Intense infrared light detected. Accurate tracking will probably not be possible.\n";

	// stop diagnictic mode
	returnvalue = m_DeviceProtocol->DSTOP();
	if (returnvalue != NDIOKAY)
	{
	this->SetErrorMessage("Could not stop diagnostic mode");
	return false;
	}
	}
	*** end of optional polaris code ***/

	/**
	* now add tools to the tracking system
	**/

	/* First, check if the tracking device has port handles that need to be freed and free them */
	returnvalue = FreePortHandles();
	// non-critical, therefore no error handling

	/**
	* POLARIS: initialize the tools that were added manually
	**/
	{

		MutexLockHolder toolsMutexLockHolder(*m_ToolsMutex); // lock and unlock the mutex
		std::string portHandle;
		Tool6DContainerType::iterator endIt = m_6DTools.end();
		for(Tool6DContainerType::iterator it = m_6DTools.begin(); it != endIt; ++it)
		{
			/* get a port handle for the tool */
			returnvalue = m_DeviceProtocol->PHRQ(&portHandle);
			if (returnvalue == NDIOKAY)
			{
				(*it)->SetPortHandle(portHandle.c_str());
				/* now write the SROM file of the tool to the tracking system using PVWR */
				if (this->m_Data.Line == NDIPolaris)
				{
					returnvalue = m_DeviceProtocol->PVWR(&portHandle, (*it)->GetSROMData(), (*it)->GetSROMDataLength());
					if (returnvalue != NDIOKAY)
					{vpsThrowException(IGTHardwareException) << (std::string("Could not write SROM file for tool '") + (*it)->GetToolName() + std::string("' to tracking device")).c_str();}

					returnvalue = m_DeviceProtocol->PINIT(&portHandle);
					if (returnvalue != NDIOKAY)
					{vpsThrowException(IGTHardwareException) << (std::string("Could not initialize tool '") + (*it)->GetToolName()).c_str();}

					if ((*it)->IsEnabled() == true)
					{
						returnvalue = m_DeviceProtocol->PENA(&portHandle, (*it)->GetTrackingPriority()); // Enable tool
						if (returnvalue != NDIOKAY)
						{
							vpsThrowException(IGTHardwareException) << (std::string("Could not enable port '") + portHandle +
								std::string("' for tool '")+ (*it)->GetToolName() + std::string("'")).c_str();
						}
					}
				}
			}
		}
	} // end of toolsmutexlockholder scope

	/* check for wired tools and add them too */
	if (this->DiscoverWiredTools() == false)  // query the tracking device for wired tools and add them to our tool list
		return false; // \TODO: could we continue anyways?


	/*POLARIS: set the illuminator activation rate */
	if (this->m_Data.Line == NDIPolaris)
	{
		returnvalue = m_DeviceProtocol->IRATE(this->m_IlluminationActivationRate);
		if (returnvalue != NDIOKAY)
		{vpsThrowException(IGTHardwareException) << "Could not set the illuminator activation rate";}
	}
	/* finish  - now all tools should be added, initialized and enabled, so that tracking can be started */
	this->SetState(Ready);
	try
	{
		SetVolume(this->m_Data);
	}
	catch (IGTHardwareException e)
	{
		std::cout <<e.GetDescription();
	}

	return true;
}


TrackingDeviceType NDITrackingDevice::TestConnection()
{
	if (this->GetState() != Setup)
	{
		return TrackingSystemNotSpecified;
	}

	m_SerialCommunication = SerialCommunication::New();
	//m_DeviceProtocol =  NDIProtocol::New();
	//m_DeviceProtocol->SetTrackingDevice(this);
	//m_DeviceProtocol->UseCRCOn();
	/* init local com port to standard com settings for a NDI tracking device:
	9600 baud, 8 data bits, no parity, 1 stop bit, no hardware handshake
	*/
	if (m_DeviceName.empty())
		m_SerialCommunication->SetPortNumber(m_PortNumber);
	else
		m_SerialCommunication->SetDeviceName(m_DeviceName);

	m_SerialCommunication->SetBaudRate(SerialCommunication::BaudRate9600);
	m_SerialCommunication->SetDataBits(SerialCommunication::DataBits8);
	m_SerialCommunication->SetParity(SerialCommunication::None);
	m_SerialCommunication->SetStopBits(SerialCommunication::StopBits1);
	m_SerialCommunication->SetSendTimeout(5000);
	m_SerialCommunication->SetReceiveTimeout(5000);
	if (m_SerialCommunication->OpenConnection() == 0) // error
	{
		m_SerialCommunication = NULL;
		return TrackingSystemNotSpecified;
	}

	/* Reset Tracking device by sending a serial break for 500ms */
	m_SerialCommunication->SendBreak(400);

	/* Read answer from tracking device (RESETBE6F) */
	static const std::string reset("RESETBE6F\r");
	std::string answer = "";
	this->Receive(&answer, reset.length());  // read answer (should be RESETBE6F)
	this->ClearReceiveBuffer();     // flush the receive buffer of all remaining data (carriage return, strings other than reset
	if (reset.compare(answer) != 0)  // check for RESETBE6F
	{
		m_SerialCommunication->CloseConnection();
		m_SerialCommunication = NULL;
		vpsThrowException(IGTHardwareException) << "Hardware Reset of tracking device did not work";
	}

	/* Now the tracking device is reset, start initialization */
	NDIErrorCode returnvalue;

	TrackingDeviceType deviceType;
	returnvalue = m_DeviceProtocol->VER(deviceType);
	if ((returnvalue != NDIOKAY) || (deviceType == TrackingSystemNotSpecified))
	{
		return TrackingSystemNotSpecified;
	}
	m_SerialCommunication = NULL;

	return deviceType;
}

bool NDITrackingDevice::CloseConnection()
{
	if (this->GetState() != Setup)
	{
		//init before closing to force the field generator from aurora to switch itself off
		m_DeviceProtocol->INIT();
		/* close the serial connection */
		m_SerialCommunication->CloseConnection();
		/* invalidate all tools */
		this->InvalidateAll();
		/* return to setup mode */
		this->SetState(Setup);

	}
	return true;
}


ITK_THREAD_RETURN_TYPE NDITrackingDevice::ThreadStartTracking(void* pInfoStruct)
{
	/* extract this pointer from Thread Info structure */
	struct itk::MultiThreader::ThreadInfoStruct * pInfo = (struct itk::MultiThreader::ThreadInfoStruct*)pInfoStruct;
	if (pInfo == NULL)
	{
		return ITK_THREAD_RETURN_VALUE;
	}
	if (pInfo->UserData == NULL)
	{
		return ITK_THREAD_RETURN_VALUE;
	}
	NDITrackingDevice *trackingDevice = (NDITrackingDevice*)pInfo->UserData;
	if (trackingDevice != NULL)
	{
		if (trackingDevice->GetOperationMode() == ToolTracking6D)
			trackingDevice->TrackTools();             // call TrackTools() from the original object
		else if (trackingDevice->GetOperationMode() == MarkerTracking3D)
			trackingDevice->TrackMarkerPositions();   // call TrackMarkerPositions() from the original object
		else if (trackingDevice->GetOperationMode() == ToolTracking5D)
			trackingDevice->TrackMarkerPositions(); // call TrackMarkerPositions() from the original object
		else if (trackingDevice->GetOperationMode() == HybridTracking)
		{
			trackingDevice->TrackToolsAndMarkers();
		}
	}
	trackingDevice->m_ThreadID = 0;  // erase thread id, now that this thread will end.
	return ITK_THREAD_RETURN_VALUE;
}


bool NDITrackingDevice::StartTracking()
{
	if (this->GetState() != Ready)
		return false;

	this->SetState(Tracking);      // go to mode Tracking
	this->m_StopTrackingMutex->Lock();  // update the local copy of m_StopTracking
	this->m_StopTracking = false;
	this->m_StopTrackingMutex->Unlock();

	m_TrackingFinishedMutex->Unlock(); // transfer the execution rights to tracking thread

	m_ThreadID = m_MultiThreader->SpawnThread(this->ThreadStartTracking, this);    // start a new thread that executes the TrackTools() method
	IGTTimeStamp::GetInstance()->Start(this);
	return true;
}


void NDITrackingDevice::TrackTools()
{
	if (this->GetState() != Tracking)
		return;

	NDIErrorCode returnvalue;
	returnvalue = m_DeviceProtocol->TSTART();
	if (returnvalue != NDIOKAY)
		return;

	/* lock the TrackingFinishedMutex to signal that the execution rights are now transfered to the tracking thread */
	MutexLockHolder trackingFinishedLockHolder(*m_TrackingFinishedMutex); // keep lock until end of scope

	bool localStopTracking;       // Because m_StopTracking is used by two threads, access has to be guarded by a mutex. To minimize thread locking, a local copy is used here
	this->m_StopTrackingMutex->Lock();  // update the local copy of m_StopTracking
	localStopTracking = this->m_StopTracking;
	this->m_StopTrackingMutex->Unlock();
	while ((this->GetState() == Tracking) && (localStopTracking == false))
	{
		if (this->m_DataTransferMode == TX)
		{
			returnvalue = this->m_DeviceProtocol->TX();
			if (!((returnvalue == NDIOKAY) || (returnvalue == NDICRCERROR) || (returnvalue == NDICRCDOESNOTMATCH))) // right now, do not stop on crc errors
				break;
		}
		else
		{
			returnvalue = this->m_DeviceProtocol->BX();
			if (returnvalue != NDIOKAY)
				break;
		}
		/* Update the local copy of m_StopTracking */
		this->m_StopTrackingMutex->Lock();
		localStopTracking = m_StopTracking;
		this->m_StopTrackingMutex->Unlock();
	}
	/* StopTracking was called, thus the mode should be changed back to Ready now that the tracking loop has ended. */

	returnvalue = m_DeviceProtocol->TSTOP();
	if (returnvalue != NDIOKAY)
	{vpsThrowException(IGTHardwareException) << "An error occured while tracking tools.";}

	return;       // returning from this function (and ThreadStartTracking()) this will end the thread and transfer control back to main thread by releasing trackingFinishedLockHolder
}


void NDITrackingDevice::TrackMarkerPositions()
{
	if (m_OperationMode == ToolTracking6D)
		return;

	if (this->GetState() != Tracking)
		return;

	NDIErrorCode returnvalue;

	returnvalue = m_DeviceProtocol->DSTART();   // Start Diagnostic Mode
	if (returnvalue != NDIOKAY)
		return;

	MutexLockHolder trackingFinishedLockHolder(*m_TrackingFinishedMutex); // keep lock until end of scope

	bool localStopTracking;       // Because m_StopTracking is used by two threads, access has to be guarded by a mutex. To minimize thread locking, a local copy is used here
	this->m_StopTrackingMutex->Lock();  // update the local copy of m_StopTracking
	localStopTracking = this->m_StopTracking;
	this->m_StopTrackingMutex->Unlock();
	while ((this->GetState() == Tracking) && (localStopTracking == false))
	{
		m_MarkerPointsMutex->Lock();                                    // lock points data structure
		returnvalue = this->m_DeviceProtocol->POS3D(&m_MarkerPoints); // update points data structure with new position data from tracking device
		m_MarkerPointsMutex->Unlock();
		if (!((returnvalue == NDIOKAY) || (returnvalue == NDICRCERROR) || (returnvalue == NDICRCDOESNOTMATCH))) // right now, do not stop on crc errors
		{
			std::cout << "Error in POS3D: could not read data. Possibly no markers present." << std::endl;
		}
		/* Update the local copy of m_StopTracking */
		this->m_StopTrackingMutex->Lock();
		localStopTracking = m_StopTracking;
		this->m_StopTrackingMutex->Unlock();

		itksys::SystemTools::Delay(1);
	}
	/* StopTracking was called, thus the mode should be changed back to Ready now that the tracking loop has ended. */
	returnvalue = m_DeviceProtocol->DSTOP();
	if (returnvalue != NDIOKAY)
		return;     // how can this thread tell the application, that an error has occured?

	this->SetState(Ready);
	return;       // returning from this function (and ThreadStartTracking()) this will end the thread
}


void NDITrackingDevice::TrackToolsAndMarkers()
{
	if (m_OperationMode != HybridTracking)
		return;

	NDIErrorCode returnvalue;

	returnvalue = m_DeviceProtocol->TSTART();   // Start Diagnostic Mode
	if (returnvalue != NDIOKAY)
		return;

	MutexLockHolder trackingFinishedLockHolder(*m_TrackingFinishedMutex); // keep lock until end of scope

	bool localStopTracking;       // Because m_StopTracking is used by two threads, access has to be guarded by a mutex. To minimize thread locking, a local copy is used here
	this->m_StopTrackingMutex->Lock();  // update the local copy of m_StopTracking
	localStopTracking = this->m_StopTracking;
	this->m_StopTrackingMutex->Unlock();
	while ((this->GetState() == Tracking) && (localStopTracking == false))
	{
		m_MarkerPointsMutex->Lock();                                     // lock points data structure
		returnvalue = this->m_DeviceProtocol->TX(true, &m_MarkerPoints); // update points data structure with new position data from tracking device
		m_MarkerPointsMutex->Unlock();
		if (!((returnvalue == NDIOKAY) || (returnvalue == NDICRCERROR) || (returnvalue == NDICRCDOESNOTMATCH))) // right now, do not stop on crc errors
		{
			std::cout << "Error in TX: could not read data. Possibly no markers present." << std::endl;
		}
		/* Update the local copy of m_StopTracking */
		this->m_StopTrackingMutex->Lock();
		localStopTracking = m_StopTracking;
		this->m_StopTrackingMutex->Unlock();
	}
	/* StopTracking was called, thus the mode should be changed back to Ready now that the tracking loop has ended. */

	returnvalue = m_DeviceProtocol->TSTOP();
	if (returnvalue != NDIOKAY)
		return;     // how can this thread tell the application, that an error has occurred?

	this->SetState(Ready);
	return;       // returning from this function (and ThreadStartTracking()) this will end the thread
}


TrackingTool* NDITrackingDevice::GetTool(unsigned int toolNumber) const
{
	MutexLockHolder toolsMutexLockHolder(*m_ToolsMutex); // lock and unlock the mutex
	if (toolNumber < m_6DTools.size())
		return m_6DTools.at(toolNumber);
	return NULL;
}


TrackingTool* NDITrackingDevice::GetToolByName(std::string name) const
{
	MutexLockHolder toolsMutexLockHolder(*m_ToolsMutex); // lock and unlock the mutex
	Tool6DContainerType::const_iterator end = m_6DTools.end();
	for (Tool6DContainerType::const_iterator iterator = m_6DTools.begin(); iterator != end; ++iterator)
		if (name.compare((*iterator)->GetToolName()) == 0)
			return *iterator;
	return NULL;
}


NDIPassiveTool* NDITrackingDevice::GetInternalTool(std::string portHandle)
{
	MutexLockHolder toolsMutexLockHolder(*m_ToolsMutex); // lock and unlock the mutex
	Tool6DContainerType::iterator end = m_6DTools.end();
	for (Tool6DContainerType::iterator iterator = m_6DTools.begin(); iterator != end; ++iterator)
		if (portHandle.compare((*iterator)->GetPortHandle()) == 0)
			return *iterator;
	return NULL;
}


unsigned int NDITrackingDevice::GetToolCount() const
{
	MutexLockHolder toolsMutexLockHolder(*m_ToolsMutex); // lock and unlock the mutex
	return m_6DTools.size();
}


bool NDITrackingDevice::Beep(unsigned char count)
{
	if (this->GetState() != Setup)
	{
		return (m_DeviceProtocol->BEEP(count) == NDIOKAY);
	}
	else
	{
		return false;
	}
}

TrackingTool* NDITrackingDevice::AddTool( const char* toolName, const char* fileName, TrackingPriority p /*= NDIPassiveTool::Dynamic*/ )
{
	NDIPassiveTool::Pointer t = NDIPassiveTool::New();
	if (t->LoadSROMFile(fileName) == false)
		return NULL;
	t->SetToolName(toolName);
	t->SetTrackingPriority(p);
	if (this->InternalAddTool(t) == false)
		return NULL;
	return t.GetPointer();
}


bool NDITrackingDevice::InternalAddTool(NDIPassiveTool* tool)
{
	if (tool == NULL)
		return false;
	NDIPassiveTool::Pointer p = tool;
	/* if the connection to the tracking device is already established, add the new tool to the device now */
	if (this->GetState() == Ready)
	{
		/* get a port handle for the tool */
		std::string newPortHandle;
		NDIErrorCode returnvalue;
		returnvalue = m_DeviceProtocol->PHRQ(&newPortHandle);
		if (returnvalue == NDIOKAY)
		{
			p->SetPortHandle(newPortHandle.c_str());
			/* now write the SROM file of the tool to the tracking system using PVWR */
			returnvalue = m_DeviceProtocol->PVWR(&newPortHandle, p->GetSROMData(), p->GetSROMDataLength());
			if (returnvalue != NDIOKAY)
			{vpsThrowException(IGTHardwareException) << (std::string("Could not write SROM file for tool '") + p->GetToolName() + std::string("' to tracking device")).c_str();}
			/* initialize the port handle */
			returnvalue = m_DeviceProtocol->PINIT(&newPortHandle);
			if (returnvalue != NDIOKAY)
			{
				vpsThrowException(IGTHardwareException) << (std::string("Could not initialize port '") + newPortHandle +
					std::string("' for tool '")+ p->GetToolName() + std::string("'")).c_str();
			}
			/* enable the port handle */
			if (p->IsEnabled() == true)
			{
				returnvalue = m_DeviceProtocol->PENA(&newPortHandle, p->GetTrackingPriority()); // Enable tool
				if (returnvalue != NDIOKAY)
				{
					vpsThrowException(IGTHardwareException) << (std::string("Could not enable port '") + newPortHandle +
						std::string("' for tool '")+ p->GetToolName() + std::string("'")).c_str();
				}
			}
		}
		/* now that the tool is added to the device, add it to list too */
		m_ToolsMutex->Lock();
		this->m_6DTools.push_back(p);
		m_ToolsMutex->Unlock();
		this->Modified();
		return true;
	}
	else if (this->GetState() == Setup)
	{
		/* In Setup mode, we only add it to the list, so that OpenConnection() can add it later */
		m_ToolsMutex->Lock();
		this->m_6DTools.push_back(p);
		m_ToolsMutex->Unlock();
		this->Modified();
		return true;
	}
	else  // in Tracking mode, no tools can be added
		return false;
}


bool NDITrackingDevice::RemoveTool(TrackingTool* tool)
{
	NDIPassiveTool* ndiTool = dynamic_cast<NDIPassiveTool*>(tool);
	if (ndiTool == NULL)
		return false;

	std::string portHandle = ndiTool->GetPortHandle();
	/* a valid portHandle has length 2. If a valid handle exists, the tool is already added to the tracking device, so we have to remove it there
	if the connection to the tracking device has already been established.
	*/
	if ((portHandle.length() == 2) && (this->GetState() == Ready))  // do not remove a tool in tracking mode
	{
		NDIErrorCode returnvalue;
		returnvalue = m_DeviceProtocol->PHF(&portHandle);
		if (returnvalue != NDIOKAY)
			return false;
		/* Now that the tool is removed from the tracking device, remove it from our tool list too */
		MutexLockHolder toolsMutexLockHolder(*m_ToolsMutex); // lock and unlock the mutex (scope is inside the if-block
		Tool6DContainerType::iterator end = m_6DTools.end();
		for (Tool6DContainerType::iterator iterator = m_6DTools.begin(); iterator != end; ++iterator)
		{
			if (iterator->GetPointer() == ndiTool)
			{
				m_6DTools.erase(iterator);
				this->Modified();
				return true;
			}
		}
		return false;
	}
	else if (this->GetState() == Setup)  // in Setup Mode, we are not connected to the tracking device, so we can just remove the tool from the tool list
	{
		MutexLockHolder toolsMutexLockHolder(*m_ToolsMutex); // lock and unlock the mutex
		Tool6DContainerType::iterator end = m_6DTools.end();
		for (Tool6DContainerType::iterator iterator = m_6DTools.begin(); iterator != end; ++iterator)
		{
			if ((*iterator).GetPointer() == ndiTool)
			{
				m_6DTools.erase(iterator);
				this->Modified();
				return true;
			}
		}
		return false;
	}
	return false;
}


void NDITrackingDevice::InvalidateAll()
{
	MutexLockHolder toolsMutexLockHolder(*m_ToolsMutex); // lock and unlock the mutex
	Tool6DContainerType::iterator end = m_6DTools.end();
	for (Tool6DContainerType::iterator iterator = m_6DTools.begin(); iterator != end; ++iterator)
		(*iterator)->SetDataValid(false);
}


bool NDITrackingDevice::SetOperationMode(OperationMode mode)
{
	if (GetState() == Tracking)
		return false;

	m_OperationMode = mode;
	return true;
}


OperationMode NDITrackingDevice::GetOperationMode()
{
	return m_OperationMode;
}


bool NDITrackingDevice::GetMarkerPositions(MarkerPointContainerType* markerpositions)
{
	m_MarkerPointsMutex->Lock();
	*markerpositions = m_MarkerPoints;  // copy the internal vector to the one provided
	m_MarkerPointsMutex->Unlock();
	return (markerpositions->size() != 0)  ;
}


bool NDITrackingDevice::DiscoverWiredTools()
{
	/* First, check for disconnected tools and remove them */
	this->FreePortHandles();

	/* check for new tools, add and initialize them */
	NDIErrorCode returnvalue;
	std::string portHandle;
	returnvalue = m_DeviceProtocol->PHSR(OCCUPIED, &portHandle);

	if (returnvalue != NDIOKAY)
	{vpsThrowException(IGTHardwareException) << "Could not obtain a list of port handles that are connected";}

	/* if there are port handles that need to be initialized, initialize them. Furthermore instantiate tools for each handle that has no tool yet. */
	std::string ph;

	/* we need to remember the ports which are occupied to be able to readout the serial numbers of the connected tools later */
	std::vector<int> occupiedPorts = std::vector<int>();
	int numberOfToolsAtStart = this->GetToolCount(); //also remember the number of tools at start to identify the automatically detected tools later

	for (unsigned int i = 0; i < portHandle.size(); i += 2)
	{
		ph = portHandle.substr(i, 2);
		if (this->GetInternalTool(ph) != NULL) // if we already have a tool with this handle
			continue;                            // then skip the initialization

		//instantiate an object for each tool that is connected
		NDIPassiveTool::Pointer newTool = NDIPassiveTool::New();
		newTool->SetPortHandle(ph.c_str());
		newTool->SetTrackingPriority(NDIPassiveTool::Dynamic);

		//set a name for identification
		newTool->SetToolName((std::string("Port ") + ph).c_str());

		returnvalue = m_DeviceProtocol->PINIT(&ph);
		if (returnvalue != NDIINITIALIZATIONFAILED) //if the initialization failed (AURORA) it can not be enabled. A srom file will have to be specified manually first. Still return true to be able to continue
		{
			if (returnvalue != NDIOKAY)
			{
				vpsThrowException(IGTHardwareException) << (std::string("Could not initialize port '") + ph +
					std::string("' for tool '")+ newTool->GetToolName() + std::string("'")).c_str();
			}
			/* enable the port handle */
			returnvalue = m_DeviceProtocol->PENA(&ph, newTool->GetTrackingPriority()); // Enable tool
			if (returnvalue != NDIOKAY)
			{
				vpsThrowException(IGTHardwareException) << (std::string("Could not enable port '") + ph +
					std::string("' for tool '")+ newTool->GetToolName() + std::string("'")).c_str();
			}
		}
		//we have to temporarily unlock m_ModeMutex here to avoid a deadlock with another lock inside InternalAddTool()
		if (this->InternalAddTool(newTool) == false)
		{vpsThrowException(IGTException) << "Error while adding new tool";}
		else occupiedPorts.push_back(i);
	}


	// after initialization readout serial numbers of automatically detected tools
	for (unsigned int i = 0; i < occupiedPorts.size(); i++)
	{
		ph = portHandle.substr(occupiedPorts.at(i), 2);
		std::string portInfo;
		NDIErrorCode returnvaluePort = m_DeviceProtocol->PHINF(ph, &portInfo);
		if ((returnvaluePort==NDIOKAY) && (portInfo.size()>31)) dynamic_cast<NDIPassiveTool*>(this->GetTool(i+numberOfToolsAtStart))->SetSerialNumber(portInfo.substr(23,8));
		itksys::SystemTools::Delay(10);
	}

	return true;
}


NDIErrorCode NDITrackingDevice::FreePortHandles()
{
	/*  first search for port handles that need to be freed: e.g. because of a reset of the tracking system */
	NDIErrorCode returnvalue = NDIOKAY;
	std::string portHandle;
	returnvalue = m_DeviceProtocol->PHSR(FREED, &portHandle);
	if (returnvalue != NDIOKAY)
	{vpsThrowException(IGTHardwareException) << "Could not obtain a list of port handles that need to be freed";}

	/* if there are port handles that need to be freed, free them */
	if (portHandle.empty() == true)
		return returnvalue;

	std::string ph;
	for (unsigned int i = 0; i < portHandle.size(); i += 2)
	{
		ph = portHandle.substr(i, 2);

		NDIPassiveTool* t = this->GetInternalTool(ph);
		if (t != NULL)  // if we have a tool for the port handle that needs to be freed
		{
			if (this->RemoveTool(t) == false)  // remove it (this will free the port too)
				returnvalue = NDIERROR;
		}
		else  // we don't have a tool, the port handle exists only in the tracking device
		{
			returnvalue = m_DeviceProtocol->PHF(&ph);  // free it there
			// What to do if port handle could not be freed? This seems to be a non critical error
			if (returnvalue != NDIOKAY)
			{vpsThrowException(IGTHardwareException) << "Could not free all port handles";}
		}
	}
	return returnvalue;
}


int NDITrackingDevice::GetMajorFirmwareRevisionNumber()
{
	std::string revision;
	if (m_DeviceProtocol->APIREV(&revision) != NDIOKAY || revision.empty() || (revision.size() != 9) )
	{
		std::cout << "Could not receive firmware revision number!";
		return 0;
	}

	const std::string majrevno = revision.substr(2,3); //cut out "004" from "D.004.001"

	return std::atoi(majrevno.c_str());
}

const char* NDITrackingDevice::GetFirmwareRevisionNumber()
{
	static std::string revision;
	if (m_DeviceProtocol->APIREV(&revision) != NDIOKAY || revision.empty() || (revision.size() != 9) )
	{
		std::cout << "Could not receive firmware revision number!";
		revision = "";
		return revision.c_str();
	}
	return revision.c_str();
}

bool NDITrackingDevice::GetSupportedVolumes(unsigned int* numberOfVolumes, NDITrackingDevice::NDITrackingVolumeContainerType* volumes, NDITrackingDevice::TrackingVolumeDimensionType* volumesDimensions)
{
	if (numberOfVolumes == NULL || volumes == NULL || volumesDimensions == NULL)
		return false;

	static std::string info;
	if (m_DeviceProtocol->SFLIST(&info) != NDIOKAY || info.empty())
	{
		std::cout << "Could not receive tracking volume information of tracking system!";
		return false;
	}

	/*info contains the following:
	<HEX:number of volumes> (+n times:) <HEX:shape type> <shape parameters D1-D10> <HEX:reserved / number of wavelength supported> <metal resistant / supported wavelength>
	*/
	(*numberOfVolumes) = (unsigned int) std::atoi(info.substr(0,1).c_str());

	for (unsigned int i=0; i<(*numberOfVolumes); i++)
	{
		//e.g. for cube:  "9-025000+025000-025000+025000-055000-005000+000000+000000+000000+00000011"
		//for dome:       "A+005000+048000+005000+066000+000000+000000+000000+000000+000000+00000011"

		std::string::size_type offset, end;
		offset = (i*73)+1;
		end = 73+(i*73);
		std::string currentVolume = info.substr(offset, end);//i=0: from 1 to 73 characters; i=1: from 75 to 148 char;
		// if i>0 then we have a return statement <LF> infront
		if (i>0)
			currentVolume = currentVolume.substr(1, currentVolume.size());
		if (currentVolume.compare(0,1,DeviceDataPolarisOldModel.HardwareCode)==0)
			volumes->push_back(DeviceDataPolarisOldModel.Model);
		if (currentVolume.compare(0,3,DeviceDataPolarisSpectra.HardwareCode)==0)
			volumes->push_back(DeviceDataPolarisSpectra.Model);
		if (currentVolume.compare(1,3,DeviceDataSpectraExtendedPyramid.HardwareCode)==0)
		{
			currentVolume = currentVolume.substr(1,currentVolume.size());
			volumes->push_back(DeviceDataSpectraExtendedPyramid.Model);
		}
		if (currentVolume.compare(0,1,DeviceDataPolarisVicra.HardwareCode)==0)
			volumes->push_back(DeviceDataPolarisVicra.Model);
		else if (currentVolume.compare(0,1,DeviceDataAuroraPlanarCube.HardwareCode)==0)
			volumes->push_back(DeviceDataAuroraPlanarCube.Model);//alias cube
		else if (currentVolume.compare(0,1,DeviceDataAuroraPlanarDome.HardwareCode)==0)
			volumes->push_back(DeviceDataAuroraPlanarDome.Model);

		//fill volumesDimensions
		for (unsigned int index = 0; index < 10; index++)
		{
			std::string::size_type offD, endD;
			offD = 1+(index*7); //7 digits per dimension and the first is the type of volume
			endD = offD+7;
			int dimension = std::atoi(currentVolume.substr(offD, endD).c_str());
			dimension /= 100; //given in mm. 7 digits are xxxx.xx according to NDI //strange, the last two digits (11) also for the metal flag get read also...
			volumesDimensions->push_back(dimension);
		}
	}

	return true;
}

bool NDITrackingDevice::SetVolume(TrackingDeviceData volume)
{
	if (m_DeviceProtocol->VSEL(volume) != NDIOKAY)
	{
		vpsThrowException(IGTHardwareException) << "Could not set volume!";
	}
	return true;
}

