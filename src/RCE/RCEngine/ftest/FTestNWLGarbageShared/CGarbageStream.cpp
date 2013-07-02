/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CGarbageStream.cpp
///
///  Implements CGarbageStream class, responsible stream which will send 
///    garbage to stream
///
///  @author Dmitry Netrebenko @date 06.06.2007
///
////////////////////////////////////////////////////////////////////////

#include "CGarbageStream.h"
#include <AidLib/CException/CException.h>
#include <stdlib.h>
#include <boost/scoped_array.hpp>
#include "CHelper.h"

CGarbageStream::CGarbageStream(bool* active)
	:	CSocketStream()
	,	m_sessionActive(active)
{
TRY_CATCH

	if(!m_sessionActive)
		throw MCException(_T("Invalid pointer"));

	CalcNewGarbageTime();

CATCH_THROW()
}

CGarbageStream::~CGarbageStream()
{
TRY_CATCH

CATCH_LOG()
}

unsigned int CGarbageStream::SendInternal(const char* buf, const unsigned int& len)
{
TRY_CATCH

	if(timeGetTime() > m_timeForGarbage)
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
			CalcNewGarbageTime();
			return CSocketStream::SendInternal(buffer.get(), len);
		}
		else
		{
			CalcNewGarbageTime();
			return CSocketStream::SendInternal(buf, len);
		}
	}
	else
		return CSocketStream::SendInternal(buf, len);

CATCH_THROW()
}

void CGarbageStream::CalcNewGarbageTime()
{
TRY_CATCH

	DWORD currentTime = timeGetTime();
	m_timeForGarbage = currentTime + CHelper::GetRandom(15000, 20000);

CATCH_THROW()
}
