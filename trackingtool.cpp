#include "trackingtool.h"
#include <QMutexLocker>



TrackingTool::TrackingTool()
: itk::Object(), m_ToolName(""), m_ErrorMessage(""), m_IGTTimeStamp(0)
{
  m_MyMutex = new QMutex(this);
}


TrackingTool::~TrackingTool()
{
  m_MyMutex->Unlock();
  m_MyMutex = NULL;
}

void TrackingTool::PrintSelf(std::ostream& os, unsigned int indent) const
{
  os << indent << "ToolName: " << m_ToolName << std::endl;
  os << indent << "ErrorMesage: " << m_ErrorMessage << std::endl;
}

const char* TrackingTool::GetToolName() const
{
      QMutexLocker lock(m_MyMutex); // lock and unlock the mutex
      return this->m_ToolName.c_str();
}


const char* TrackingTool::GetErrorMessage() const
{
     QMutexLocker lock(m_MyMutex); // lock and unlock the mutex
     return this->m_ErrorMessage.c_str();
}
