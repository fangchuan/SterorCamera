#include "includes.h"
#include <iostream>
#include <QDebug>

vpsToolManager * vpsToolManager::m_Instance = NULL;

vpsToolManager::vpsToolManager()
{
    m_Instance = this;
    m_portIndex = 0;

    char  portData[2];
    int i = 0;
    for (; i < MAX_TOOL; i ++){
        sprintf(portData, "%02X", i);
        std::string pd(portData);
        setPortHandle(pd);
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
        PortHandle& ph = m_portPool.at(m_portIndex);
        ph.statu = PORT_OCCUPIED;
        m_portIndex ++;
        return ph.portName;
    }else{
#ifdef USE_DEBUG
        qDebug() << "PortHandle assign out of bound!!!";
#endif
        return "";
    }
}

void vpsToolManager::setPortHandle(std::string& port)
{
    PortHandle ph;
    ph.portName = port;
    ph.priority = Dynamic;
    ph.statu = PORT_FREE;

    m_portPool.push_back(ph);
}

std::string vpsToolManager::getOccupiedPortHandle()
{
    std::string out;
    for(unsigned int i =0; i < m_portIndex; i++)
    {
        char tmp[3];
        PortHandle& ph = m_portPool.at(i);
        sprintf(tmp, "%s%c", ph.portName.c_str(), ph.statu);
        out.append(tmp);
    }
    char numbers[2];
    sprintf(numbers, "%02X", m_portIndex);
    out.insert(0, numbers);
    return out;
}

std::string vpsToolManager::getFreePortHandle()
{
    std::string out;
    if( !m_portPool.empty() ){
        for(unsigned int i = m_portPool.size(); i > m_portIndex; i--)
        {
            char tmp[3];
            PortHandle& ph = m_portPool.at(i);
            sprintf(tmp, "%s%c", ph.portName.c_str(), ph.statu);
            out.append(tmp);
        }
        char numbers[2];
        sprintf(numbers, "%02X", (m_portPool.size() - m_portIndex));
        out.insert(0, numbers);
    }

    return out;
}

bool vpsToolManager::initPortHandle(std::string &data)
{
    unsigned int i =0;
    for(; i< m_portIndex; i++)
    {
        PortHandle& ph = m_portPool.at(i);
        if(ph.portName == data)
        {
            ph.statu = PORT_INITED;
            return true;
        }
    }
    if(i == m_portIndex)
        return false;
}

bool vpsToolManager::setTrackingPriority(std::__cxx11::string &port, char priority)
{
    unsigned int i =0;
    for(; i< m_portIndex; i++)
    {
        PortHandle& ph = m_portPool.at(i);
        if( ph.portName == port)
        {
            ph.statu = PORT_ENABLED;
            ph.priority = (TrackingPriority)priority;
            return true;
        }
    }
    if(i == m_portIndex)
        return false;

}
