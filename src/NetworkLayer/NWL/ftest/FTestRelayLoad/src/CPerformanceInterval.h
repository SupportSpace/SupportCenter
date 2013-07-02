#pragma once

#include <windows.h>

class CPerformanceInterval
{
     LARGE_INTEGER     m_counterFreq;
     LARGE_INTEGER     m_startedTime;
     LARGE_INTEGER     m_finishedTime;
public:
     CPerformanceInterval();

     /// Start counter
     void Start();

     /// Stop counter
     void Stop();

     /// Get interval in milliseconds
     /// @return          Return interval between Start() and Stop() methods in milliseconds
     __int64 GetSecExp3() const;

     /// Get interval in microseconds
     /// @return          Return interval between Start() and Stop() methods in microseconds
     __int64 GetSecExp6() const;

     /// Get interval in nanoseconds
     /// @return          Return interval between Start() and Stop() methods in microseconds
     __int64 GetSecExp9() const;
};
//----------------------------------------------------------------------------------------

inline void CPerformanceInterval::Start()
{
     QueryPerformanceCounter(&m_startedTime);
}
//----------------------------------------------------------------------------------------

inline void CPerformanceInterval::Stop()
{
     QueryPerformanceCounter(&m_finishedTime);
}