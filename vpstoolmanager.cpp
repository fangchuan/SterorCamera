#include "vpstoolmanager.h"
#include <QDebug>

vpsToolManager * vpsToolManager::m_Instance = NULL;

vpsToolManager::vpsToolManager()
{
    m_Instance = this;
    m_portIndex = 0;
    m_tarckPriority  = Dynamic;

    char  portData[2];
    int i = 0;
    for (; i < MAX_TOOL; i ++){
        sprintf(portData, "%02X", i);
        qDebug() << portData;
        setPortHandle(std::string(portData));
    }
}

vpsToolManager::~vpsToolManager()
{
    m_portPool.clear();
}


vpsToolManager *vpsToolManager::getInstance()
{
    return m_Instance == NULL ? new vpsToolManager : m_Instance;

}

std::string vpsToolManager::getPortHandle()
{
    if(m_portIndex < MAX_TOOL){
        return m_portPool.at(m_portIndex ++);
    }else{
#ifdef USE_DEBUG
        qDebug() << "PortHandle assign out of bound!!!";
#endif
        return "";
    }
}

void vpsToolManager::setPortHandle(std::string &port)
{
    m_portPool.push_back(port);
}

std::string vpsToolManager::getOccupiedPortHandle()
{
    std::string out;
    for(int i =0; i < m_portIndex; i++)
    {
        std::strcat(out,m_portPool.at(i).c_str());
    }
    return out;
}

std::string vpsToolManager::getFreePortHandle()
{
    std::string out;
    if( !m_portPool.empty() ){
        for(int i = m_portPool.size(); i > m_portIndex; i--)
        {
            std::strcat(out,m_portPool.at(i-1).c_str());
        }
    }

    return out;
}

bool vpsToolManager::initPortHandle(std::string &data)
{
    int i =0;
    for(; i< m_portIndex; i++)
    {
        if(m_portPool.at(i) == data)
            return true;
    }
    if(i == m_portIndex)
        return false;
}

bool vpsToolManager::setTrackingPriority(std::string &port, char priority)
{
    int i =0;
    for(; i< m_portIndex; i++)
    {
        if(m_portPool.at(i) == data)
        {
            m_trackPriority = priority;
            return true;
        }
    }
    if(i == m_portIndex)
        return false;

}
