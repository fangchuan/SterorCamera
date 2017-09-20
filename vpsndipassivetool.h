#ifndef NDIPASSIVETOOL_H_HEADER_INCLUDED_
#define NDIPASSIVETOOL_H_HEADER_INCLUDED_

#include "vpsinternaltrackingtool.h"
#include "trackingtypes.h"
#include <string>
class NDITrackingDevice;
/**Documentation
* \brief Implementation of a passive NDI optical tool
*
* implements the TrackingTool interface and has the ability to
* load an srom file that contains the marker configuration for that tool
*
* \ingroup IGT
*/

#define vpsSetStringMacro(name)         void Set##name (const char *_arg)               \
                                        {                                                       \
                                            if ( _arg && ( _arg == this->m_##name ) ) { return; } \
                                            if ( _arg )                                             \
                                            {                                                     \
                                              this->m_##name = _arg;                              \
                                            }                                                      \
                                            else                                                    \
                                            {                                                     \
                                              this->m_##name = "";                                \
                                            }                                                     \
                                        }                                                       \
                                        void Set##name (const std::string & _arg)       \
                                        {                                                       \
                                            this->Set##name( _arg.c_str() );                      \
                                        }

#define vpsGetStringMacro(name)         const char *Get##name () const \
                                        {                                      \
                                            return this->m_##name.c_str();       \
                                        }



class NDIPassiveTool// : public InternalTrackingTool
{
public:
	friend class NDITrackingDevice;
	/**
	* \brief tracking priority for NDI tracking devices
	*/
	enum TrackingPriority
	{
		Static    = 'S',
		Dynamic   = 'D',
		ButtonBox = 'B'
	};


	virtual bool LoadSROMFile(const char* filename);      ///< load a srom tool description file
	virtual const unsigned char* GetSROMData() const;     ///< get loaded srom file as unsigned char array
	virtual unsigned int GetSROMDataLength() const;       ///< get length of SROMData char array


    virtual void SetPortHandle(const std::string &_arg);
    virtual const char* GetPortHandle() const;
    virtual void SetFile(const std::string &_arg);
    virtual const char* GetFile() const;
    virtual void SetSerialNumber(const std::string &_arg);
    virtual const char* GetSerialNumber() const;
    virtual void SetTrackingPriority(TrackingPriority _arg);
    virtual const TrackingPriority GetTrackingPriority() const;


protected:
	NDIPassiveTool();
	virtual ~NDIPassiveTool();

	unsigned char* m_SROMData;            ///< content of the srom tool description file
	unsigned int m_SROMDataLength;        ///< length of the  srom tool description file
	TrackingPriority m_TrackingPriority;  ///< priority for this tool
	std::string m_PortHandle;             ///< port handle for this tool
	std::string m_SerialNumber;           ///< serial number for this tool
	std::string m_File;                   ///< the original file from which this tool was loaded
};

#endif /* MITKNDIPASSIVETOOL_H_HEADER_INCLUDED_ */
