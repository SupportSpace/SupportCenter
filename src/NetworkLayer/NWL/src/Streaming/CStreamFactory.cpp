/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CStreamFactory.cpp
///
///  Steam factory
///
///  @author "Archer Software" Sogin M. @date 20.11.2006
///
////////////////////////////////////////////////////////////////////////

#include <NWL/Streaming/CExternalIPRequest.h>
#include <NWL/Streaming/CDirectNetworkStream.h>
#include <NWL/Streaming/CNetworkLayer.h>
#include <NWL/Streaming/CNATTraversingUDPNetworkStream.h>
#include <NWL/Streaming/CRelayedNetworkStream.h>
#include <NWL/Streaming/CStreamFactory.h>
#include <AidLib/CCritSection/CCritSection.h>
#include <AidLib/Localization/CLocale.h>
#include <AidLib/Logging/cLog.h>
#include <NWL/Streaming/CFirewallConfigurator.h>
#include <AidLib/CSingleton/CSingleton.h>
#include <NWL/Statistic/CStatisticClient.h>
#include <NWL/Statistic/SStatisticMessage.h>
#include <NWL/Statistic/CMeasurement.h>

std::map<UINT_PTR,CStreamFactory*> CStreamFactory::m_fabricTimersMap;

CStreamFactory::CStreamFactory()
	:	CInstanceTracker(_T("StreamFactory")),
		m_conenctAborted(false),
		m_strServerUserId( _T("") ),
		m_strServerPassword( _T("") ),
		m_waitingForCompletion(false)
{
TRY_CATCH
	InitializeCriticalSection(&m_cs);
	InitializeCriticalSection(&m_streamEventsCS);
	// Create event object
	m_hConnectEvent = CreateEvent(	NULL,
									TRUE,
									FALSE,
									NULL );
 
	if ( !m_hConnectEvent )
		throw MCStreamException( _T("Event creation failed") );

	TRY_CATCH
		// Configure firewall for incoming connections
		FIREWALL_CONFIGURATOR_INSTANCE.AllowIncoming();
	CATCH_LOG("CSocketStream::CSocketStream. Failed to allow incomming connects for windows firewall")

	TRY_CATCH
		// Acquiring NWL process singleton instance to avoid threading issues
		NWL_INSTANCE;
	CATCH_LOG("Failed to acquire NWL process singleton instance")

CATCH_THROW("CStreamFactory::CStreamFactory")
}

CStreamFactory::~CStreamFactory()
{
TRY_CATCH

	if (NULL != m_threadStarter.get())
	{
		Log.Add(_MESSAGE_,_T("Stream factory: destroying connect thread"));
		m_threadStarter.reset();
	}
	DeleteCriticalSection(&m_cs);
	DeleteCriticalSection(&m_streamEventsCS);
	if (m_hConnectEvent) 
		CloseHandle(m_hConnectEvent);

CATCH_LOG("CStreamFactory::~CStreamFactory")
}

HANDLE CStreamFactory::GetHandleMsgEvent()
{
TRY_CATCH
	return NULL;
CATCH_THROW("CStreamFactory::GetHandleMsgEvent")
}

void CStreamFactory::AbortConnect()
{
TRY_CATCH
	m_conenctAborted = true;
	CallCompletion(boost::shared_ptr<CAbstractNetworkStream>(reinterpret_cast<CAbstractNetworkStream*>(NULL)),MLocalize(_T("Connect aborted")));
	AbortExchanger();
	SetEvent(m_hConnectEvent);
	//CallCompletion(boost::shared_ptr<CAbstractNetworkStream>(reinterpret_cast<CAbstractNetworkStream*>(NULL)),MLocalize(_T("Connect aborted")));
CATCH_THROW("CStreamFactory::AbortConnect")
}

void CStreamFactory::CheckAborted()
{
	if (m_conenctAborted)
	{
		throw MCStreamException(MLocalize(_T("Connect aborted")));
	}
}

void CStreamFactory::SomeStreamConnected(void* stream)
{
TRY_CATCH
	CCritSection cs(&m_streamEventsCS);
	/// Checking if we already connected
	if (m_connectedStream == NULL)
	{
		m_connectedStream = stream;
		SetEvent(m_hConnectEvent);
	}
CATCH_LOG()
}

void CStreamFactory::SomeStreamFailed2Connect(void*, EConnectErrorReason)
{
TRY_CATCH
	/// Doing nothing - normal case
CATCH_LOG()
}

void UnpackAddress(tstring& address, int& port)
{
TRY_CATCH
	size_t pos;
	int defPort(port);
	if (tstring::npos != (pos = address.find_last_of(_T(":"))))
	{
		tstring _port = address.substr(pos+1);
		port = _ttoi(_port.c_str());
		if (port != 0)
			address.erase(pos);
		else
		{
			/// Invalid address format - left 'as is'
			port = defPort;
			Log.Add(_WARNING_,_T("Address (%s) has invalid format"),address.c_str());
		}
	}
CATCH_THROW()
}

tstring PackAddress(const tstring& address, const int port)
{
TRY_CATCH
	return Format(_T("%s:%d"),address.c_str(),port);
CATCH_THROW()
}

void CStreamFactory::DirectConnect(	const tstring& connectId, 
									const tstring& secret, 
									const tstring& _remoteAddress,
									const int timeOut,
									boost::shared_ptr<CDirectNetworkStream>& directStream )
{
TRY_CATCH
	if (WaitForSingleObject(m_hConnectEvent, NET_DELAY_ORDER) == WAIT_OBJECT_0)
	{
		// Already connected
		return;
	}
	tstring remoteAddress(_remoteAddress);
	int remotePort = NWL_INSTANCE.GetDirectStreamPort();
	NotifyProgress(50,Format(_T("%s %s"),MLocalize(_T("Starting direct connect to")).c_str(),remoteAddress.c_str()));
	Log.Add(_MESSAGE_,_T("Starting direct connect to %s"),remoteAddress.c_str());
	/// Separating remota address from remote port
	UnpackAddress(remoteAddress,remotePort);
	directStream.reset(new CDirectNetworkStream());
	STLSCredentials TLSsecret;
	TLSsecret.UserID = connectId; //TODO: check if this is acceptably - to use connId as userId
	TLSsecret.Key = secret;
	directStream->SetCredentials(TLSsecret);
	STLSSuite suite;
	suite.Compression = PRS_NULL;
	suite.Cipher = CPH_AES_256;
	suite.KeyExchange = KX_PSK;
	directStream->SetSuite(suite);
	directStream->SetConnectTimeout(timeOut);
	directStream->SetConnectThroughProxy( false );
	directStream->SetLocalAddr( NWL_INSTANCE.GetDirectStreamPort() );
	directStream->SetRemoteAddr( remoteAddress, remotePort );
	directStream->SetConnectErrorEvent(boost::bind( &CStreamFactory::SomeStreamFailed2Connect, this, _1, _2 ));
	directStream->SetConnectedEvent(boost::bind( &CStreamFactory::SomeStreamConnected, this, _1 ));
	directStream->Connect( true /*async*/ );
CATCH_LOG("CStreamFactory::DirectConnect")
}

void CStreamFactory::DirectProxyConnect(	const tstring& connectId, 
											const tstring& secret, 
											const tstring& _remoteAddress,
											const int timeOut,
											boost::shared_ptr<CDirectNetworkStream>& directStream )
{
TRY_CATCH
	if (!NWL_INSTANCE.GetUseProxy()) return;
	if (WaitForSingleObject(m_hConnectEvent, NET_DELAY_ORDER) == WAIT_OBJECT_0)
	{
		// Already connected
		return;
	}
	tstring remoteAddress(_remoteAddress);
	int remotePort = NWL_INSTANCE.GetDirectStreamPort();
		/// Separating remota address from remote port
	UnpackAddress(remoteAddress,remotePort);
	NotifyProgress(60,Format(_T("%s %s"),MLocalize(_T("Starting direct connect through proxy to")).c_str(),remoteAddress.c_str()));
	Log.Add(_MESSAGE_,_T("%s %s"),_T("Starting direct connect through proxy to"),remoteAddress.c_str());
	directStream.reset(new CDirectNetworkStream());
	STLSCredentials TLSsecret;
	TLSsecret.UserID = connectId; //TODO: check if this is acceptably - to use connId as userId
	TLSsecret.Key = secret;
	directStream->SetCredentials(TLSsecret);
	STLSSuite suite;
	suite.Compression = PRS_NULL;
	suite.Cipher = CPH_AES_256;
	suite.KeyExchange = KX_PSK;
	directStream->SetSuite(suite);
	directStream->SetConnectTimeout(timeOut);
	directStream->SetProxySettings( NWL_INSTANCE.GetProxySettings() );
	directStream->SetConnectThroughProxy( true );
	directStream->SetLocalAddr( NWL_INSTANCE.GetDirectStreamPort() );
	directStream->SetRemoteAddr( remoteAddress, remotePort );
	directStream->SetConnectErrorEvent(boost::bind( &CStreamFactory::SomeStreamFailed2Connect, this, _1, _2 ));
	directStream->SetConnectedEvent(boost::bind( &CStreamFactory::SomeStreamConnected, this, _1 ));
	directStream->Connect( true /*async*/ );
CATCH_LOG("CStreamFactory::DirectProxyConnect")
}

void CStreamFactory::NatTraversalConnect(	const tstring& connectId, 
											const tstring& secret, 
											const tstring& _remoteAddress,
											const int timeOut,
											const bool masterRole,
											const tstring& destPeerId,
											const tstring& sourcePeerId,
											boost::shared_ptr<CNATTraversingUDPNetworkStream> &natStream )
{
TRY_CATCH
	if (WaitForSingleObject(m_hConnectEvent, NET_DELAY_ORDER) == WAIT_OBJECT_0)
	{
		// Already connected
		return;
	}
	tstring remoteAddress(_remoteAddress);
	int remotePort;
	NotifyProgress(70,MLocalize(_T("Starting NAT traversal connect")));
	Log.Add(_MESSAGE_,_T("Starting NAT traversal connect"));
	UnpackAddress(remoteAddress,remotePort);
	natStream.reset(new CNATTraversingUDPNetworkStream());
	STLSCredentials TLSsecret;
	TLSsecret.UserID = connectId; //TODO: check if this is acceptably - to use connId as userId
	TLSsecret.Key = secret;
	natStream->SetCredentials(TLSsecret);
	STLSSuite suite;
	suite.Compression = PRS_NULL;
	suite.Cipher = CPH_AES_256;
	suite.KeyExchange = KX_PSK;
	natStream->SetSuite(suite);
	natStream->SetConnectTimeout(timeOut);

	natStream->SetRelayServer( 
		NWL_INSTANCE.GetRelayHost(),
		NWL_INSTANCE.GetRelayUDPPort(),
		m_strServerUserId,
		NWL_INSTANCE.GetRelayPasswd() );

	natStream->SetConnectionId( 
		connectId, 
		sourcePeerId,
		destPeerId );

	natStream->SetAuthRetry( 
		NWL_INSTANCE.GetAuthRetryDelay(),
		NWL_INSTANCE.GetAuthMaxRetryCount() );

	natStream->SetBindRetry(
		NWL_INSTANCE.GetBindRetryDelay(),
		NWL_INSTANCE.GetBindMaxRetryCount() );

	natStream->SetProbeRetry(
		NWL_INSTANCE.GetProbeRetryDelay(),
		NWL_INSTANCE.GetProbeMaxRetryCount() );

	natStream->SetProbePortRange( NWL_INSTANCE.GetProbePortRange() );

	natStream->SetIsMaster( masterRole );

	natStream->SetConnectErrorEvent(boost::bind( &CStreamFactory::SomeStreamFailed2Connect, this, _1, _2 ));
	//natStream->SetConnectedEvent(boost::bind( &CStreamFactory::SomeStreamConnected, this, _1 ));

	natStream->Connect( true /*async*/ );

CATCH_LOG("CStreamFactory::NatTraversalConnect")
}

boost::shared_ptr<CAbstractNetworkStream> CStreamFactory::RelayedConnect(	const tstring& connectId, 
																	const tstring& secret, 
																	const tstring& _remoteAddress,
																	const int timeOut,
																	const bool masterRole,
																	const tstring& destPeerId,
																	const tstring& sourcePeerId )
{
TRY_CATCH
	CheckAborted();
	tstring remoteAddress(_remoteAddress);
	int remotePort;
	UnpackAddress(remoteAddress,remotePort);
	/// Trying to connect through relayed stream
	CRelayedNetworkStream<>* relayedStream = new CRelayedNetworkStream<>();
	relayedStream->SetIsMaster(masterRole);
	STLSCredentials TLSsecret;
	TLSsecret.UserID = connectId; //TODO: check if this is acceptably - to use connId as userId
	TLSsecret.Key = secret;
	relayedStream->SetCredentials(TLSsecret);
	/*STLSSuite suite;
	suite.Compression = PRS_NULL;
	suite.Cipher = CPH_AES_256;
	suite.KeyExchange = KX_PSK;
	relayedStream->SetSuite(suite);*/ //TODO: fix this
	relayedStream->SetConnectTimeout(timeOut);
	relayedStream->SetRelayServer( 
		NWL_INSTANCE.GetRelayHost(),
		NWL_INSTANCE.GetRelayTCPPort(),
		m_strServerUserId,
		NWL_INSTANCE.GetRelayPasswd() );
	relayedStream->SetConnectionId( 
		connectId, 
		sourcePeerId,
		destPeerId );
	NotifyProgress(80,MLocalize(_T("Trying connect to peer through relay server")));
	Log.Add(_MESSAGE_,_T("Trying connect to peer through relay server"));
	relayedStream->Connect();
	boost::shared_ptr<CAbstractNetworkStream> aStream;
	aStream.reset(relayedStream);
	NotifyProgress(100,MLocalize(_T("Connect through relayed stream")));
	Log.Add(_MESSAGE_,_T("Connect through relayed stream"));
	return aStream;
CATCH_THROW("CStreamFactory::RelayedConnect")
}

boost::shared_ptr<CAbstractNetworkStream> CStreamFactory::RelayedProxyConnect(	const tstring& connectId, 
																		const tstring& secret, 
																		const tstring& _remoteAddress,
																		const int timeOut,
																		const bool masterRole,
																		const tstring& destPeerId,
																		const tstring& sourcePeerId )
{
TRY_CATCH
	CheckAborted();
	//TODO: fix!!!!
	throw MCStreamException("Proxy relayed stream need to be fixed"); 
	tstring remoteAddress(_remoteAddress);
	int remotePort;
	UnpackAddress(remoteAddress,remotePort);
	/// Trying to connect through relayed streama
	CRelayedNetworkStream<>* relayedStream = new CRelayedNetworkStream<>();
	relayedStream->SetIsMaster(masterRole);
	STLSCredentials TLSsecret;
	TLSsecret.UserID = connectId; //TODO: check if this is acceptably - to use connId as userId
	TLSsecret.Key = secret;
	relayedStream->SetCredentials(TLSsecret);
	/*STLSSuite suite;
	suite.Compression = PRS_NULL;
	suite.Cipher = CPH_AES_256;
	suite.KeyExchange = KX_PSK;
	relayedStream->SetSuite(suite);*/ //TODO: fix this
	relayedStream->SetConnectTimeout(timeOut);
	relayedStream->SetRelayServer( 
		NWL_INSTANCE.GetRelayHost(),
		NWL_INSTANCE.GetRelayTCPPort(),
		m_strServerUserId,
		NWL_INSTANCE.GetRelayPasswd() );
	relayedStream->SetConnectionId( 
		connectId, 
		sourcePeerId,
		destPeerId );
	relayedStream->SetProxySettings( NWL_INSTANCE.GetProxySettings() );
	relayedStream->SetConnectThroughProxy( true );
	NotifyProgress(90,MLocalize(_T("Trying connect to peer through relay server through proxy")));
	Log.Add(_MESSAGE_,_T("Trying connect to peer through relay server through proxy"));
	relayedStream->Connect();
	boost::shared_ptr<CAbstractNetworkStream> aStream;
	aStream.reset(relayedStream);
	NotifyProgress(100,MLocalize(_T("Connect through relayed through proxy")));
	Log.Add(_MESSAGE_,_T("Connect through relayed through proxy"));
	return aStream;
CATCH_THROW("CStreamFactory::RelayedProxyConnect")
}

std::vector<tstring> CStreamFactory::GetRemoteAddresses(const tstring &addressesStr)
{
TRY_CATCH
	std::vector<tstring> remoteAddresses;
	TCHAR seps[] = _T("#");
	TCHAR *nextToken;
	std::auto_ptr<char> string;
	string.reset(new char[addressesStr.length()+1]);
	strcpy_s(string.get(),addressesStr.length()+1,addressesStr.c_str());
	for(PTCHAR token = _tcstok_s( string.get(), seps, &nextToken );
		token;
		token = _tcstok_s(NULL, seps, &nextToken))
	{
		remoteAddresses.push_back(token);
	}
	return remoteAddresses;
CATCH_THROW("CStreamFactory::GetRemoteAddresses")
}

std::set<tstring> CStreamFactory::GetLocalAddresses()
{
TRY_CATCH
	std::set<tstring> localAddresses;
	char name_buf[MAX_PATH];
	memset( name_buf, 0, MAX_PATH );
	// Get host name
	if ( !gethostname( name_buf, MAX_PATH ) )
	{
		// Get host info
		hostent* host = gethostbyname( name_buf );
		if ( host )
		{
			// Copy addresses
			char** addrs = host->h_addr_list;
			while ( *addrs )
			{
				// Get address
				char* str_addr = inet_ntoa( *((struct in_addr *)*addrs) );
				// Add address
				localAddresses.insert( str_addr );
				addrs++;
			}
		}
	}
	return localAddresses;
CATCH_THROW("CStreamFactory::GetLocalAddresses")
}

boost::shared_ptr<CAbstractNetworkStream> CStreamFactory::Connect(const tstring& sId,const tstring& sourcePeerId, const tstring& destPeerId, int timeOut, bool masterRole, bool async)
{
TRY_CATCH

	//Starting connect thread thread
	if (m_threadStarter.get())
	{
		NotifyProgress(5,MLocalize(_T("Waiting while previous connect terminates...")));
		Log.Add(_MESSAGE_,_T("Waiting while previous connect terminates..."));
	}
	m_threadStarter.reset(new CStarter(this));

	CCritSection cs(&m_cs);

	m_waitingForCompletion = true;
	m_connectedStream = NULL;
	m_async = async;
	m_sourcePeerId = sourcePeerId;
	m_destPeerId = destPeerId;
	m_sessionId	= sId;
	m_timeOut = timeOut;
	m_masterRole = masterRole;
	m_conenctAborted = true;
	m_stream = boost::shared_ptr<CAbstractNetworkStream>(reinterpret_cast<CAbstractNetworkStream*>(NULL));
	ResetEvent(m_hConnectEvent);
	m_conenctAborted = false;
	ResetEvent(m_threadStarter->hTerminatedEvent.get());
	m_threadStarter->Start();

	/// Returning immediately on async connect
	if (async) 
	{
		if ((m_timerId = SetTimer(NULL, 0, m_timeOut, &CStreamFactory::OnTimer)) == 0)
			Log.Add(_WARNING_,MLocalize(_T("Failed to SetTimer. Connect timeout won't be handled")).c_str());
		else
			m_fabricTimersMap[m_timerId] = this;
		
		return boost::shared_ptr<CAbstractNetworkStream>(reinterpret_cast<CAbstractNetworkStream*>(NULL));
	}

	if ( WAIT_TIMEOUT == WaitForSingleObject( m_threadStarter->hTerminatedEvent.get(), m_timeOut ) )
	{
		CheckAborted();
		//WaitForSingleObject(m_threadStarter->hTerminatedEvent.get(), 1000); //Waiting while connect thread is looking for connected stream
		if (m_stream.get())
		{
			NotifyProgress(100,MLocalize(_T("Connected")));
			Log.Add(_MESSAGE_,_T("Connected"));
			return m_stream;
		}
		throw MCStreamException(MLocalize(_T("Connect timeout expired")));
	} else
	{
		CheckAborted();
		if (m_stream.get())
		{
			NotifyProgress(100,MLocalize(_T("Connected")));
			Log.Add(_MESSAGE_,_T("Connected"));
			ReportStat(m_stream);
			return m_stream;
		}
		else
			throw MCStreamException(m_error);
	}
CATCH_THROW("CStreamFactory::Connect")
}

void CALLBACK CStreamFactory::OnTimer(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
TRY_CATCH
	CStreamFactory* _this;
	if (m_fabricTimersMap.find(idEvent) == m_fabricTimersMap.end())
	{
		throw MCException(Format(_T("Timer id(%d) not found"),idEvent));
		return;
	}
	_this = m_fabricTimersMap[idEvent];
	KillTimer(NULL, idEvent);
	_this->m_timerId = -1;
	if (_this->m_async && _this->m_conenctAborted) return;
	m_fabricTimersMap.erase(m_fabricTimersMap.find(idEvent));
	WaitForSingleObject(_this->m_threadStarter->hTerminatedEvent.get(), 1000); //Waiting while connect thread is looking for connected stream
	if (_this->m_stream.get())
	{
		_this->NotifyProgress(100,MLocalize(_T("Connected")));
		Log.Add(_MESSAGE_,_T("Connected"));
		_this->CallCompletion(_this->m_stream);
	} else
	if (_this->m_async)
	{
		_this->m_error = MLocalize(_T("Connect timeout expired"));
		_this->CallCompletion(boost::shared_ptr<CAbstractNetworkStream>(reinterpret_cast<CAbstractNetworkStream*>(NULL)));
		_this->m_conenctAborted = true;
	}

CATCH_LOG("CStreamFactory::OnTimer")
}

void CStreamFactory::StartThread(void*)
{
TRY_CATCH
	try
	{
		if (!m_async)
		{
			m_stream = ConnectInternal(m_sourcePeerId, m_destPeerId, m_timeOut, m_masterRole);
		}
		else
			ConnectInternal(m_sourcePeerId, m_destPeerId, m_timeOut, m_masterRole);
	}
	catch(CStreamException &e)
	{
		m_error = e.What();
	}
	catch(CExceptionBase &e)
	{
		m_error = e.what();
	}
	SetEvent(m_hConnectEvent);
	CallCompletion(boost::shared_ptr<CAbstractNetworkStream>(reinterpret_cast<CAbstractNetworkStream*>(NULL)));
	return;
CATCH_LOG("CStreamFactory::StartThread")
try
{
	m_error = "unknown error";
	SetEvent(m_hConnectEvent);
}
catch(...)
{
}
TRY_CATCH
	CallCompletion(boost::shared_ptr<CAbstractNetworkStream>(reinterpret_cast<CAbstractNetworkStream*>(NULL)));
CATCH_LOG("CStreamFactory::StartThread. CallCompletion")
}

void CStreamFactory::GetExternalIp(SPeerAddr& extAddress, const int timeOut)
{
TRY_CATCH

	try	//Trying to get external IP
	{
		NotifyProgress(10,MLocalize(_T("Getting external IP")));
		Log.Add(_MESSAGE_,_T("Getting external IP"));
		CExternalIPRequest request(NWL_INSTANCE.GetRelayHost(),NWL_INSTANCE.GetRelayTCPPort());
		extAddress = request.GetExternalAddress(m_strServerUserId, NWL_INSTANCE.GetRelayPasswd());
		CheckAborted();
	}
	catch(CStreamException &e)
	{	
		if (!NWL_INSTANCE.GetUseProxy()) 
			throw e;
		MLog_Exception(CExceptionBase(e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__))));
		try
		{
			// Trying to get external ip through proxy
			NotifyProgress(20,MLocalize(_T("Getting external IP through proxy")));
			Log.Add(_MESSAGE_,_T("Getting external IP through proxy"));
			CExternalIPRequest request(NWL_INSTANCE.GetRelayHost(),NWL_INSTANCE.GetRelayTCPPort());
			extAddress = request.GetExternalAddressThroughProxy(m_strServerUserId, NWL_INSTANCE.GetRelayPasswd(), timeOut);
			CheckAborted();
		}
		catch(CStreamException &e)
		{
			throw CStreamException(e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)));
		}
	}

CATCH_THROW()
}

tstring CStreamFactory::RetriveLocalAddressesList(std::set<tstring> &localAddresses, const int timeOut)
{
TRY_CATCH

	SPeerAddr extAddress;
	/// Getting external IP
	GetExternalIp(extAddress, timeOut);

	/// Retriving local addresses
	std::set<tstring> localAddresses = GetLocalAddresses();
	localAddresses.insert( extAddress.address );
	tstring addressStr;
	bool customPort = ( DEFAULT_DIRECT_STREAM_PORT != NWL_INSTANCE.GetDirectStreamPort() );
	for(std::set<tstring>::const_iterator addr = localAddresses.begin();
		addr != localAddresses.end();
		++addr)
	{
		if (!addressStr.empty()) 
			addressStr += _T("#");
		addressStr += customPort?PackAddress(*addr,NWL_INSTANCE.GetDirectStreamPort()):*addr;
	}
	return addressStr;
CATCH_THROW()
}

void CStreamFactory::ExchngeInitialStuff(const tstring& sourcePeerId, 
										const tstring& destPeerId, 
										int timeOut, 
										bool masterRole,
										const tstring &addressStr, 
										std::vector<tstring> &remoteAddresses,
										tstring& connectId,
										tstring& secret,
										tstring& remoteAddress
										)
{
TRY_CATCH
	tstring remoteAddressStr;
	int retryInterval = NWL_INSTANCE.GetIMRetryInterval(); /*1 second is default*/

	if (masterRole) //master peer
	{
		connectId = GetGUID();
		secret = GetGUID();
		/// Master message format is address@secret@connectId
		NotifyProgress(30,MLocalize(_T("Sending initial message to remote peer")));
		Log.Add(_MESSAGE_,_T("Sending initial message to remote peer"));
		HANDLE hEvent = GetHandleMsgEvent();
		if (!hEvent)
		{
			SendMsg(destPeerId, Format(("%s@%s@%s"),addressStr.c_str(),secret.c_str(),connectId.c_str()));
		} else
		{
			DWORD res;
			int n;
			for(n=0; n < m_timeOut; n+=retryInterval)
			{
				CheckAborted();
				SendMsg(destPeerId, Format(("%s@%s@%s"),addressStr.c_str(),secret.c_str(),connectId.c_str()));
				NotifyProgress(static_cast<int>(40 + 10/*%*/ * (((float)n) / m_timeOut)),MLocalize(_T("Retriving initial message from remote peer")));
				Log.Add(_MESSAGE_,_T("Retriving initial message from remote peer"));
				if ((res = WaitForSingleObject(hEvent, retryInterval)) == WAIT_OBJECT_0)
				{
					break;
				} 
				else if (res != WAIT_TIMEOUT)
				{
					/// invalid message handle
					throw MCStreamException(MLocalize(_T("GetHandleMsgEvent returned invalid event handle")));
				}
				NotifyProgress(static_cast<int>(40 + 10/*%*/ * (((float)n) / m_timeOut)),MLocalize(_T("ReSending initial message to remote peer")));
				Log.Add(_MESSAGE_,_T("ReSending initial message to remote peer"));
			}

			if (n >= m_timeOut)
			{
				/// connect timed out
				throw MCStreamException(MLocalize(_T("Receiving initial message timed out")));
			}
		}
		CheckAborted();
		NotifyProgress(40,MLocalize(_T("Retriving initial message from remote peer")));
		Log.Add(_MESSAGE_,_T("Retriving initial message from remote peer"));
		HandleMsg(destPeerId, remoteAddressStr);
		remoteAddresses = GetRemoteAddresses(remoteAddressStr);
		if (remoteAddresses.empty())
			MCStreamException(Format("Empty address list in message(%s) from master host",remoteAddressStr.c_str()));
		remoteAddress = remoteAddresses.back();
		CheckAborted();
	} else //slave peer
	{
		tstring masterMsg;
		NotifyProgress(30,MLocalize(_T("Retriving initial message from remote peer")));
		Log.Add(_MESSAGE_,_T("Retriving initial message from remote peer"));
		HANDLE hEvent = GetHandleMsgEvent();
		if (hEvent)
		{
			DWORD res;
			int n;
			for(n=0; n < m_timeOut; n+=retryInterval)
			{
				CheckAborted();
				NotifyProgress(static_cast<int>(30 + 10/*%*/ * (((float)n) / m_timeOut)),MLocalize(_T("Retriving initial message from remote peer")));
				Log.Add(_MESSAGE_,_T("Retriving initial message from remote peer"));
				if ((res = WaitForSingleObject(hEvent, retryInterval)) == WAIT_OBJECT_0)
				{
					//Ok
					break;
				} else
				if (res != WAIT_TIMEOUT)
				{
					/// invalid message handle
					throw MCStreamException(MLocalize(_T("GetHandleMsgEvent returned invalid event handle")));
				}
			}

			if (n >= m_timeOut)
			{
				/// connect timed out
				throw MCStreamException(MLocalize(_T("Receiving initial message timed out")));
			}
		}
		CheckAborted();
		HandleMsg(destPeerId, masterMsg);
		CheckAborted();
		if (masterMsg.find('@') == tstring::npos)
			throw MCStreamException(Format("Invalid message(%s) from master host",masterMsg.c_str()));
		TCHAR seps[] = _T("@");
		TCHAR *nextToken;
		std::auto_ptr<char> string;
		string.reset(new char[masterMsg.length()+1]);
		strcpy_s(string.get(),masterMsg.length()+1,masterMsg.c_str());
		PTCHAR token = _tcstok_s( string.get() , seps, &nextToken );
		if (!token) throw MCStreamException(Format("Invalid message(%s) from master host",masterMsg.c_str()));
		remoteAddressStr = token;
		token = _tcstok_s(NULL, seps, &nextToken);
		if (!token) throw MCStreamException(Format("Invalid message(%s) from master host",masterMsg.c_str()));
		secret = token;
		token = _tcstok_s(NULL, seps, &nextToken);
		if (!token) throw MCStreamException(Format("Invalid message(%s) from master host",masterMsg.c_str()));
		connectId = token;
		remoteAddresses = GetRemoteAddresses(remoteAddressStr);
		if (remoteAddresses.empty())
			MCStreamException(Format("Empty address list in message(%s) from master host",masterMsg.c_str()));
		remoteAddress = remoteAddresses.back();
		///Sending own external address
		NotifyProgress(40,MLocalize(_T("Sending initial message to remote peer")));
		Log.Add(_MESSAGE_,_T("Sending initial message to remote peer"));
		CheckAborted();
		SendMsg(destPeerId, addressStr);
		CheckAborted();
	}
CATCH_THROW()
}

boost::shared_ptr<CAbstractNetworkStream> CStreamFactory::ConnectInternal(const tstring& sourcePeerId, const tstring& destPeerId, int timeOut, bool masterRole)
{
TRY_CATCH
	m_conenctAborted = false;
	m_connectingStreamsCount = 0;
	if (sourcePeerId == destPeerId)
		throw MCStreamException("Loopback connections not supported");

	// Retriving all external and internal addresses
	std::set<tstring> localAddresses;
	tstring addressStr = RetriveLocalAddressesList(localAddresses, timeOut / 3);

	// Performing initial stuff exchanging
	std::vector<tstring> remoteAddresses;
	tstring connectId;
	tstring secret;
	tstring remoteAddress;

	// Initial message exchange
	InitExchanger(sourcePeerId, destPeerId, timeOut, masterRole);
	try
	{
		CheckAborted();
		ExchngeInitialStuff(sourcePeerId, destPeerId, timeOut, masterRole, addressStr, remoteAddresses, connectId, secret, remoteAddress);
		CheckAborted();
		CloseExchanger(masterRole);
	}
	catch(...)
	{
		TRY_CATCH
			AbortExchanger();
		CATCH_LOG()
		throw;
	}
	
	m_connectId = connectId;

	// Starting concurent direct connections
	std::list<boost::shared_ptr<CDirectNetworkStream> > directStreams;
	for(std::vector<tstring>::const_iterator addr = remoteAddresses.begin();
		addr!=remoteAddresses.end();
		++addr)
	{
		std::set<tstring>::iterator addr_index = localAddresses.find( *addr );
		if ( addr_index == localAddresses.end() )
		{
			directStreams.push_back(boost::shared_ptr<CDirectNetworkStream>());
			DirectConnect( connectId, secret, *addr, timeOut / 3, directStreams.back() );
		}
	}
	directStreams.push_back(boost::shared_ptr<CDirectNetworkStream>());
	DirectProxyConnect( connectId, secret, remoteAddress, timeOut / 3, directStreams.back() );

	// Starting nat traversal connection
	boost::shared_ptr<CNATTraversingUDPNetworkStream> natStream;
	NatTraversalConnect( connectId, secret, remoteAddress, timeOut / 3, masterRole, destPeerId, sourcePeerId, natStream );

	// Waiting for connected stream
	NotifyProgress(70,MLocalize(_T("Waiting for TCP and UDP connections")));
	Log.Add(_MESSAGE_,_T("Waiting for TCP and UDP connections"));
	DWORD waitResult = WaitForSingleObject( m_hConnectEvent, m_timeOut / 3 );
	{
		CheckAborted();
		void *connectedStream = NULL;
		{
			CCritSection cs(&m_streamEventsCS);
			connectedStream = m_connectedStream;
		}
		if (waitResult == WAIT_OBJECT_0 && connectedStream != NULL)
		{
			// We have connected stream
			// Searching for corresponding smart pointer
			for(std::list<boost::shared_ptr<CDirectNetworkStream> >::iterator stream = directStreams.begin();
				stream != directStreams.end();
				++stream)
			{
				if (m_connectedStream == stream->get())
				{
					NotifyProgress(100,MLocalize(_T("Connect directly")));
					Log.Add(_MESSAGE_,_T("Connect directly"));
					CallCompletion(*stream);
					return *stream;
				}
			}
		}
		/// If no connected direct stream, try using natStream
		if (natStream.get())
		{
			if (natStream->Connected())
			{
				NotifyProgress(100,MLocalize(_T("Connect through UDP")));
				Log.Add(_MESSAGE_,_T("Connect through UDP"));
				CallCompletion(natStream);
				return natStream;
			}
		}
	}
	/// Trying to connect through relay server
	try
	{
		//TODO: check connect timeout
		boost::shared_ptr<CAbstractNetworkStream> stream;
		stream = RelayedConnect( connectId, secret, remoteAddress, timeOut / 3, masterRole, destPeerId, sourcePeerId );
		CallCompletion(stream);
		return stream;
	}
	catch(CStreamException &e)
	{
		MLog_Exception(CExceptionBase(e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__))));
		try
		{
			if (NWL_INSTANCE.GetUseProxy())
			{
				boost::shared_ptr<CAbstractNetworkStream> stream;
				stream = RelayedProxyConnect( connectId, secret, remoteAddress, timeOut / 3, masterRole, destPeerId, sourcePeerId );
				CallCompletion(stream);
				return stream;
			}
		}
		catch(CStreamException &e)
		{
			MLog_Exception(CExceptionBase(e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__))));
		}
	}
	NotifyProgress(100,MLocalize(_T("Connect failed")));
	Log.Add(_MESSAGE_,_T("Connect failed"));
	throw MCStreamException(Format("Failed to connect %s",destPeerId.c_str()));
		
CATCH_THROW("CStreamFactory::Connect")
}

tstring CStreamFactory::GetServerUserId() const
{
TRY_CATCH
	return m_strServerUserId;
CATCH_THROW("CStreamFactory::GetServerUserId")
}

void CStreamFactory::SetServerUserId( const tstring& userid )
{
TRY_CATCH
	m_strServerUserId = userid;
	STATISTIC_CLIENT_INSTANCE.SetUserId(m_strServerUserId);
CATCH_THROW("CStreamFactory::SetServerUserId")
}

tstring CStreamFactory::GetServerPassword() const
{
TRY_CATCH
	return m_strServerPassword;
CATCH_THROW("CStreamFactory::GetServerPassword")
}

void CStreamFactory::SetServerPassword( const tstring& passwd )
{
TRY_CATCH
	m_strServerPassword = passwd;
CATCH_THROW("CStreamFactory::SetServerPassword")
}

void CStreamFactory::CallCompletion(boost::shared_ptr<CAbstractNetworkStream> stream, const tstring& error)
{
TRY_CATCH

	if (false == m_async)
	{
		Log.Add(_MESSAGE_,_T("Exiting CallCompletion for not Async Connect"));
		return; //Completion should be called only for async connect
	}

	if (!error.empty())
		m_error = error;
 
	if (-1 != m_timerId) //Killing timer. No more needed
	{
		KillTimer(NULL, m_timerId);
		m_timerId = -1;
	}

	CCritSection cs(&m_cs);
	if (m_waitingForCompletion)
	{
		m_waitingForCompletion = false;
		ConnectCompletion(stream);
		Log.Add(_MESSAGE_,_T("ConnectCompletion sucessfully called"));
		ReportStat(stream);
	} else
	{
		Log.Add(_WARNING_,_T("ConnectCompletion already called"));
	}
	m_stream.reset();

CATCH_THROW("CStreamFactory::CallCompletion")
}

void CStreamFactory::ReportConnectionSpeed()
{
TRY_CATCH

	/// Reporting stat
	CMeasurement& measure=	MEASUREMENT_INFO_INSTANCE;
	CStatisticClient::CreateStatisticMessage(	m_sessionId, 
												m_sourcePeerId, 
												m_connectId,//m_masterRole?Format(_T("%s%s"),m_sourcePeerId.c_str(),m_destPeerId.c_str()):Format(_T("%s%s"),m_destPeerId.c_str(),m_sourcePeerId.c_str()), 
												-1, 
												sttPingPongData, 
												&measure, 
												sizeof(CMeasurement));

CATCH_LOG()
}

void CStreamFactory::ReportStat(boost::shared_ptr<CAbstractNetworkStream> stream)
{
TRY_CATCH
	/// Reporting stat
	if (NULL == stream.get())
	{
		SStatisticError error;
		error.Init(0, m_error);
		CStatisticClient::CreateStatisticMessage(	m_sessionId, 
													m_sourcePeerId, 
													m_connectId,//m_masterRole?Format(_T("%s%s"),m_sourcePeerId.c_str(),m_destPeerId.c_str()):Format(_T("%s%s"),m_destPeerId.c_str(),m_sourcePeerId.c_str()), 
													-1, 
													sttNWLInitFailed, 
													&error, 
													sizeof(SStatisticError));
	} else
	{
		CDirectNetworkStream* directStream = dynamic_cast<CDirectNetworkStream*>(stream.get());
		if (NULL != directStream)
		{
			EConnectType type(directStream->GetConnectThroughProxy()?conDirectProxy:conDirect);
			CStatisticClient::CreateStatisticMessage(	m_sessionId, 
														m_sourcePeerId, 
														m_connectId,//m_masterRole?Format(_T("%s%s"),m_sourcePeerId.c_str(),m_destPeerId.c_str()):Format(_T("%s%s"),m_destPeerId.c_str(),m_sourcePeerId.c_str()), 
														-1, 
														sttConnectType, 
														&type, 
														sizeof(EConnectType));
		} else
		{
			CNATTraversingUDPNetworkStream* natStream = dynamic_cast<CNATTraversingUDPNetworkStream*>(stream.get());
			if (NULL != natStream)
			{
				EConnectType type(conNat);
				CStatisticClient::CreateStatisticMessage(	m_sessionId, 
															m_sourcePeerId, 
															m_connectId,//m_masterRole?Format(_T("%s%s"),m_sourcePeerId.c_str(),m_destPeerId.c_str()):Format(_T("%s%s"),m_destPeerId.c_str(),m_sourcePeerId.c_str()), 
															-1, 
															sttConnectType, 
															&type, 
															sizeof(EConnectType));
			} else
			{
				CRelayedNetworkStream<>* relayedStream = dynamic_cast<CRelayedNetworkStream<>*>(stream.get());
				if (NULL != relayedStream)
				{
					EConnectType type(relayedStream->GetConnectThroughProxy()?conRelayProxy:conRelay);
					CStatisticClient::CreateStatisticMessage(	m_sessionId, 
																m_sourcePeerId, 
																m_connectId,//m_masterRole?Format(_T("%s%s"),m_sourcePeerId.c_str(),m_destPeerId.c_str()):Format(_T("%s%s"),m_destPeerId.c_str(),m_sourcePeerId.c_str()), 
																-1, 
																sttConnectType, 
																&type, 
																sizeof(EConnectType));
				}
				else
				{
					SStatisticError error;
					error.Init(0, _T("Unknown connection type"));
					CStatisticClient::CreateStatisticMessage(	m_sessionId, 
																m_sourcePeerId, 
																m_connectId,//m_masterRole?Format(_T("%s%s"),m_sourcePeerId.c_str(),m_destPeerId.c_str()):Format(_T("%s%s"),m_destPeerId.c_str(),m_sourcePeerId.c_str()), 
																-1, 
																sttNWLInitFailed, 
																&error, 
																sizeof(SStatisticError));
				}
			}
		}
	}

	SGatewayInfo router;
	CStatisticClient::CreateStatisticMessage(	m_sessionId, 
												m_sourcePeerId, 
												m_connectId,//m_masterRole?Format(_T("%s%s"),m_sourcePeerId.c_str(),m_destPeerId.c_str()):Format(_T("%s%s"),m_destPeerId.c_str(),m_sourcePeerId.c_str()), 
												-1, 
												sttRouterInfo, 
												&router, 
												sizeof(SGatewayInfo));

CATCH_LOG()
}
