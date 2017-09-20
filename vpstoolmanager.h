#ifndef VPSTOOLMANAGER_H
#define VPSTOOLMANAGER_H

#include <string>
#include <vector>

#define MAX_TOOL  16

enum TrackingPriority
{
    Static    = 'S',
    Dynamic   = 'D',
    ButtonBox = 'B'
};

class vpsToolManager
{
public:
    static vpsToolManager* getInstance();
    std::string getPortHandle();
    void setPortHandle(std::string& port);
    std::string getOccupiedPortHandle();
    std::string getFreePortHandle();
    bool initPortHandle(std::string& data);
    bool setTrackingPriority(std::string& port, char priority);
private:
    vpsToolManager();
    ~vpsToolManager();

    TrackingPriority m_tarckPriority;
    std::vector<std::string>  m_portPool;
    int m_portIndex;
    static  vpsToolManager *m_Instance;
};

#endif // VPSTOOLMANAGER_H
