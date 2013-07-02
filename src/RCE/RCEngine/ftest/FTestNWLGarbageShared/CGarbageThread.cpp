/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CGarbageThread.cpp
///
///  Implements CGarbageThread class, responsible thread which will send 
///    garbage to stream
///
///  @author Dmitry Netrebenko @date 06.06.2007
///
////////////////////////////////////////////////////////////////////////

#include "CGarbageThread.h"
#include <AidLib/CException/CException.h>
#include <boost/scoped_array.hpp>
#include "CHelper.h"

CGarbageThread::CGarbageThread(boost::shared_ptr<CAbstractStream> stream, bool* active)
	:	m_sessionActive(active)
	,	CThread()
	,	m_stream(stream)
{
TRY_CATCH

	if(!m_sessionActive)
		throw MCException(_T("Invalid pointer"));

CATCH_THROW()
}

CGarbageThread::~CGarbageThread()
{
TRY_CATCH

	Terminate();

CATCH_LOG()
}

void CGarbageThread::Execute(void* Params)
{
TRY_CATCH

	DWORD timeForGarbage = CalcNextGarbageTime();
	while(!Terminated())
	{
		if(timeGetTime() > timeForGarbage)
		{
			if(*m_sessionActive)
			{
				int buflen = CHelper::GetRandom(4, 1024);
				Log.Add(_MESSAGE_, _T("Garbage size: %d"), buflen);
				boost::scoped_array<char> buffer(new char[buflen]);
				char* current = buffer.get();
				for(int i = 0; i < buflen - (int)sizeof(int);)
				{
					int value = rand();
					memcpy(current, &value, sizeof(int));
					i += sizeof(int);
					current += sizeof(int);
				}
				/// Remove "Stop Session" command
				current = buffer.get();
				for(int i = 0; i < buflen; ++i)
				{
					if(17 == *current)
						*current = '\xFF';
					current++;
				}
				m_stream->Send(buffer.get(), buflen);
			}
			timeForGarbage = CalcNextGarbageTime();
		}
		Sleep(100);
	}

CATCH_LOG()
}

DWORD CGarbageThread::CalcNextGarbageTime()
{
TRY_CATCH

	return timeGetTime() + CHelper::GetRandom(10000, 20000);

CATCH_THROW()
}
