#ifndef TRACKINGTYPES_H_HEADER_INCLUDED_
#define TRACKINGTYPES_H_HEADER_INCLUDED_

#include <vector>
#include <string>
#include "TrackerCommon.h"


/**Documentation
* \brief Error codes of NDI tracking devices
*/
enum NDIErrorCode
{
	NDIOKAY = 0,
	NDIERROR = 1,
	SERIALINTERFACENOTSET,
	SERIALSENDERROR,
	SERIALRECEIVEERROR,
	SROMFILETOOLARGE,
	SROMFILETOOSMALL,
	NDICRCERROR,                          // reply has crc error, local computer detected the error
	NDIINVALIDCOMMAND,
	NDICOMMANDTOOLONG,
	NDICOMMANDTOOSHORT,
	NDICRCDOESNOTMATCH,                   // command had crc error, tracking device detected the error
	NDITIMEOUT,
	NDIUNABLETOSETNEWCOMMPARAMETERS,
	NDIINCORRECTNUMBEROFPARAMETERS,
	NDIINVALIDPORTHANDLE,
	NDIINVALIDTRACKINGPRIORITY,
	NDIINVALIDLED,
	NDIINVALIDLEDSTATE,
	NDICOMMANDINVALIDINCURRENTMODE,
	NDINOTOOLFORPORT,
	NDIPORTNOTINITIALIZED,
	NDISYSTEMNOTINITIALIZED,
	NDIUNABLETOSTOPTRACKING,
	NDIUNABLETOSTARTTRACKING,
	NDIINITIALIZATIONFAILED,
	NDIINVALIDVOLUMEPARAMETERS,
	NDICANTSTARTDIAGNOSTICMODE,
	NDICANTINITIRDIAGNOSTICS,
	NDIFAILURETOWRITESROM,
	NDIENABLEDTOOLSNOTSUPPORTED,
	NDICOMMANDPARAMETEROUTOFRANGE,
	NDINOMEMORYAVAILABLE,
	NDIPORTHANDLENOTALLOCATED,
	NDIPORTHASBECOMEUNOCCUPIED,
	NDIOUTOFHANDLES,
	NDIINCOMPATIBLEFIRMWAREVERSIONS,
	NDIINVALIDPORTDESCRIPTION,
	NDIINVALIDOPERATIONFORDEVICE,
	NDIWARNING,
	NDIUNKNOWNERROR,
	NDIUNEXPECTEDREPLY,
	UNKNOWNHANDLERETURNED,
	TRACKINGDEVICERESET,
	TRACKINGDEVICENOTSET
};

/**Documentation
* \brief identifier for tracking device. The way it is currently used
* represents a product line rather than an actal type. Refactoring is a future option.
*/
enum TrackingDeviceType
{
	NDIPolaris,                ///< Polaris: optical Tracker from NDI
	NDIAurora,                 ///< Aurora: electromagnetic Tracker from NDI
	ClaronMicron,              ///< Micron Tracker: optical Tracker from Claron
	IntuitiveDaVinci,          ///< Intuitive Surgical: DaVinci Telemanipulator API Interface
	AscensionMicroBird,        ///< Ascension microBird / PCIBird family
	VirtualTracker,            ///< Virtual Tracking device class that produces random tracking coordinates
	TrackingSystemNotSpecified,///< entry for not specified or initialized tracking system
	TrackingSystemInvalid      ///< entry for invalid state (mainly for testing)
};



/**Documentation
* \brief Error codes of NDI tracking devices
*/
enum OperationMode
{
	ToolTracking6D,
	ToolTracking5D,
	MarkerTracking3D,
	HybridTracking
};

/**
* /brief This structure defines key variables of a device model and type.
* It is specifically used to find out which models belong to which vendor, and what volume
* to use for a specific Model. Leaving VolumeModelLocation set to null will instruct the Generator
* to generate a field to the best of his ability. HardwareCode stands for a hexadecimal string,
* that represents the tracking volume. "X" stands for "hardwarecode is not known" or "tracking device has
* no hardware code". For NDI devices it is used in the SetVolume() Method in  mitkNDITrackingDevice.cpp.
* The Pyramid Volume has the hardwarecode "4", but it is not supported yet.
*/
struct TrackingDeviceData {
	TrackingDeviceType Line;
	std::string Model;
	std::string VolumeModelLocation;
	std::string HardwareCode;
};


/**
* Here all supported devices are defined.
* Dont forget to introduce new Devices into the TrackingDeviceList Array at the bottom!
* If a model does not have a corresponding tracking volume yet,
*  pass an empty string to denote "No Model". pass "cube" to render
* a default cube of 400x400 px. You can define additional magic strings in the TrackingVolumeGenerator.
*/

//############## NDI Aurora device data #############
static TrackingDeviceData DeviceDataAuroraCompact = {NDIAurora,
                                                     "Aurora Compact",
                                                     "NDIAuroraCompactFG_Dome.stl",
                                                     "A"};
static TrackingDeviceData DeviceDataAuroraPlanarCube = {NDIAurora,
                                                        "Aurora Planar (Cube)",
                                                        "NDIAurora.stl",
                                                        "9"};
static TrackingDeviceData DeviceDataAuroraPlanarDome = {NDIAurora,
                                                        "Aurora Planar (Dome)",
                                                        "NDIAuroraPlanarFG_Dome.stl",
                                                        "A"};
static TrackingDeviceData DeviceDataAuroraTabletop = {NDIAurora,
                                                      "Aurora Tabletop",
                                                      "NDIAuroraTabletopFG_Dome.stl",
                                                      "A"};

//############## NDI Polaris device data ############
static TrackingDeviceData DeviceDataPolarisOldModel = {NDIPolaris,
                                                       "Polaris (Old Model)",
                                                       "NDIPolarisOldModel.stl",
                                                       "0"};
//full hardware code of polaris spectra: 5-240000-153200-095000+057200+039800+056946+024303+029773+999999+99999924
static TrackingDeviceData DeviceDataPolarisSpectra = {NDIPolaris,
                                                      "Polaris Spectra",
                                                      "NDIPolarisSpectra.stl",
                                                      "5-2"};
//full hardware code of polaris spectra (extended pyramid): 5-300000-153200-095000+057200+039800+056946+024303+029773+999999+07350024
static TrackingDeviceData DeviceDataSpectraExtendedPyramid = {NDIPolaris,
                                                              "Polaris Spectra (Extended Pyramid)",
                                                              "NDIPolarisSpectraExtendedPyramid.stl",
                                                              "5-3"};
static TrackingDeviceData DeviceDataPolarisVicra = {NDIPolaris,
                                                    "Polaris Vicra",
                                                    "NDIPolarisVicra.stl",
                                                    "7"};

//############## other device data ##################
static TrackingDeviceData DeviceDataDaVinci = {IntuitiveDaVinci,
                                               "Intuitive DaVinci",
                                               "IntuitiveDaVinci.stl",
                                               "X"};
static TrackingDeviceData DeviceDataVirtualTracker = {VirtualTracker,
                                                      "Virtual Tracker",
                                                      "cube",
                                                      "X"};
static TrackingDeviceData DeviceDataMicronTrackerH40 = {ClaronMicron,
                                                        "Micron Tracker H40",
                                                        "ClaronMicron.stl",
                                                        "X"};
static TrackingDeviceData DeviceDataUnspecified = {TrackingSystemNotSpecified,
                                                   "Unspecified System",
                                                   "cube",
                                                   "X"};
// Careful when changing the "invalid" device: The mitkTrackingTypeTest is using it's data.
static TrackingDeviceData DeviceDataInvalid = {TrackingSystemInvalid,
                                               "Invalid Tracking System",
                                               "",
                                               "X"};

//This list should hold all devices defined above!
static TrackingDeviceData TrackingDeviceList[] = {DeviceDataAuroraPlanarCube,
                                                  DeviceDataAuroraPlanarDome,
                                                  DeviceDataAuroraCompact,
                                                  DeviceDataAuroraTabletop,
                                                  DeviceDataMicronTrackerH40,
                                                  DeviceDataPolarisSpectra,
                                                  DeviceDataPolarisVicra,
                                                  DeviceDataDaVinci,
                                                  DeviceDataVirtualTracker,
                                                  DeviceDataUnspecified,
                                                  DeviceDataSpectraExtendedPyramid,
                                                  DeviceDataInvalid,
                                                  DeviceDataPolarisOldModel};

/**
* /brief Returns all devices compatibel to the given Line of Devices
*/
//static std::vector<TrackingDeviceData> GetDeviceDataForLine(TrackingDeviceType Type){
//	std::vector<TrackingDeviceData> Result;
//	int size = (sizeof (TrackingDeviceList) / sizeof*(TrackingDeviceList));
//	for(int i=0; i < size; i++)
//	{
//                if(TrackingDeviceList[i].Line == Type )
//                    Result.push_back(TrackingDeviceList[i]);
//	}
//	return Result;
//}

/**
* /brief Returns the first TracingDeviceData mathing a given line. Useful for backward compatibility
* with the old way to manage Devices
*/
//static TrackingDeviceData GetFirstCompatibleDeviceDataForLine(TrackingDeviceType Type){

//	return GetDeviceDataForLine(Type).front();
//}

/**
* /brief Returns the device Data set matching the model name or the invalid device, if none was found
*/
//static TrackingDeviceData GetDeviceDataByName(std::string modelName){

//	int size = (sizeof (TrackingDeviceList) / sizeof*(TrackingDeviceList));
//	for(int i=0; i < size; i++)
//	{
//            if(TrackingDeviceList[i].Model == modelName)
//                return TrackingDeviceList[i];
//	}
//	return DeviceDataInvalid;
//}

/**Documentation
* \brief activation rate of IR illuminator for NDI Polaris tracking device
*/
enum IlluminationActivationRate
{
	Hz20 = 20,
	Hz30 = 30,
	Hz60 = 60
};

/**Documentation
* \brief Data transfer mode for NDI tracking devices
*/
enum DataTransferMode
{
	TX = 0,
	BX = 1
};
/**Documentation
* \brief Query mode for NDI tracking devices
*/
enum PHSRQueryType
{
	ALL         = 0x00,
	FREED       = 0x01,
	OCCUPIED    = 0x02,
	INITIALIZED = 0x03,
	ENABLED     = 0x04
};


#endif /* MITKTRACKINGTYPES_H_HEADER_INCLUDED_ */
