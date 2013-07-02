#include "cshadowedstream.h"
#include <AidLib/Logging/CLog.h>

CShadowedStream::CShadowedStream(boost::shared_ptr<CAbstractStream> mainStream, const EShadowType type)
	:	m_mainStream(mainStream), 
		m_shadowStream(reinterpret_cast<CAbstractStream*>(NULL)), 
		m_type(type)
{
TRY_CATCH
	if (m_mainStream.get() == NULL) throw MCException("m_mainStream == NULL");
	InitializeCriticalSection(&m_cs);
CATCH_THROW("CShadowedStream::CShadowedStream")
}

CShadowedStream::~CShadowedStream(void)
{
TRY_CATCH
	DeleteCriticalSection(&m_cs);
CATCH_LOG("CShadowedStream::~CShadowedStream")
}

boost::shared_ptr<CAbstractStream> CShadowedStream::SetShadowStream(boost::shared_ptr<CAbstractStream> shadowStream)
{
TRY_CATCH
	CCritSection cs(&m_cs);
	boost::shared_ptr<CAbstractStream> oldShadowStream = m_shadowStream;
	m_shadowStream = shadowStream;
	return oldShadowStream;
CATCH_THROW("CShadowedStream::SetShadowStream")
}

unsigned int CShadowedStream::ReceiveInternal( char* buf, const unsigned int& len )
{
TRY_CATCH
	if (!len) return len;
	m_mainStream->ReceiveInternal(buf, len);
	switch(m_type)
	{
		case OUTPUT:
			break;
		case INPUT:
		case BOTH:
			{
				//CCritSection cs(&m_cs);
				if (m_shadowStream.get())
				{
					m_shadowStream->SendInternal(buf, len);
				}
			}
			break;
	}
	return len;
CATCH_THROW("CShadowedStream::ReceiveInternal")
}

unsigned int CShadowedStream::SendInternal( const char* buf, const unsigned int& len )
{
TRY_CATCH
	if (!len) return len;
	m_mainStream->SendInternal(buf, len);
	switch(m_type)
	{
		case OUTPUT:
		case BOTH:
			{
				//CCritSection cs(&m_cs);
				if (m_shadowStream.get())
				{
					m_shadowStream->SendInternal(buf, len);
				}
			}
			break;
		case INPUT:
			break;
	}
	return len;
CATCH_THROW("CShadowedStream::SendInternal")
}

bool CShadowedStream::HasInData()
{
TRY_CATCH
	return m_mainStream->HasInData();
CATCH_THROW("CShadowedStream::HasInData")
}

boost::shared_ptr<CAbstractStream> CShadowedStream::GetMainStream()
{
TRY_CATCH
	return m_mainStream;
CATCH_THROW("CShadowedStream::GetMainStream")
}

/// Setup critical section ¹1 for some internal issues
/// @param cs1 pointer to critical section ¹1 structure
void CShadowedStream::SetCS1(CRITICAL_SECTION *cs1)
{
TRY_CATCH
	CAbstractStream::SetCS1(cs1);
	m_mainStream->SetCS1(cs1);
	if (m_shadowStream.get())
		m_shadowStream->SetCS1(cs1);
CATCH_THROW("CShadowedStream::SetCS1")
}

unsigned int CShadowedStream::GetInBuffer( char* buf, const unsigned int& len)
{
TRY_CATCH
	
	int result = m_mainStream->GetInBuffer(buf, len);
	if (!result) return 0;
	switch(m_type)
	{
		case OUTPUT:
			break;
		case INPUT:
		case BOTH:
			{
				//CCritSection cs(&m_cs);
				if (m_shadowStream.get())
				{
					m_shadowStream->SendInternal(buf, result);
				}
			}
			break;
	}
	return result;
	//return CAbstractStream::GetInBuffer(buf, len);

CATCH_THROW("CShadowedStream::GetInBuffer")
}

void CShadowedStream::CancelReceiveOperation()
{
TRY_CATCH
	m_mainStream->CancelReceiveOperation();
CATCH_THROW("CShadowedStream::CancelReceiveOperation")
}