#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CNetLogClient.h
///
///  NetLogClient COM object
///
///  @author Sogin Max @date 27.03.2007
///
////////////////////////////////////////////////////////////////////////

#include "resource.h"       // main symbols
#include "CLogMessage.h"
#include <NWL/Streaming/CSocketStream.h>

// INetLogClient
[
	object,
	uuid("46005FF7-AC90-4826-A66E-C7DA2D101B44"),
	dual,	helpstring("INetLogClient Interface"),
	pointer_default(unique)
]
__interface INetLogClient : IDispatch
{
	[propget, id(1), helpstring("property Name")] HRESULT Name([out, retval] BSTR* pVal);
	[propput, id(1), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
	[propget, id(2), helpstring("property RecentReply")] HRESULT RecentReply([out, retval] DATE* pVal);
	[propput, id(2), helpstring("property RecentReply")] HRESULT RecentReply([in] DATE newVal);
	[id(3), helpstring("method Attach")] HRESULT Attach(void);
	[id(4), helpstring("method Detach")] HRESULT Detach(void);
	[propget, id(5), helpstring("property IP")] HRESULT IP([out, retval] BSTR* pVal);
	[propput, id(5), helpstring("property IP")] HRESULT IP([in] BSTR newVal);
	[propget, id(6), helpstring("property State")] HRESULT State([out, retval] SHORT* pVal);
	[id(7), helpstring("method SetVerbosity")] HRESULT SetVerbosity(SHORT verbosity);
	[propget, id(8), helpstring("property DelayedMode")] HRESULT DelayedMode([out, retval] VARIANT_BOOL* pVal);
	[propput, id(8), helpstring("property DelayedMode")] HRESULT DelayedMode([in] VARIANT_BOOL newVal);
	[propget, id(9), helpstring("property TCPPort")] HRESULT TCPPort([out, retval] LONG* pVal);
	[propput, id(9), helpstring("property TCPPort")] HRESULT TCPPort([in] LONG newVal);
	[id(10), helpstring("method RequestDelayedMessages")] HRESULT RequestDelayedMessages(void);
};


// _INetLogClientEvents
[
	dispinterface,
	uuid("06992A7A-D868-4664-9015-C70C84B33E6C"),
	helpstring("_INetLogClientEvents Interface")
]
__interface _INetLogClientEvents
{
	[id(1), helpstring("method OnLogMessage")] HRESULT OnLogMessage([in] ILogMessage* message);
	[id(2), helpstring("method OnStateChanged")] HRESULT OnStateChanged(SHORT state);
};


/// Net log client states
typedef enum _ENetLogClientState
{
	DETACHED			= 0,
	ATTACHING			= 1,
	ATTACHED			= 2,
} ENetLogClientState;

// CNetLogClient
[
	coclass,
	default(INetLogClient, _INetLogClientEvents),
	threading(free),
	support_error_info("INetLogClient"),
	event_source(com),
	aggregatable(never),
	vi_progid("NetLogViewerLib.NetLogClient"),
	progid("NetLogViewerLib.NetLogClient.1"),
	version(1.0),
	uuid("E1035987-C670-4686-844E-B82E8C861E1D"),
	helpstring("NetLogClient Class")
]
/// NetLog client representation
class ATL_NO_VTABLE CNetLogClient
	:	public INetLogClient,
		public CThread
{
private:
	/// Client name
	tstring m_name;
	/// Client ip address
	tstring m_clientIP;
	/// Recent reply date time
	cDate m_recentReply;
	/// Socket stream
	CSocketStream m_stream;
	/// Net log client state
	ENetLogClientState m_state;
	/// Port for TCP connects
	int m_TPPPort;
	/// Send critical section
	CRITICAL_SECTION m_cs;
	/// Delayed mode; false by default
	bool m_delayedMode;

	/// Internal thread entry point
	virtual void Execute(void*);

	/// Set new client state
	void SetState(const ENetLogClientState& state);
public:
	/// Initializes object instance
	CNetLogClient();
	/// dtor
	virtual ~CNetLogClient();

	__event __interface _INetLogClientEvents;


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}
private:
	/// Connected event handler
	void OnConnected( void* param );
	/// Disconnected event handler
	void OnConnectError( void* param, EConnectErrorReason reason );
	/// Stream disconnected event handler
	void OnDisconnected( void* param );

public:
	/// Returns client name
	/// @remark name is unique client identifier
	STDMETHOD(get_Name)(BSTR* pVal);
	/// Set client name
	/// @remark name is unique client identifier
	STDMETHOD(put_Name)(BSTR newVal);
	/// Returns recent reply date time
	STDMETHOD(get_RecentReply)(DATE* pVal);
	/// Set recent reply date time
	STDMETHOD(put_RecentReply)(DATE newVal);
	/// Attaches to client
	STDMETHOD(Attach)(void);
	/// Detaches from client
	STDMETHOD(Detach)(void);
	/// Returns client IP address
	STDMETHOD(get_IP)(BSTR* pVal);
	/// Setup client IP address
	STDMETHOD(put_IP)(BSTR newVal);
	/// Returns current client state
	STDMETHOD(get_State)(SHORT* pVal);
	/// Sets new verbosity value for client
	STDMETHOD(SetVerbosity)(SHORT verbosity);
	/// Gets delayed mode value
	STDMETHOD(get_DelayedMode)(VARIANT_BOOL* pVal);
	/// Sets delayed mode value
	STDMETHOD(put_DelayedMode)(VARIANT_BOOL newVal);
	/// Gets port for TCP connects
	STDMETHOD(get_TCPPort)(LONG* pVal);
	/// Sets port for TCP connects
	STDMETHOD(put_TCPPort)(LONG newVal);
	/// Requests delayed messages from client
	STDMETHOD(RequestDelayedMessages)(void);
};

