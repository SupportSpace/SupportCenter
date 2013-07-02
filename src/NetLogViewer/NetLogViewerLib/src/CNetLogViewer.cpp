/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CNetLogViewer.cpp
///
///  NetLogViewer COM object
///
///  @author Sogin Max @date 27.03.2007
///
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CNetLogViewer.h"
#include <NetLog/CNetLog.h>
#include <NetLog/SUDPListenerMsg.h>

// CNetLogViewer

CNetLogViewer::CNetLogViewer()
	:	CThread(),
		m_udpSocket(stUDP)
{
TRY_CATCH
	// Preparing socket to send broadcasts
	InitializeCriticalSection(&m_cs);
	if (!m_udpSocket.Create())
		Log.Add(MCSocketStreamException(_T("Failed to create udp socket")));
	if (!m_udpSocket.Bind(NETLOG_UDP_SERVER_PORT))
		Log.Add(MCSocketStreamException(_T("Failed to bind for udp socket")));
	m_udpSocket.SwitchBroadCastOption(true);
CATCH_LOG()
}

CNetLogViewer::~CNetLogViewer()
{
TRY_CATCH
	DeleteCriticalSection(&m_cs);
CATCH_LOG()
}


void CNetLogViewer::SendBroadCastRequest()
{
TRY_CATCH
	CCritSection cs(&m_cs);
	m_udpSocket.SendBCast(NETLOG_UDP_PORT, NETLOG_UDP_SERVER_REQUEST, static_cast<unsigned int>(_tcslen(NETLOG_UDP_SERVER_REQUEST) + sizeof(TCHAR)) );
CATCH_THROW()
}

STDMETHODIMP CNetLogViewer::GetClientsList(INetLogClientList** clientsList)
{
TRY_CATCH

	SendBroadCastRequest();

	HRESULT hr = NetLogClientList::CNetLogClientList::CreateInstance(clientsList);
	if (S_OK != hr)
		return hr;

	// Protecting internal clients map
	CCritSection cs(&m_cs);

	for(std::map<tstring, CComPtr<INetLogClient> >::iterator client = m_clients.begin();
		client != m_clients.end();
		++client)
	{
		(*clientsList)->AddClient(client->second);
	}

	return S_OK;
CATCH_LOG_COMERROR()
}

void CNetLogViewer::Execute(void*)
{
TRY_CATCH

	/// Adding internal clients for tests
	/*{
		INetLogClient* newClient;
		tstring clientName;
		clientName = Format(_T("%s:%s"),"192.168.0.5","iexplore.exe");
		SYSTEMTIME sysTime = static_cast<SYSTEMTIME>(cDate().GetNow().AddCountDays(0));
		DATE date;
		SystemTimeToVariantTime(&sysTime,&date);
		CNetLogClient::CreateInstance(&newClient);
		newClient->put_RecentReply(date);
		newClient->put_Name(CComBSTR(clientName.c_str()));
		newClient->put_IP(CComBSTR("192.168.0.5"));
		m_clients[clientName] = newClient;
		OnClientFound(newClient);
	}
	{
		INetLogClient* newClient;
		tstring clientName;
		clientName = Format(_T("%s:%s"),"192.168.0.3","NWLTest.exe");
		SYSTEMTIME sysTime = static_cast<SYSTEMTIME>(cDate().GetNow().AddCountDays(1));
		DATE date;
		SystemTimeToVariantTime(&sysTime,&date);
		CNetLogClient::CreateInstance(&newClient);
		newClient->put_RecentReply(date);
		newClient->put_Name(CComBSTR(clientName.c_str()));
		newClient->put_IP(CComBSTR("192.168.0.3"));
		m_clients[clientName] = newClient;
		OnClientFound(newClient);
	}*/

	
	/// Last bCast send time
	cDate lastBCast;
	tstring fromIp;
	unsigned int fromPort;
	char buf[MAX_PATH + 1];
	buf[MAX_PATH] = 0;
	INetLogClient* newClient;
	tstring clientName;
	std::map<tstring, CComPtr<INetLogClient> >::iterator client;
	while(!Terminated())
	{
		/// Freing time quant
		/// Sleep is used instead of SwitchToThread(), cause it forces Dispatcher to switch context
		/// while SwitchToThread switches context only there are scheduled threads
		/// and whis is totally not what we want, cause it will result in 100% CPU usage
		Sleep(1); 	
		
		/// Sending broadcast request
		if (lastBCast < cDate().GetNow().AddMilliSecs(0 - REQUESTS_INTERVAL))
		{
			SendBroadCastRequest();
			lastBCast.GetNow();
			//(cMsgBoxLog()).Add(_MESSAGE_,"BCast sent");

			if (Terminated())
				break;
			/// Cleaning up timed out clients
			CleanUpTimedOutClients();
		}

		if (Terminated())
			break;
		/// Receiving alive replys from clients
		SUDPListenerMsg *msg;
		while (m_udpSocket.GetReadyDataCount())
		{
			memset(buf,0,MAX_PATH); /// TODO:remove
			if (SOCKET_ERROR == m_udpSocket.ReceiveFrom(fromIp, fromPort, buf, MAX_PATH))
			{
				Log.Add(MCSocketStreamException(_T("failed to m_udpSocket.ReceiveFrom")));
				break;
			}
			/// Creating client
			msg = reinterpret_cast<SUDPListenerMsg*>(buf);
			clientName = Format(_T("%s:%d at %s"), msg->m_data, msg->m_process, fromIp.c_str());
			client = m_clients.find(clientName);
			if (client != m_clients.end())
			{
				SYSTEMTIME sysTime = static_cast<SYSTEMTIME>(cDate().GetNow());
				DATE date;
				SystemTimeToVariantTime(&sysTime,&date);
				client->second->put_RecentReply(date);
				client->second->put_TCPPort(msg->m_tcpPort);
				//client
			} else
			{
				CNetLogClient::CreateInstance(&newClient);
				newClient->put_Name(CComBSTR(clientName.c_str()));
				newClient->put_IP(CComBSTR(fromIp.c_str()));
				newClient->put_TCPPort(msg->m_tcpPort);
				m_clients[clientName] = newClient;
				OnClientFound(newClient);
			}
		}
	}

CATCH_LOG()
}

void CNetLogViewer::CleanUpTimedOutClients()
{
TRY_CATCH

	// Protecting internal clients map
	CCritSection cs(&m_cs);

	std::map<tstring, CComPtr<INetLogClient> >::iterator client = m_clients.begin();
	DATE replyDate;
	cDate timedOutDate(cDate().GetNow());
	timedOutDate.AddMilliSecs( 0 - CLIENT_TIMEOUT );
	while(client != m_clients.end())
	{
		client->second->get_RecentReply(&replyDate);
		cDate replyDate(replyDate);
		if (timedOutDate > replyDate) ///Client timed out
		{
			std::map<tstring, CComPtr<INetLogClient> >::iterator timedOutClient = client;
			++client;
			OnClientTimedOut(timedOutClient->second);
			m_clients.erase(timedOutClient);
		}
		else
			++client;
	}
CATCH_LOG()
}
STDMETHODIMP CNetLogViewer::Start(void)
{
TRY_CATCH
	CThread::Start();
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CNetLogViewer::Stop(void)
{
TRY_CATCH
	CThread::Stop(false);
	return S_OK;
CATCH_LOG_COMERROR()}
