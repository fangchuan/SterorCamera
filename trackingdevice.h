#ifndef TRACKINGDEVICE_H
#define TRACKINGDEVICE_H

#include "vpscommon.h"
#include "trackingtypes.h"
#include <QMutex>

class TrackingTool; // interface for a tool that can be tracked by the TrackingDevice

class TrackingDevice : public QObject
{
     Q_OBJECT
public:

    enum RotationMode {RotationStandard, RotationTransposed};
    ///< Type for state variable. The trackingdevice is always in one of these states
    enum TrackingDeviceState {Setup=0, Ready=1, Tracking=2, ErrorState=-1};
    /**
    * \brief Opens a connection to the device
    *
    * This may only be called if there is currently no connection to the device.
    * If OpenConnection() is successful, the object will change from Setup state to Ready state
    */
    virtual bool OpenConnection() = 0;

    /**
    * \brief Closes the connection to the device
    *
    * This may only be called if there is currently a connection to the device, but tracking is
    * not running (e.g. object is in Ready state)
    */
    virtual bool CloseConnection() = 0; ///< Closes the connection with the device

    /**
    * \brief start retrieving tracking data from the device.
    *
    * This may only be called after the connection to the device has been established
    * with a call to OpenConnection() (E.g. object is in Ready mode). This will change the
    * object state from Ready to Tracking
    */
    virtual bool StartTracking() = 0;

    /**
    * \brief stop retrieving tracking data from the device.
    * stop retrieving tracking data from the device.
    * This may only be called after StartTracking was called
    * (e.g. the object is in Tracking mode).
    * This will change the object state from Tracking to Ready.
    */
    virtual bool StopTracking();

    virtual TrackingTool* GetTool(unsigned int toolNumber) const = 0;

    /**
    * \brief Returns the tool with the given tool name
    *
    * Note: subclasses can and should implement optimized versions of this method
    * \return the given tool or NULL if no tool with that name exists
    */
    virtual TrackingTool* GetToolByName(std::string name) const;

    virtual unsigned int GetToolCount() const = 0;

    virtual void SetRotationMode(RotationMode r);

    TrackingDeviceState GetState() const;

    TrackingDeviceData GetData() const;

    void SetData(TrackingDeviceData data);

protected:

    void SetState(TrackingDeviceState state);

    TrackingDevice();
    virtual ~TrackingDevice();

    TrackingDeviceData m_Data; ///< current device Data
    TrackingDeviceState m_State; ///< current object state (Setup, Ready or Tracking)
    bool m_StopTracking;       ///< signal stop to tracking thread
    QMutex *m_StopTrackingMutex; ///< mutex to control access to m_StopTracking
    QMutex *m_TrackingFinishedMutex; ///< mutex to manage control flow of StopTracking()
    QMutex *m_StateMutex; ///< mutex to control access to m_State
    RotationMode m_RotationMode; ///< defines the rotation mode Standard or Transposed, Standard is default
};

#endif /* TRACKINGDEVICE_H */
