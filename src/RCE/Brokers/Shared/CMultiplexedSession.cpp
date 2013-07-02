/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CMultiplexedSession.cpp
///
///  CMultiplexedSessions object implementation. (Session (connection) object)
///
///  @author Kirill Solovyov @date 05.11.2007
///
////////////////////////////////////////////////////////////////////////

//for objbase.h
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

#include "CMultiplexedSession.h"
#include <NWL/Streaming/CNetworkLayer.h>

#include <NWL/Streaming/CSocketSystem.h>
#include <NWL/Statistic/CMeasurement.h>
#include <NWL/TLS/CTLSSystem.h>
CSocketSystem sockSystem;
CTLSSystem tlsSystem;

#include <AidLib/CThread/CThreadLS.h>

#include <objbase.h>


#define SERVICE_SUBSTREAM 1L
#define SERVICE_SUBSTREAM_PRIORITY 0x00ffffff
/// reconnect attempt count
#define WDTDEFAULTCOUNT 10

//------------------------------------------------------------------------------------------------------------------------------------
//CLiveThread
CLiveThread::CLiveThread(void *_Param, const bool createMessageQueue, DWORD livePeriod):
	CThread(_Param,createMessageQueue),
	m_livePeriod(livePeriod),
	m_wakeEvent(FALSE,FALSE),
	CInstanceTracker(_T("CLiveThread"))
{
}
CLiveThread::~CLiveThread()
{
TRY_CATCH
	Terminate();
	if(!m_wakeEvent.Set())
		MLog_Exception(MCException_Win("Wake up event set failed"));
CATCH_LOG()
}

void CLiveThread::Execute(void *Params)
{
	SET_THREAD_LS;
TRY_CATCH
	CMultiplexedSession *_this=reinterpret_cast<CMultiplexedSession*>(Params);
	MeasurementPoint mPoint(0,0);
	boost::shared_ptr<SData> pingMsg(reinterpret_cast<SData*>(new char[sizeof(SData)+sizeof(MeasurementPoint)]));
	while(_TERMINATING!=State)
	{
		pingMsg->m_size=sizeof(MeasurementPoint);//ping message
		mPoint.fState = TRUE; // Start of measurement
		mPoint.dTimeStampSend = GetTickCount();
		CopyMemory(&(pingMsg->m_data),&mPoint,sizeof(MeasurementPoint));
		_this->innerSend(pingMsg);
		DWORD result=WaitForSingleObject(m_wakeEvent,m_livePeriod);
		if(WAIT_TIMEOUT!=result&&WAIT_OBJECT_0!=result)
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("WaitForSingleObject return with code=0x%x"),result),GetLastError());
	}
CATCH_LOG()
}



//------------------------------------------------------------------------------------------------------------------------------------
//CStreamMultiplexerBaseImpl

CStreamMultiplexerBaseImpl::CStreamMultiplexerBaseImpl(boost::shared_ptr<CAbstractStream> transportStream, CMultiplexedSession* owner):
	CStreamMultiplexerBase(transportStream),
	m_owner(owner),
	CInstanceTracker(_T("CStreamMultiplexerBaseImpl"))
{
TRY_CATCH
CATCH_LOG()
}

int CStreamMultiplexerBaseImpl::OnSubStreamConnected(unsigned int serviceID)
{
TRY_CATCH
	if(!m_owner)
		throw MCException("Owner has not been set");
	if(SERVICE_SUBSTREAM==serviceID)
	{
		//TODO???
		CCritSection cs(&m_owner->m_cs);
		m_owner->m_svcStreamConnectedEvent.Set();
	}
	else
	{
		m_owner->OnSubStreamTaken(serviceID);
	}
CATCH_LOG()
	return 0;
}

int CStreamMultiplexerBaseImpl::OnSubStreamDisconnected(unsigned int serviceID)
{
TRY_CATCH
	if(!m_owner)
		throw MCException("Owner has not been set");
CATCH_LOG()
	return 0;
}

int CStreamMultiplexerBaseImpl::OnConnectionBroke(boost::shared_ptr<CAbstractStream> transportStream)
{
TRY_CATCH
	if(!m_owner)
		throw MCException("Owner has not been set");
CATCH_LOG()
	return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------
//CMultiplexedSession

CMultiplexedSession::CMultiplexedSession(CSessionsMgr* owner):
	m_owner(owner),
	m_innerState(false),
	CThread(),
	m_svcStreamConnectedEvent(false/*ManualReset*/, false /*InitialState*/),
	m_lastSubStreamId(SERVICE_SUBSTREAM)
{
TRY_CATCH
	MEASUREMENT_INFO_INSTANCE.SetLifePeriod(DEFAULT_ALIVEPERIOD);
CATCH_LOG()
}


CMultiplexedSession::~CMultiplexedSession(void)
{
TRY_CATCH
	if(m_innerState)
	{
		ReportConnectionSpeed();
		Disconnect();
	}
	//m_svcStreamConnectedEvent.Set();
	//join()
	if(_RUNNING == State||_PAUSED == State)
	{
		//Log.Add(_MESSAGE_,_T("CMultiplexedSession::~CMultiplexedSession _RUNNING == State||_PAUSED == State"));
		Terminate();
		WaitForSingleObject(hTerminatedEvent.get(),INFINITE);
	}
	if(m_muxStream.get())
		{
			m_muxStream->m_owner=NULL;
			m_muxStream.reset();
		}
	Log.Add(_MESSAGE_,_T("CMultiplexedSession::~CMultiplexedSession()"));
CATCH_LOG()
}

void CMultiplexedSession::ConnectCompletion(boost::shared_ptr<CAbstractNetworkStream> stream)
{
TRY_CATCH
	if (stream.get())
	{
		Log.Add(_MESSAGE_,_T("CMultiplexedSession::ConnectCompletion(TRUE)"));
		m_transportStream=stream;
		m_muxStream = CStreamMultiplexerBase::GetInstance<CStreamMultiplexerBaseImpl>(m_transportStream);
		m_muxStream->m_owner=this;
		CCritSection cs(&m_cs);
		m_svcStream = m_muxStream->GetSubStream(SERVICE_SUBSTREAM,SERVICE_SUBSTREAM_PRIORITY);
		//m_svcStreamConnectedEvent.Set();
	}
	else
	{
		Log.Add(_MESSAGE_,_T("CMultiplexedSession::ConnectCompletion(FALSE) %s"),m_error.c_str());
		m_svcStreamConnectedEvent.Set();

	}
CATCH_THROW()
}

void CMultiplexedSession::Init(const tstring& relaySrv, const tstring& sId, const tstring& user, const tstring& passwd, const tstring& remoteUser, unsigned int timeOut,bool masterRole)
{
TRY_CATCH
	if(m_innerState)
		throw MCException("Multiplexed Session has already in connect state");
	m_params.m_relaySrv=relaySrv;
	m_params.m_sId=sId;
	m_params.m_user=user;
	m_params.m_passwd=passwd;
	m_params.m_remoteUser=remoteUser;
	m_params.m_timeOut=timeOut;
	m_params.m_masterRole=masterRole;
CATCH_THROW()
}

void CMultiplexedSession::Connect(void)
{
TRY_CATCH
	if(m_innerState)
		throw MCException("Multiplexed Session has already in connect state");
	m_innerState=true;

	Start();
CATCH_THROW()
}

void CMultiplexedSession::Disconnect(void)
{
TRY_CATCH
	if(!m_innerState)
		throw MCException("Multiplexed Session has not yet in connect state");
	m_innerState=false;
	AbortConnect();
	if(m_transportStream.get()&&m_transportStream->Connected())
		m_transportStream->Disconnect();
	{
		CCritSection cs(&m_csOfflineMessages);
		while(!m_offlineMessages.empty())
			m_offlineMessages.pop();
	}
	m_svcStreamConnectedEvent.Set();
	m_svcStreamConnectedEvent.Reset();
	DWORD dAvgConnectionSpeed = MEASUREMENT_INFO_INSTANCE.GetPerformance();
	Log.Add(_MESSAGE_,_T("CMultiplexedSession::Disconnect (Avarage connection speed %d bytes)"),dAvgConnectionSpeed);
CATCH_THROW()
}

void CMultiplexedSession::innerSend(boost::shared_ptr<SData> data)
{
TRY_CATCH
			Log.Add(_MESSAGE_,_T("CMultiplexedSession::innerSend(%d bytes)"),sizeof(*data)+data->m_size);
			m_svcStream->Send(reinterpret_cast<char*>(data.get()),sizeof(*data)+data->m_size);
CATCH_THROW()
}

ESessionSentMessageState CMultiplexedSession::Send(boost::shared_ptr<SData> data)
{
TRY_CATCH
	if(m_svcStream.get())
	{
		TRY_CATCH
			innerSend(data);
				return SSMS_SENT;
		CATCH_LOG()
	}
	CCritSection cs(&m_csOfflineMessages);
	m_offlineMessages.push(data);
	return (m_innerState)?SSMS_ENQUEUE_CONNECTING:SSMS_ENQUEUE;
CATCH_THROW()
}

void CMultiplexedSession::OnReceived(boost::shared_ptr<SData> data)
{
TRY_CATCH
	if(!m_owner)
		throw MCException("Owner has not been set");
	m_owner->OnRequestReceived(m_params.m_remoteUser,data);
CATCH_LOG()
}

void CMultiplexedSession::ReportConnectionSpeed()
{
TRY_CATCH
	CStreamFactoryRelayedImpl::ReportConnectionSpeed();
CATCH_LOG()
}

void CMultiplexedSession::OnConnecting(const tstring& message)
{
TRY_CATCH
	if(!m_owner)
		throw MCException("Owner has not been set");
	m_owner->OnConnecting(m_params.m_remoteUser,message);
CATCH_THROW()
}

void CMultiplexedSession::InitExchanger(const tstring& sourcePeerId, const tstring& destPeerId, int timeOut, bool masterRole)
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("CMultiplexedSession::InitExchanger"));
CATCH_THROW()
}

bool CMultiplexedSession::IsMasterRole(void)
{
TRY_CATCH
	return m_params.m_masterRole;
CATCH_THROW()
}

unsigned int CMultiplexedSession::GetSubStream(unsigned int subStreamId, unsigned int priorityLevel)
{
TRY_CATCH
	CCritSection cs(&m_cs);
	if(!m_muxStream.get())
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Session with userId=%s has not yet been connected"),m_params.m_remoteUser.c_str());
	if(SUBSTREAMID_AUTOSET!=subStreamId)
	{
		if(m_askedStreams.find(subStreamId)!=m_askedStreams.end())
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Stream with userId=%s serviceID=0x%x has been asked already"),m_params.m_remoteUser.c_str(),subStreamId);
		boost::shared_ptr<CAbstractStream> stream=m_muxStream->GetSubStream(subStreamId,priorityLevel);
		m_askedStreams[subStreamId]=stream;
		m_lastSubStreamId=subStreamId;
	}
	else
	{
		boost::shared_ptr<CAbstractStream> stream;
		unsigned int id;
		for(id=m_lastSubStreamId+1;id!=m_lastSubStreamId;++id)
		{
			if(SUBSTREAMID_AUTOSET==id)
				id=SERVICE_SUBSTREAM+1;
			try
			{
				stream=m_muxStream->GetSubStream(id,priorityLevel);
			}
			catch(CStreamException& e)
			{
				continue;
			}
			break;
		}
		if(id==m_lastSubStreamId)
			throw MCException("Free sub stream id does not exist more");
		m_askedStreams[id]=stream;
		m_lastSubStreamId=id;
	}
	return m_lastSubStreamId;
CATCH_THROW()
}

void CMultiplexedSession::OnSubStreamTaken(unsigned int subStreamId)
{
TRY_CATCH
	boost::shared_ptr<CAbstractStream> stream;
	{
		CCritSection cs(&m_cs);
		if(m_askedStreams.find(subStreamId)==m_askedStreams.end())
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Stream with remoteUser=%s serviceID=0x%x was not asked"),m_params.m_remoteUser.c_str(),subStreamId);
		stream=m_askedStreams[subStreamId];
		m_askedStreams.erase(subStreamId);
	}
	m_owner->OnSubStreamTaken(m_params.m_remoteUser,subStreamId,stream);
CATCH_THROW()
}

ESessionState CMultiplexedSession::GetSessionState(void)
{
TRY_CATCH
	if(!m_innerState)
	{
		return SS_DISCONNECTED;
	}
	else
	{
		return (m_svcStream.get())?SS_CONNECTED:SS_CONNECTING;
	}
CATCH_THROW()
}

void CMultiplexedSession::Execute(void *Params)
{
	SET_THREAD_LS;
TRY_CATCH
	HRESULT result;
	// TODO CoUninitializeEx
	if(S_OK!=(result=CoInitializeEx(NULL,COINIT_MULTITHREADED)))
		MLog_Exception(CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("CoInitializeEx() failed")),result));

	//TODO remove
	int WDTCounter=WDTDEFAULTCOUNT;
	while(State!=_TERMINATING)
	{
		//ATTATION WDT TURNED OFF
		if(false&&WDTCounter<0)
		{
			{
				CCritSection cs(&m_csOfflineMessages);
				while(!m_offlineMessages.empty())
					m_offlineMessages.pop();
			}
			m_innerState=false;
			Log.Add(_ERROR_,_T("WDTCOUNTER IS NULL, reconnection is stoped, offline messages were removed"));
			return;
		}
		try
		{
			//initial exchenger stream synchronous connection
			Log.Add(_MESSAGE_,_T("INITIAL CONNECT"));
			TRY_CATCH
				//OnConnecting(_T("Initial connection"));
			CATCH_LOG()

			NWL_INSTANCE.SetRelayHost(m_params.m_relaySrv);
			m_initialStream.SetRelayServer(m_params.m_relaySrv,NWL_INSTANCE.GetRelayTCPPort(),m_params.m_user+m_params.m_sId+_T("_ie"),NWL_INSTANCE.GetRelayPasswd());
			m_initialStream.SetConnectionId(m_params.m_sId,m_params.m_user+m_params.m_sId+_T("_ie"),m_params.m_remoteUser+m_params.m_sId+_T("_ie"));
			m_initialStream.SetConnectTimeout(m_params.m_timeOut);//TODO think about: this time out is general not for initial exchenger connection
			//TODO m_initialStream.SetConnectErrorEvent(EConnectErrorReason)
			m_initialStream.Connect();
		}
		catch(CStreamException& e)
		{
			MLog_Exception(e);
			// if not relay timeout wait and try again
			if(1/*timeout*/!=e.GetInternalErrorCode()&&m_innerState)
			{
				TRY_CATCH
					OnConnecting(_T("Initial connection problem - relay server connection timeout"));
				CATCH_LOG()
				WaitForSingleObject(m_svcStreamConnectedEvent,m_params.m_timeOut);
			}
			if(m_innerState)
			{
				if(m_initialStream.Connected())
					m_initialStream.Close();
				--WDTCounter;
				continue;
			}
			else
					return;
		}
		//StreamFactory aynchronous connection use initial exchenger stream.
		//After StreamFactory has connected,Stream Multiplexer is initialized and service stream is get.
		//see CMultiplexedSession::ConnectCompletion()
		Log.Add(_MESSAGE_,_T("FACTORY CONNECT"));
		TRY_CATCH
			//OnConnecting(_T("Connection"));
		CATCH_LOG()

		if(m_muxStream.get())
		{
			m_muxStream->m_owner=NULL;
			m_muxStream.reset();
		}
		m_svcStream.reset();
		NWL_INSTANCE.SetRelayHost(m_params.m_relaySrv);
		
		m_strServerUserId = m_params.m_user+m_params.m_sId;
		//SetServerUserId(m_params.m_user);

		SetServerPassword(m_params.m_passwd);
		CStreamFactoryRelayedImpl::Connect(m_params.m_sId,m_params.m_user+m_params.m_sId,m_params.m_remoteUser+m_params.m_sId,m_params.m_timeOut,m_params.m_masterRole,true/*async*/);
		
		//TODO think about INFINITE
		//if(WAIT_OBJECT_0!=WaitForSingleObject(m_svcStreamConnectedEvent,INFINITE))
		if(WAIT_OBJECT_0!=WaitForSingleObject(m_svcStreamConnectedEvent,m_params.m_timeOut+m_params.m_timeOut/2))
			//MLog_Exception(MCException_Win("WaitForSingleObject failed"));
			throw MCException_Win("WaitForSingleObject failed");

		//connection checking
		if(_TERMINATING==State)
			return;
		try
		{
			if(!m_svcStream.get())
				throw MCStreamException("Service stream does not exist");
			//send all sent offline messages
			{
				CCritSection cs(&m_csOfflineMessages);
				while(!m_offlineMessages.empty())
				{
					boost::shared_ptr<SData> data;
					data=m_offlineMessages.front();
					//m_svcStream->Send(reinterpret_cast<char*>(data.get()),sizeof(*data)+data->m_size);
					innerSend(data);
					m_offlineMessages.pop();
				}
			}
			/// successful connect, reset WDT counter
			WDTCounter=WDTDEFAULTCOUNT;
			
			boost::scoped_ptr<CLiveThread> liveThread(new CLiveThread(this,false,m_params.m_timeOut/2));
			liveThread->Start();

			while(_TERMINATING!=State)
			{
				DWORD msgLen;
				do
				{
					m_svcStream->Receive(reinterpret_cast<char*>(&msgLen),sizeof(msgLen));
				}
				while(msgLen==0);//live message
				boost::shared_ptr<SData> buf;
				const DWORD dataLen=msgLen;
				if(0<dataLen)
				{
					TRY_CATCH
						//Log.Add(_MESSAGE_,_T("Allocate memory dataLen=%d sizeof(SData)=%d"),sizeof(SData),dataLen);
						buf.reset(reinterpret_cast<SData*>(new char[sizeof(SData)+dataLen]));
					CATCH_LOG()
					buf->m_size=dataLen;
					//Log.Add(_MESSAGE_,_T("before receive. dataLen=%d + sizeof(SData)=%d"),dataLen,sizeof(SData));
					//m_svcStream->Receive(buf->m_data,dataLen);
					m_svcStream->Receive(buf->m_data,dataLen);
					MeasurementPoint &mPoint = reinterpret_cast<MeasurementPoint&>(buf->m_data);
					if (dataLen == sizeof (MeasurementPoint) && mPoint.dHeader == PING_PONG_MARK)
					{
						if (mPoint.fState == TRUE)
						{
							mPoint.fState = FALSE; // PONG End of measurement
							innerSend(buf);
							continue;
						} else
						{
							mPoint.dTimeStampReceived = GetTickCount();
							MEASUREMENT_INFO_INSTANCE.AddSample(mPoint);
							Log.Add(_MESSAGE_,_T("PING msgLen=%d"),msgLen);
							continue;
						}

					} else
						Log.Add(_MESSAGE_,_T("Ping Message size error"));
					 


					//Log.Add(_MESSAGE_,_T("after receive"));
				}
				try
				{
					OnReceived(buf);
				}
				catch(CExceptionBase &e)
				{
					MLog_Exception(e);
				}
			}
		}
		catch(CStreamException& e)
		{
			MLog_Exception(e);
			if(m_transportStream.get()&&m_transportStream->Connected())
				m_transportStream->Disconnect();
			if(m_innerState)
			{
				--WDTCounter;
				// Sending requests to services, indicating NWL failure
				if (!Terminated() && NULL != m_owner)
				{
					CRequestsMgr* requestMgr = m_owner->GetRequestManager();
					if (!Terminated() && NULL != requestMgr)
					{
						requestMgr->OnNWLDisconnect(m_params.m_remoteUser);
					}
				}
				continue;
			}
			else
				return;
		}
	}
CATCH_LOG()
}
