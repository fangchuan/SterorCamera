#ifndef VPSTOOLMANAGER_H
#define VPSTOOLMANAGER_H

#include <string>
#include <vector>

#define MAX_TOOL  16

//
enum TrackingPriority
{
    Static    = 'S',
    Dynamic   = 'D',
    ButtonBox = 'B'
};

//port handle statu
enum PortStatu{
    PORT_FREE = 0,
    PORT_OCCUPIED = 1,
    PORT_INITED = 2,
    PORT_ENABLED = 3,
};
// port handle structure
struct PortHandle{
  TrackingPriority priority;
  std::string portName;
  PortStatu  statu;
};



class vpsToolManager
{
public:
    static vpsToolManager* getInstance();
    std::__cxx11::string getPortHandle();
    void setPortHandle(std::string &port);
    std::__cxx11::string getOccupiedPortHandle();
    std::__cxx11::string getFreePortHandle();
    bool initPortHandle(std::string& data);
    bool setTrackingPriority(std::string& port, char priority);
    char getTrackingPriority(std::string& port);

    void deconstructor();
private:
    vpsToolManager();

    std::vector<PortHandle>  m_portPool;
    int m_portIndex;
    static  vpsToolManager *m_Instance;
};

#endif // VPSTOOLMANAGER_H
