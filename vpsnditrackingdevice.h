#ifndef NDITRACKINGDEVICE_H_HEADER_INCLUDED_C1C2FCD2
#define NDITRACKINGDEVICE_H_HEADER_INCLUDED_C1C2FCD2

#include "trackingdevice.h"
#include <vector>

#include "trackingtypes.h"
#include "vpsndiprotocol.h"
#include "vpsndipassivetool.h"
#include "serialworker.h"

class NDIProtocol;

/** Documentation
* \brief superclass for specific NDI tracking Devices that use serial communication.
*
* implements the TrackingDevice interface for NDI tracking devices (POLARIS, AURORA)
*
* \ingroup IGT
*/
class NDITrackingDevice : public TrackingDevice
{
	friend class NDIProtocol;

public:
    ///< List of 6D tools of the correct type for this tracking device
    typedef std::vector<NDIPassiveTool::Pointer> Tool6DContainerType;
    ///< This enumeration includes the two types of NDI tracking devices (Polaris, Aurora).
    typedef TrackingDeviceType NDITrackingDeviceType;
    typedef QSerialPort::BaudRate BaudRate;     ///< Baud rate of the serial connection
    typedef QSerialPort::DataBits DataBits;     ///< Number of data bits used in the serial connection
    typedef QSerialPort::Parity Parity;         ///< Parity mode used in the serial connection
    typedef QSerialPort::StopBits StopBits;     ///< Number of stop bits used in the serial connection
    typedef QSerialPort::FlowControl  FlowControl; ///< Hardware handshake mode of the serial connection
	typedef NDIPassiveTool::TrackingPriority TrackingPriority; ///< Tracking priority used for tracking a tool

	/**
	* \brief initialize the connection to the tracking device
	*
	* OpenConnection() establishes the connection to the tracking device by:
	* - initializing the serial port with the given parameters (port number, baud rate, ...)
	* - connection to the tracking device
	* - initializing the device
	* - initializing all manually added passive tools (user supplied srom file)
	* - initializing active tools that are connected to the tracking device
	* @throw IGTHardwareException Throws an exception if there are errors while connecting to the device.
    * @throw IGTException Throws a normal IGT exception if an error occures which is not related to the
    *                       hardware.
	*/
	virtual bool OpenConnection();

	/**
	* \brief Closes the connection
	*
	* CloseConnection() resets the tracking device, invalidates all tools and then closes the serial port.
	*/
	virtual bool CloseConnection();

	/** @throw IGTHardwareException Throws an exception if there are errors while connecting to the device. */
	bool InitializeWiredTools();

	/** Sets the rotation mode of this class. See documentation of enum RotationMode for details
	*  on the different modes.
	*/
	virtual void SetRotationMode(RotationMode r);

	/**
    * \brief TestConnection() tries to connect to a NDI tracking device on the current port/device
    *           and returns which device it has found
	*
	* TestConnection() tries to connect to a NDI tracking device on the current port/device.
    * \return It returns the type of the device that answers at the port/device.
    *           Throws an exception if no device is available on that port.
	* @throw IGTHardwareException Throws an exception if there are errors while connecting to the device.
	*/
	virtual TrackingDeviceType TestConnection();

	/**
	* \brief retrieves all wired tools from the tracking device
	*
    * This method queries the tracking device for all wired tools, initializes them and creates
    * TrackingTool representation objects for them
	* \return True if the method was executed successful.
	* @throw IGTHardwareException Throws an exception if there are errors while connecting to the device.
    * @throw IGTException Throws a normal IGT exception if an error occures which is not related to the
    *                        hardware.
	*/
	bool DiscoverWiredTools();

	/**
	* \brief Start the tracking.
	*
	* A new thread is created, which continuously reads the position and orientation information of each tool and stores them inside the tools.
	* Depending on the current operation mode (see SetOperationMode()), either the 6D tools (ToolTracking6D), 5D tools (ToolTracking5D),
	* 3D marker positions (MarkerTracking3D) or both 6D tools and 3D markers (HybridTracking) are updated.
	* Call StopTracking() to stop the tracking thread.
	*/
	virtual bool StartTracking();

    ///< return the tool with index toolNumber
	virtual TrackingTool* GetTool(unsigned int toolNumber) const;

	virtual TrackingTool* GetToolByName(std::string name) const;

    ///< return current number of tools
    virtual unsigned int GetToolCount() const;

	/**
	* \brief Create a passive 6D tool with toolName and fileName and add it to the list of tools
	*
	* This method will create a new NDIPassiveTool object, load the SROM file fileName,
	* set the tool name toolName and the tracking priority p and then add
	* it to the list of tools. It returns a pointer of type TrackingTool to the tool
	* that can be used to read tracking data from it.
	* This is the only way to add tools to NDITrackingDevice.
	* @throw IGTHardwareException Throws an exception if there are errors while adding the tool.
	*
	* \warning adding tools is not possible in tracking mode, only in setup and ready.
	*/
	TrackingTool* AddTool(const char* toolName, const char* fileName, TrackingPriority p = NDIPassiveTool::Dynamic);

    ///< Remove a passive 6D tool from the list of tracked tools.
    ///<warning removing tools is not possible in tracking mode, only in setup and ready modes.
	virtual bool RemoveTool(TrackingTool* tool);

    ///< reloads the srom file and reinitializes the tool
	virtual bool UpdateTool(TrackingTool* tool);

    ///< set device name (e.g. COM1, /dev/ttyUSB0). If this is set, PortNumber will be ignored
    virtual void SetDeviceName(std::string _arg);

    ///< set baud rate for serial communication
    virtual void SetBaudRate(const BaudRate _arg);

    ///< set number of data bits
    virtual void SetDataBits(const DataBits _arg);

    ///< set parity mode
    virtual void SetParity(const Parity _arg);

    ///< set number of stop bits
    virtual void SetStopBits(const StopBits _arg);

    ///< set use hardware handshake for serial communication
    virtual void SetHardwareHandshake(const FlowControl _arg);

    ///< set activation rate of IR illumator for polaris
    virtual void SetIlluminationActivationRate(const IlluminationActivationRate _arg);

    ///< set data transfer mode to text (TX) or binary (BX). \warning: only TX is supportet at the moment
    virtual void SetDataTransferMode(const DataTransferMode _arg);

    ///< Beep the tracking device 1 to 9 times
    virtual bool Beep(unsigned char count);

    ///< returns the error code for a string that contains an error code in hexadecimal format
    NDIErrorCode GetErrorCode(const std::string* input);

    ///< set operation mode to 6D tool tracking, 3D marker tracking or 6D&3D hybrid tracking
    virtual bool SetOperationMode(OperationMode mode);

    ///< get current operation mode
    virtual OperationMode GetOperationMode();

    /// Get 3D marker positions (operation mode must be set to MarkerTracking3D or HybridTracking)
	virtual bool GetMarkerPositions(MarkerPointContainerType* markerpositions);

    ///Get major revision number from tracking device
    ///should not be called directly after starting to track
	virtual int GetMajorFirmwareRevisionNumber();

    ///Get revision number from tracking device as string
    ///should not be called directly after starting to track
	virtual const char* GetFirmwareRevisionNumber();

protected:

	typedef std::vector<std::string> NDITrackingVolumeContainerType;  ///< vector of tracking volumes
	typedef std::vector<int> TrackingVolumeDimensionType;          ///< List of the supported tracking volume dimensions.

	/**
	* \brief Get number of supported tracking volumes, a vector containing the supported volumes and
	* a vector containing the signed dimensions in mm. For each volume 10 boundaries are stored in the order of
	* the supported volumes (see AURORA API GUIDE: SFLIST p.54).
	**/
	virtual bool GetSupportedVolumes(unsigned int* numberOfVolumes, NDITrackingVolumeContainerType* volumes, TrackingVolumeDimensionType* volumesDimensions);

	/**
	* \brief Sets the desired tracking volume. Returns true if the volume type could be set. It is set in the OpenConnection() Method and sets the tracking volume out of m_Data.
	* @throw IGTHardwareException Throws an IGT hardware exception if the volume could not be set.
	**/
	virtual bool SetVolume(TrackingDeviceData volume);

	/**
	* \brief Add a passive 6D tool to the list of tracked tools. This method is used by AddTool
	* @throw IGTHardwareException Throws an exception if there are errors while adding the tool.
	* \warning adding tools is not possible in tracking mode, only in setup and ready.
	*/
	virtual bool InternalAddTool(NDIPassiveTool* tool);

	/* Methods for NDIProtocol friend class */
	virtual void InvalidateAll();             ///< invalidate all tools
	NDIPassiveTool* GetInternalTool(std::string portHandle); ///< returns the tool object that has been assigned the port handle or NULL if no tool can be found

	/**
	* \brief free all port handles that need to be freed
	*
	* This method retrieves a list of all port handles that need to be freed (e.g. tool got disconnected)
	* and frees the handles at the tracking device and it removes the tools from the internal tool list
	* \warning This method can remove TrackingTools from the tool list! After calling this method, GetTool(i) could return
	*          a different tool, because tool indices could have changed.
	* @throw IGTHardwareException Throws an exception if there are errors while communicating with the device.
	* \return returns NDIOKAY if everything was sucessfull, returns an error code otherwise
	*/
	NDIErrorCode FreePortHandles();

	NDIErrorCode Send(const std::string* message, bool addCRC = true);      ///< Send message to tracking device
    NDIErrorCode Receive(std::string *answer, unsigned int numberOfBytes);  ///< receive numberOfBytes bytes from tracking device
	NDIErrorCode ReceiveByte(char* answer);   ///< lightweight receive function, that reads just one byte
    NDIErrorCode ReceiveLine(std::string *answer); ///< receive characters until the first LF (The LF is included in the answer string)
	void ClearSendBuffer();                   ///< empty send buffer of serial communication interface
	void ClearReceiveBuffer();                ///< empty receive buffer of serial communication interface
	const std::string CalcCRC(const std::string* input);  ///< returns the CRC16 for input as a std::string

public:

	/**
	* \brief TrackTools() continuously polls serial interface for new 6d tool positions until StopTracking is called.
	*
	* Continuously tracks the 6D position of all tools until StopTracking() is called.
	* This function is executed by the tracking thread (through StartTracking() and ThreadStartTracking()).
	* It should not be called directly.
	* @throw IGTHardwareException Throws an exception if there are errors while tracking the tools.
	*/
	virtual void TrackTools();

	/**
	* \brief continuously polls serial interface for new 3D marker positions until StopTracking is called.
	*
	* Continuously tracks the 3D position of all markers until StopTracking() is called.
	* This function is executed by the tracking thread (through StartTracking() and ThreadStartTracking()).
	* It should not be called directly.
	*/
	virtual void TrackMarkerPositions();

	/**
	* \brief continuously polls serial interface for new 3D marker positions and 6D tool positions until StopTracking is called.
	*
	* Continuously tracks the 3D position of all markers and the 6D position of all tools until StopTracking() is called.
	* This function is executed by the tracking thread (through StartTracking() and ThreadStartTracking()).
	* It should not be called directly.
	*/
	virtual void TrackToolsAndMarkers();

	/**
	* \brief static start method for the tracking thread.
	*/
	static ITK_THREAD_RETURN_TYPE ThreadStartTracking(void* data);

protected:
	NDITrackingDevice();          ///< Constructor
	virtual ~NDITrackingDevice(); ///< Destructor

	std::string m_DeviceName;///< Device Name
	BaudRate m_BaudRate;     ///< COM Port Baud Rate
	DataBits m_DataBits;     ///< Number of Data Bits per token
	Parity m_Parity;         ///< Parity mode for communication
	StopBits m_StopBits;     ///< number of stop bits per token
    FlowControl m_HardwareHandshake; ///< use hardware handshake for serial port connection
	///< which tracking volume is currently used (if device supports multiple volumes) (\warning This parameter is not used yet)
	IlluminationActivationRate m_IlluminationActivationRate; ///< update rate of IR illuminator for Polaris
	DataTransferMode m_DataTransferMode;  ///< use TX (text) or BX (binary) (\warning currently, only TX mode is supported)
	Tool6DContainerType m_6DTools;        ///< list of 6D tools

    QMutex *m_ToolsMutex; ///< mutex for coordinated access of tool container
    SerialWorker *m_SerialCommunication;    ///< serial communication interface
    QMutex *m_SerialCommunicationMutex; ///< mutex for coordinated access of serial communication interface
	NDIProtocol::Pointer m_DeviceProtocol;    ///< create and parse NDI protocol strings

    QMutex *m_MultiThreader;      ///< creates tracking thread that continuously polls serial interface for new tracking data
	int m_ThreadID;                 ///< ID of tracking thread
	OperationMode m_OperationMode;  ///< tracking mode (6D tool tracking, 3D marker tracking,...)
    QMutex *m_MarkerPointsMutex;  ///< mutex for marker point data container
	MarkerPointContainerType m_MarkerPoints;          ///< container for markers (3D point tracking mode)
};

#endif /* NDITRACKINGDEVICE_H_HEADER_INCLUDED_C1C2FCD2 */
