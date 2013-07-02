/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CNetLogClient.cpp
///
///  NetLogClient COM object
///
///  @author Sogin Max @date 27.03.2007
///
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CNetLogClient.h"
#include <NetLog/CNetLog.h>
#include <NetLog/SLogMsg.h>
#include <boost/scoped_array.hpp>
#include <NetLog/SServerMessage.h>
#include "CLogMessage.h"

// CNetLogClient

CNetLogClient::CNetLogClient()
	:	CThread(),
		m_recentReply(cDate().GetNow()),
		m_state(DETACHED),
		m_delayedMode(false)
{
TRY_CATCH
	InitializeCriticalSection(&m_cs);
	/// Initializing singleton to avoid threding problems
	CSingleton<CNetLogFieldsMap>::instance();
CATCH_LOG()
}

CNetLogClient::~CNetLogClient()
{
TRY_CATCH
	DeleteCriticalSection(&m_cs);
CATCH_LOG()
}

STDMETHODIMP CNetLogClient::get_Name(BSTR* pVal)
{
TRY_CATCH
	USES_CONVERSION;
	CComBSTR bstr(m_name.c_str());
	*pVal = bstr.Copy();
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CNetLogClient::put_Name(BSTR newVal)
{
TRY_CATCH
	USES_CONVERSION;
	m_name = W2T(newVal);
	return S_OK;
CATCH_LOG_COMERROR()
}
STDMETHODIMP CNetLogClient::get_IP(BSTR* pVal)
{
TRY_CATCH
	USES_CONVERSION;
	CComBSTR bstr(m_clientIP.c_str());
	*pVal = bstr.Copy();
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CNetLogClient::put_IP(BSTR newVal)
{
TRY_CATCH
	USES_CONVERSION;
	m_clientIP = W2T(newVal);
	return S_OK;
CATCH_LOG_COMERROR()
}


STDMETHODIMP CNetLogClient::get_RecentReply(DATE* pVal)
{
TRY_CATCH
	SYSTEMTIME sysTime = static_cast<SYSTEMTIME>(m_recentReply);
	SystemTimeToVariantTime(&sysTime,pVal);
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CNetLogClient::put_RecentReply(DATE newVal)
{
TRY_CATCH
	m_recentReply = newVal;
	return S_OK;
CATCH_LOG_COMERROR()
}

void CNetLogClient::OnConnected( void* param )
{
TRY_CATCH
	// changing status
	SetState(ATTACHED);
	//put_DelayedMode(m_delayedMode);
	// starting internal thread
	Start();
CATCH_LOG()
}

void CNetLogClient::OnDisconnected( void* param )
{
TRY_CATCH
	// changing status
	SetState(DETACHED);
	m_stream.Close();
CATCH_LOG()
}
void CNetLogClient::OnConnectError( void* param, EConnectErrorReason reason )
{
TRY_CATCH
	// changing status
	SetState(DETACHED);
CATCH_LOG()
}

STDMETHODIMP CNetLogClient::Attach(void)
{
TRY_CATCH
	// changing status
	SetState(ATTACHING);
	// setting stream events
	m_stream.SetConnectedEvent(boost::bind( &CNetLogClient::OnConnected, this, _1 ) );
	m_stream.SetConnectErrorEvent( boost::bind( &CNetLogClient::OnConnectError, this, _1, _2 ) );
	m_stream.SetDisconnectedEvent( boost::bind( &CNetLogClient::OnDisconnected, this, _1) );
	// connecting stream
	//cMsgBoxLog().Add(_MESSAGE_,"%s:%s",m_name.c_str(),m_clientIP.c_str());
	m_stream.Connect(m_clientIP, m_TPPPort, false/*async connect*/);
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CNetLogClient::Detach(void)
{
TRY_CATCH
	// disconnecting stream
	m_stream.Disconnect();
	// stoppin internal thread
	Stop(false /*not forced stop*/);
	return S_OK;
CATCH_LOG_COMERROR()
}

void CNetLogClient::SetState(const ENetLogClientState& state)
{
TRY_CATCH
	m_state = state;
	OnStateChanged(m_state);
CATCH_THROW()
}

void CNetLogClient::Execute(void*)
{
TRY_CATCH
	ILogMessage* logMessage;
	/*{
		for(int i=0;i<100;++i)
		{
			/// Adding messages for test purposes
			Sleep(1000);
			HRESULT hr = CLogMessage::CreateInstance(&logMessage);
			if (S_OK != hr)
				Log.Add(_ERROR_,_T("Failed to CLogMessage::CreateInstance"));
			else
			{
				logMessage->put_Message(CComBSTR(Format(_T("Test log message %d\nlog test Message"),i).c_str()));
				logMessage->put_Severity(1);
				DATE date;
				SYSTEMTIME sysTime = static_cast<SYSTEMTIME>(cDate().GetNow());
				SystemTimeToVariantTime(&sysTime,&date);
				logMessage->put_AddedDate(date);
				OnLogMessage(logMessage);
			}
		}
	}*/
	SLogMsg logMsg;
	boost::scoped_array<BYTE> buf;
	while(!Terminated())
	{
		m_stream.Receive(reinterpret_cast<char*>(&logMsg), NETLOG_MESSAGE_HEADER_SIZE);
		buf.reset(new BYTE[logMsg.m_size - NETLOG_MESSAGE_HEADER_SIZE]);
		m_stream.Receive(reinterpret_cast<char*>(buf.get()), logMsg.m_size - NETLOG_MESSAGE_HEADER_SIZE);
		HRESULT hr = CLogMessage::CreateInstance(&logMessage);
		if (S_OK != hr)
			Log.Add(_ERROR_,_T("Failed to CLogMessage::CreateInstance"));
		else
		{
			logMessage->put_Severity(logMsg.m_severity);
			logMessage->Decode(buf.get(), logMsg.m_size - NETLOG_MESSAGE_HEADER_SIZE);
			/// Notifying - message received
			OnLogMessage(logMessage);
		}
	}
	return;
CATCH_LOG()
}

STDMETHODIMP CNetLogClient::get_State(SHORT* pVal)
{
TRY_CATCH
	*pVal = m_state;
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CNetLogClient::SetVerbosity(SHORT verbosity)
{
TRY_CATCH
	CCritSection cs(&m_cs);
	if (m_state != ATTACHED)
	{
		throw MCException("Not attached");
	}
	char Buf[MAX_PATH];
	SServerMessage *msg = reinterpret_cast<SServerMessage*>(Buf);
	msg->m_code = smcChangeVerbosity;
	msg->m_size = LOG_SERVER_MESSAGE_HEADER_SIZE + sizeof(eVerbosity);
	eVerbosity verb(static_cast<eVerbosity>(verbosity));
	memcpy(msg->m_data,&verb,sizeof(verb));
	m_stream.Send(reinterpret_cast<char*>(msg),msg->m_size);
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CNetLogClient::get_DelayedMode(VARIANT_BOOL* pVal)
{
TRY_CATCH
	*pVal = m_delayedMode;
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CNetLogClient::put_DelayedMode(VARIANT_BOOL newVal)
{
TRY_CATCH
	CCritSection cs(&m_cs);
	if (m_state != ATTACHED)
	{
		m_delayedMode = (newVal != 0);
		return S_OK;
	}
	SServerMessage msg;
	msg.m_code = smcChangeDelayedMode;
	msg.m_size = sizeof(msg);
	m_delayedMode = ( 0 != newVal );
	msg.m_data[0] = m_delayedMode;
	m_stream.Send(reinterpret_cast<char*>(&msg),msg.m_size);
	return S_OK;
CATCH_LOG_COMERROR()
}
STDMETHODIMP CNetLogClient::get_TCPPort(LONG* pVal)
{
TRY_CATCH
	*pVal = m_TPPPort;
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CNetLogClient::put_TCPPort(LONG newVal)
{
TRY_CATCH
	m_TPPPort = newVal;
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CNetLogClient::RequestDelayedMessages(void)
{
TRY_CATCH
	CCritSection cs(&m_cs);
	if (m_state != ATTACHED)
	{
		throw MCException("Not attached to client");
	}
	SServerMessage msg;
	msg.m_code = smcGetDelayedLog;
	msg.m_size = sizeof(msg);
	m_stream.Send(reinterpret_cast<char*>(&msg),msg.m_size);
	return S_OK;
CATCH_LOG_COMERROR();
}
