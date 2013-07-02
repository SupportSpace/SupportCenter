/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CConnectedLogSvr.cpp
///
///  Implements CConnectedLogSvr class, responsible for transportation
///    log messages to separated log server 
///
///  @author Dmitry Netrebenko @date 20.03.2007
///
////////////////////////////////////////////////////////////////////////


#ifndef _WIN32_WINNT		// Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0400		
#endif						


#include <NetLog/CConnectedLogSvr.h>
#include <AidLib/CThread/CThreadLS.h>
#include <AidLib/CCritSection/CCritSection.h>
#include <boost/bind.hpp>

CConnectedLogSvr::CConnectedLogSvr(const int id, SPAbstractStream stream)
	:	CThread()
	,	m_delayedSend(false)
	,	m_svrId(id)
	,	m_stream(stream)
{
	try
	{
		InitializeCriticalSection(&m_section);
	}
	catch(...)
	{
	}
}

CConnectedLogSvr::~CConnectedLogSvr()
{
	try
	{
		DeleteCriticalSection(&m_section);
	}
	catch(...)
	{
	}
}

void CConnectedLogSvr::Execute(void *Params)
{
	DISABLE_TRACE;
	SET_THREAD_LS;

	try
	{
		while(!Terminated())
		{

			while(m_stream->HasInData() && !Terminated())
			{
				SServerMessage header;
				// Receive message header
				m_stream->Receive(reinterpret_cast<char*>(&header), LOG_SERVER_MESSAGE_HEADER_SIZE);
				// Allocate memory for server message
				SPServerMessage msg(reinterpret_cast<SServerMessage*>(new char[header.m_size]));
				// Copy header
				memcpy(msg.get(), &header, LOG_SERVER_MESSAGE_HEADER_SIZE);
				// Receive message data
				m_stream->Receive(msg->m_data, msg->m_size - LOG_SERVER_MESSAGE_HEADER_SIZE);

				// Process server message
				OnServerMessage(msg);
			}

			if(!m_delayedSend && !Terminated())
				OnGetDelayedLog();

			// Switch to other thread
			//SwitchToThread();
			// TODO: SwitchToThread uses all CPU time
			Sleep(1);
		}
	}
	catch(...)
	{
	}
}

void CConnectedLogSvr::AddLogMsg(SPLogMsg msg)
{
	try
	{
		// Enter critical section
		CCritSection section(&m_section);
		// Add message to queue
		m_delayedMessages.push(msg);
	}
	catch(...)
	{
	}
}

void CConnectedLogSvr::OnServerMessage(SPServerMessage msg)
{
	try
	{
		// TODO: switch server message type
		switch(msg->m_code)
		{
		case smcChangeDelayedMode:
			OnChangeDelayedMode(static_cast<bool>(0 != *msg->m_data));
			break;

		case smcGetDelayedLog:
			OnGetDelayedLog();
			break;

		case smcChangeVerbosity:
			eVerbosity verbosity;
			memcpy(&verbosity, msg->m_data, sizeof(eVerbosity));
			OnChangeVerbosity(verbosity);
			break;

		default:
			;
		}
	}
	catch(...)
	{
	}
}

void CConnectedLogSvr::OnStreamDisconnected(void*)
{
	try
	{
		RaiseDisconnectedEvent();
	}
	catch(...)
	{
	}
}

LogSvrEvent CConnectedLogSvr::GetLogSvrEvent() const
{
	return m_event;
}

void CConnectedLogSvr::SetLogSvrEvent(LogSvrEvent handler)
{
	try
	{
		m_event = handler;
	}
	catch(...)
	{
	}
}

void CConnectedLogSvr::RaiseDisconnectedEvent()
{
	try
	{
		if(m_event)
			m_event(m_svrId, clseDisconnect, NULL);
	}
	catch(...)
	{
	}
}

void CConnectedLogSvr::OnChangeDelayedMode(const bool mode)
{
	try
	{
		m_delayedSend = mode;
	}
	catch(...)
	{
	}
}

void CConnectedLogSvr::OnGetDelayedLog()
{
	try
	{
		// Enter critical section
		CCritSection section(&m_section);

		// Send all messages to server
		while(!m_delayedMessages.empty() && !Terminated())
		{
			SPLogMsg msg = m_delayedMessages.front();
			m_stream->Send(reinterpret_cast<char*>(msg.get()), msg->m_size);
			m_delayedMessages.pop();
		}
	}
	catch(...)
	{
	}
}

int CConnectedLogSvr::GetSvrId() const
{
	return m_svrId;
}

void CConnectedLogSvr::OnChangeVerbosity(eVerbosity verbosity)
{
	try
	{
		if(m_event)
		{
			void* param = &verbosity;
			m_event(m_svrId, clseChangeVerbosity, &verbosity);
		}
	}
	catch(...)
	{
	}
}
