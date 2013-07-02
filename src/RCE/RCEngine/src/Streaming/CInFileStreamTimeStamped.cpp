/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CInFileStreamTimeStamped.cpp
///
///  File output stream timestamped
///
///  @author "Archer Software" Sogin M. @date 04.10.2006
///
////////////////////////////////////////////////////////////////////////

#include "CInFileStreamTimeStamped.h"
#include "SBlock.h"
#include <AidLib/CCritSection/CCritSection.h>
#include <memory>
#include <AidLib/Logging/cLog.h>
#include "rfb.h"
#include "PermissibleWarnings.h"
#include <AidLib/CThread/CThreadLS.h>

CInFileStreamTimeStamped::CInFileStreamTimeStamped(const tstring &fileName)
	:	CThread(), 
		m_EOF(false), 
		m_state(STREAM_RUNNING),
		m_delayFactor(1),
		m_readCancelled(false)
{
TRY_CATCH

	m_hFile = CreateFile(	fileName.c_str(),
							GENERIC_READ,
							FILE_SHARE_READ,
							NULL,
							OPEN_EXISTING,
							0,
							NULL );
	if (m_hFile == INVALID_HANDLE_VALUE)
		throw MCStreamException_Win("failed to CreateFile",GetLastError());

	m_hFile = NULL;
    InitializeCriticalSection(&m_getCs);
	InitializeCriticalSection(&m_cs);
	m_dataReadEvent = CreateEvent(NULL,true,FALSE,NULL);
	if (m_dataReadEvent == NULL)
		throw MCStreamException_Win("failed to CreateEvent",GetLastError());

CATCH_THROW("CInFileStreamTimeStamped::CInFileStreamTimeStamped")
}

/// Protected constructor for successors
CInFileStreamTimeStamped::CInFileStreamTimeStamped()
	:	CThread(),
		m_EOF(false),
		m_state(STREAM_RUNNING),
		m_delayFactor(1),
		m_readCancelled(false)
{
TRY_CATCH

	m_hFile = NULL;
    InitializeCriticalSection(&m_getCs);
	InitializeCriticalSection(&m_cs);
	m_dataReadEvent = CreateEvent(NULL,true,FALSE,NULL);
	if (m_dataReadEvent == NULL)
		throw MCStreamException_Win("failed to CreateEvent",GetLastError());

CATCH_THROW("CInFileStreamTimeStamped::CInFileStreamTimeStamped")
}

CInFileStreamTimeStamped::~CInFileStreamTimeStamped(void)
{
TRY_CATCH

	CThread::Stop(false);
	DeleteCriticalSection(&m_cs);
	DeleteCriticalSection(&m_getCs);
	CloseHandle(m_hFile);
	CloseHandle(m_dataReadEvent);

CATCH_LOG("CInFileStreamTimeStamped::~CInFileStreamTimeStamped")
}

unsigned int CInFileStreamTimeStamped::ReceiveInternal( char* buf, const unsigned int& len )
{
TRY_CATCH
	
	CCritSection getCs(&m_getCs);
	//char buf [MAX_PATH];
	//itoa(GetCurrentThreadId(),buf,10);
	//OutputDebugString(buf);
	{
		CCritSection cs(&m_cs);
		if (m_EOF && (m_queue.size() < len))
			throw MCStreamException("EOF");
		//Starting thread if it's first call
		if (State != _RUNNING) 
		{
			CThread::Start();
		}
	}
	/// Waiting while we have as much data as we wanna to
	while(true)
	{
		CheckCancelled();
		ResetEvent(m_dataReadEvent);
		{
			CCritSection cs(&m_cs);
			if (m_queue.size() >= len) 
				break;
			else
				if (m_EOF)
					throw MCStreamException("EOF");
		}
		WaitForSingleObject(m_dataReadEvent, INFINITE);
	}
	{
		/// Reading data
		if (len != GetInBuffer(buf,len))
			throw MCStreamException("internal error (len != GetInBuffer(buf,len))");
	}
	return len;

CATCH_THROW("CInFileStreamTimeStamped::ReceiveInternal")
}

bool CInFileStreamTimeStamped::HasInData()
{
TRY_CATCH

	CCritSection getCs(&m_getCs);
	CCritSection cs(&m_cs);
	CheckCancelled();
	//if (m_EOF) throw MCStreamException("EOF");
	//Starting thread if it's first call
	if (State != CThread::_RUNNING)
	{
		CThread::Start();
	}
	return !m_queue.empty();

CATCH_THROW("CInFileStreamTimeStamped::HasInData")
}

unsigned int CInFileStreamTimeStamped::GetInBuffer( char* buf, const unsigned int& len )
{
TRY_CATCH
	
	CCritSection getCs(&m_getCs);
	CCritSection cs(&m_cs);
	CheckCancelled();
	if (m_EOF && (m_queue.size() < len))
		throw MCStreamException("EOF");
	//Starting thread if it's first call
	if (State != CThread::_RUNNING)
	{
		CThread::Start();
	}
	if (m_queue.empty()) return 0;
	int res = min(m_queue.size(),len);
	memcpy(buf,&m_queue[0],res);
	m_queue.erase(m_queue.begin(),m_queue.begin() + res);
	return res;

CATCH_THROW("CInFileStreamTimeStamped::GetInBuffer")
}

void CInFileStreamTimeStamped::Execute(void*)
{
	SET_THREAD_LS;

TRY_CATCH

	if (m_EOF) return;
	DWORD size;
	while(!Terminated())
	{	
		if (m_cs1Exists && m_state != STREAM_RUNNING && TryEnterCriticalSection(m_cs1))
		{
			LeaveCriticalSection(m_cs1);
			for(;m_state != STREAM_RUNNING && !Terminated();)
				Sleep(100);
			if (Terminated()) 
				break;
		}

		RealGet(reinterpret_cast<char*>(&size),sizeof(size));
		if (size <= 0) break;
		std::auto_ptr<SBlock> block = std::auto_ptr<SBlock> (reinterpret_cast<SBlock*>(new char[size]));
		block->size = size;
		RealGet(reinterpret_cast<char*>(block.get())+sizeof(size), size-sizeof(size));
		switch(block->type)
		{
			case TIMESTAMP:
				//Making delay
				if (m_delayFactor != 0)
				{
					int delay = *reinterpret_cast<DWORD*>(block->buf) * m_delayFactor;
					if (delay > 15000) delay=1; //TODO: WTF? How can this occures?
					Sleep(delay?delay:1);
				}
				break;
			case DATA:
				//Posting data to queue
				{
					CCritSection cs(&m_cs);
					int oldSize = m_queue.size();
					m_queue.resize(oldSize + block->size - BLOCK_HEAD_SIZE);
					memcpy(&m_queue[oldSize],block->buf,block->size - BLOCK_HEAD_SIZE);
					SetEvent(m_dataReadEvent);
				}
				break;
			default:
				throw MCStreamException("Wrong block type");

		}
	}
CATCH_LOG("CInFileStreamTimeStamped::Execute")
	m_EOF = true;
	SetEvent(m_dataReadEvent);
}

/// Really gets buffer from file
/// this method can be redeffined to perform compressed input
/// @param buf buffer for transfer
/// @len length of buffer
void CInFileStreamTimeStamped::RealGet( char* buf, const unsigned int &len)
{
TRY_CATCH
	DWORD szread;
	if (!ReadFile(m_hFile,buf,len,&szread,0))
		throw MCStreamException_Win("Failed to ReadFile",GetLastError());
CATCH_THROW("CInFileStreamTimeStamped::RealGet")
}

unsigned int CInFileStreamTimeStamped::SendInternal( const char*, const unsigned int& len )
{
TRY_CATCH
	CCritSection getCs(&m_getCs);
	/// And nothing more
	return len;
CATCH_THROW("CInFileStreamTimeStamped::SendInternal")
}

void CInFileStreamTimeStamped::ResetFilePos()
{
TRY_CATCH
	if (SetFilePointer(m_hFile,0,0,FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		throw MCStreamException_Win("Failed to SetFilePointer",GetLastError());
CATCH_THROW("CInFileStreamTimeStamped::ResetFilePos")
}

/// Starts stream if it was stopped or paused
void CInFileStreamTimeStamped::Start()
{
TRY_CATCH
	CCritSection getCs(&m_getCs);
	CCritSection cs(&m_cs);
	switch(m_state)
	{
		/*case STREAM_RUNNING:*/
		case STREAM_STOPPED:
			m_queue.clear();
			/// Reseting position in file
			ResetFilePos();
			/// А вот теперь самое интересное
			{
				/// Reading 3 data blocks
				DWORD size;
				for(int i=0; i<3; )
				{
					RealGet(reinterpret_cast<char*>(&size),sizeof(size));
					if (size <= 0) 
						throw MCException("size <= 0");
					std::auto_ptr<SBlock> block = std::auto_ptr<SBlock> (reinterpret_cast<SBlock*>(new char[size]));
					block->size = size;
					RealGet(reinterpret_cast<char*>(block.get())+sizeof(size), size-sizeof(size));
					if (block->type == DATA)
						++i;
				}
				/// Posting rfbResetStreams to queue
				m_queue.push_back(rfbResetStreams);
			}
		}	
		
	/// Starting 
	m_state = STREAM_RUNNING;
CATCH_THROW("CInFileStreamTimeStamped::Start")
}

/// Stops stream if it was running or paused
void CInFileStreamTimeStamped::Stop()
{
TRY_CATCH
	CCritSection getCs(&m_getCs);
	CCritSection cs(&m_cs);
	/// Stopping
	m_state = STREAM_STOPPED;
CATCH_THROW("CInFileStreamTimeStamped::Stop")
}

void CInFileStreamTimeStamped::Pause()
{
TRY_CATCH
	CCritSection getCs(&m_getCs);
	CCritSection cs(&m_cs);
	/// Pausing
	m_state = STREAM_PAUSED;
CATCH_THROW("CInFileStreamTimeStamped::Pause")
}

void CInFileStreamTimeStamped::SetDelayFactor(float delayFactor)
{
TRY_CATCH
	m_delayFactor = delayFactor;
CATCH_THROW("CInFileStreamTimeStamped::SetDelayFactor")
}

void CInFileStreamTimeStamped::CancelReceiveOperation()
{
TRY_CATCH
	CCritSection cs(&m_cs);
	m_readCancelled = true;
	SetEvent(m_dataReadEvent);
CATCH_THROW("CInFileStreamTimeStamped::CancelReceiveOperation")
}