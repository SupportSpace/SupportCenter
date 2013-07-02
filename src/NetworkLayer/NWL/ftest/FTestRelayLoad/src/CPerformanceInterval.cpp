#include "CPerformanceInterval.h"

CPerformanceInterval::CPerformanceInterval()
{
     QueryPerformanceFrequency(&m_counterFreq);
     
     //To prevent zero divide if hardware doesn't support a high-resolution performance counter
     if (!m_counterFreq.QuadPart)
          m_counterFreq.QuadPart = 1;
     
     m_startedTime.QuadPart     = 0;
     m_finishedTime.QuadPart     = 0;
}
//----------------------------------------------------------------------------------------

__int64 CPerformanceInterval::GetSecExp3() const
{
     return ((m_finishedTime.QuadPart - m_startedTime.QuadPart)*1000)/m_counterFreq.QuadPart;
}
//----------------------------------------------------------------------------------------

__int64 CPerformanceInterval::GetSecExp6() const
{
     return ((m_finishedTime.QuadPart - m_startedTime.QuadPart)*1000000)/m_counterFreq.QuadPart;
}
//----------------------------------------------------------------------------------------

__int64 CPerformanceInterval::GetSecExp9() const
{
     return ((m_finishedTime.QuadPart - m_startedTime.QuadPart)*1000000000)/m_counterFreq.QuadPart;
}
