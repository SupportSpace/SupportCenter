/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CAsyncSendStream.cpp
///
///  Stream with asynchronous send method 
///
///  @author "Archer Software" Sogin M. @date 21.09.2007
///
////////////////////////////////////////////////////////////////////////
#include "CAsyncSendStream.h"
#include <boost/bind.hpp>

void CAsyncSendStream::ThreadEntryPoint()
{
TRY_CATCH
	
	MSG msg; // Creating thread's message queue
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	m_threadId = GetCurrentThreadId();
	SetEvent(m_startedEvent);

	int priority = THREAD_PRIORITY_ABOVE_NORMAL;
	if (!SetThreadPriority(GetCurrentThread(), priority))
		Log.WinError(_ERROR_,_T("Failed to set %d priority to send thread "),priority);

	while(FALSE != GetMessage(&msg,NULL,0,0))
	{
		if (m_msgSend == msg.message)
		{
			m_stream->Send(reinterpret_cast<char*>(msg.lParam),msg.wParam);
			delete [] reinterpret_cast<char*>(msg.lParam);
		} else if (m_msgSend2Queue == msg.message)
		{
			m_stream->Send2Queue(reinterpret_cast<char*>(msg.lParam),msg.wParam);
			delete [] reinterpret_cast<char*>(msg.lParam);
		} else if (m_msgFlush == msg.message)
		{
			m_stream->FlushQueue();
		}
		else
		{
			Log.Add(_ERROR_,_T("Message with wrong code(%d) received"),msg.message);
			continue;
		}
	}

CATCH_LOG()
}

CAsyncSendStream::CAsyncSendStream(boost::shared_ptr<CAbstractStream> stream)
	:	CInstanceTracker(_T("CAsyncSendStream")),
		m_stream(stream),
		m_msgSend(RegisterWindowMessage(_T("CAsyncSendStream::m_msgSend"))),
		m_msgSend2Queue(RegisterWindowMessage(_T("CAsyncSendStream::m_msgSend2Queue"))),
		m_msgFlush(RegisterWindowMessage(_T("CAsyncSendStream::m_msgFlush")))
{
TRY_CATCH
	if (NULL == m_stream.get())
		throw MCStreamException(_T("NULL == m_stream.get()"));

	if (FALSE == (m_startedEvent = CreateEvent(NULL, TRUE, FALSE, NULL)))
		throw MCStreamException_Win(_T("Failed to create event"),GetLastError());

	m_thread.reset(new boost::thread(boost::bind(&CAsyncSendStream::ThreadEntryPoint, this)));

	if (WAIT_OBJECT_0 != WaitForSingleObject(m_startedEvent, MAX_START_TIMEOUT))
		throw MCStreamException_Win(_T("Wait for thread failed"),GetLastError());

CATCH_THROW()
}

CAsyncSendStream::~CAsyncSendStream()
{
TRY_CATCH
	if (FALSE == PostThreadMessage(m_threadId, WM_QUIT, 0, 0))
		Log.WinError(_ERROR_,_T("Failed to send quit message "));
	CloseHandle(m_startedEvent);
CATCH_LOG()
}

unsigned int CAsyncSendStream::ReceiveInternal( char* buf, const unsigned int& len )
{
TRY_CATCH
	m_stream->Receive(buf,len);
	return len;
CATCH_THROW()
}

unsigned int CAsyncSendStream::SendInternal( const char* buf, const unsigned int& len )
{
TRY_CATCH
	FastSend(buf, len);
	return len;
CATCH_THROW()
}

bool CAsyncSendStream::HasInData()
{
TRY_CATCH
	return m_stream->HasInData();
CATCH_THROW()
}