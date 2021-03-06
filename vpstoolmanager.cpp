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
        sprintf(portData, "%02d", i);
        std::string pd(portData);
        setPortHandle(pd);
    }
}

void vpsToolManager::deconstructor()
{
    m_portPool.clear();
    if(m_Instance)
        delete m_Instance;
    m_Instance = NULL;
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
    for(int i =0; i < m_portIndex; i++)
    {
        char tmp[5];
        PortHandle& ph = m_portPool.at(i);
        sprintf(tmp, "%s%03d", ph.portName.c_str(), ph.statu);
        out.append(tmp);
    }
    char numbers[2];
    sprintf(numbers, "%02d", m_portIndex);
    out.insert(0, numbers);
    return out;
}

std::string vpsToolManager::getFreePortHandle()
{
    std::string out;
    if( !m_portPool.empty() ){
        for(int i = m_portPool.size()-1; i >= m_portIndex; i--)
        {
            char tmp[5];
            PortHandle& ph = m_portPool.at(i);
            sprintf(tmp, "%s%03d", ph.portName.c_str(), ph.statu);
            out.append(tmp);
        }
        char numbers[2];
        sprintf(numbers, "%02d", (m_portPool.size() - m_portIndex));
        out.insert(0, numbers);
    }

    return out;
}

bool vpsToolManager::initPortHandle(std::string &data)
{
    int i =0;
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
