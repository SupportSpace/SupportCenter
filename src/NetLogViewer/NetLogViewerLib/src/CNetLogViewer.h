#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CNetLogViewer.h
///
///  NetLogViewer COM object
///
///  @author Sogin Max @date 27.03.2007
///
////////////////////////////////////////////////////////////////////////

#include "resource.h"       // main symbols
#include <atlcom.h>
#include <AidLib/CThread/CThread.h>
#include <NWL/Streaming/CSSocket.h>
#include <map>
#include "CNetLogClientList.h"

/// client timeout value
#define CLIENT_TIMEOUT 6000

/// Time interval between two client requests
#define REQUESTS_INTERVAL 3000

/// NetLog viewer udp port
#define NETLOG_UDP_SERVER_PORT 5907

// INetLogViewer
[
	object,
	uuid("8FF54CF0-5FA2-4AF5-A3ED-D877BADA4944"),
	dual,	helpstring("INetLogViewer Interface"),
	pointer_default(unique)
]
__interface INetLogViewer : IDispatch
{
	[id(1), helpstring("method GetClientsList")] HRESULT GetClientsList([out,retval] INetLogClientList** clientsList);
	[id(2), helpstring("method Start")] HRESULT Start(void);
	[id(3), helpstring("method Stop")] HRESULT Stop(void);
};


// _INetLogViewerEvents
[
	dispinterface,
	uuid("2430B5A7-5C19-4C6C-B728-412253FB0DA7"),
	helpstring("_INetLogViewerEvents Interface")
]
__interface _INetLogViewerEvents
{
	[id(1), helpstring("method OnClientFound")] HRESULT OnClientFound(INetLogClient* client);
	[id(2), helpstring("method OnClientTimedOut")] HRESULT OnClientTimedOut(INetLogClient* client);
};


// CNetLogViewer

[
	coclass,
	default(INetLogViewer, _INetLogViewerEvents),
	threading(free),
	support_error_info("INetLogViewer"),
	event_source(com),
	aggregatable(never),
	vi_progid("NetLogViewerLib.NetLogViewer"),
	progid("NetLogViewerLib.NetLogViewer.1"),
	version(1.0),
	uuid("BD03D7AB-217A-4A2A-9231-D771520DFAFB"),
	helpstring("NetLogViewer Class")
]
/// Main NetLogViewer library class
class ATL_NO_VTABLE CNetLogViewer :
	public INetLogViewer,
	public CThread
{
private:
	/// list of alive clients
	std::map<tstring, CComPtr<INetLogClient> > m_clients;
	/// udp socket
	CSSocket m_udpSocket;
	/// Internal critical section object
	CRITICAL_SECTION m_cs;

	/// Internal thread entry point
	virtual void Execute(void*);

	/// Cleans timed out clients
	void CleanUpTimedOutClients();

	/// Sends UDP broadcast request
	void SendBroadCastRequest();
public:
	/// initializes object instance
	CNetLogViewer();
	/// dtor
	virtual ~CNetLogViewer();

	__event __interface _INetLogViewerEvents;

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:
	/// Returns currenly alive clients list
	STDMETHOD(GetClientsList)(INetLogClientList** clientsList);
	/// Starts updates
	STDMETHOD(Start)(void);
	/// Stops updates
	STDMETHOD(Stop)(void);
};

