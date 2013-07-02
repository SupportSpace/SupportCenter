/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CComUDPListener.h
///
///  Declares CComUDPListener class, UDP listener COM object 
///
///  @author Dmitry Netrebenko @date 02.04.2007
///
////////////////////////////////////////////////////////////////////////


#pragma once
#include "resource.h"       // main symbols

#include <AidLib/CThread/CThread.h>
#include <NWL/Streaming/CSSocket.h>
#include <AidLib/WatchDog/CProcessWatchDog.h>

///  Time in msecs for UDP socket select operation
#define NETLOG_UDP_SELECT_TIME 100
///  Buffer size for CUDPListener
#define NETLOG_UDP_BUFFER_SIZE 1024
/// Broadcast datagramm from viewer
#define NETLOG_UDP_SERVER_REQUEST _T("Alive?")

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif


// IComUDPListener
[
	object,
	uuid("AC370F92-A9BF-4EAA-971C-3F28F770D67E"),
	dual,	helpstring("IComUDPListener Interface"),
	pointer_default(unique)
]
__interface IComUDPListener : IDispatch
{
	[id(1), helpstring("method Listen")] HRESULT Listen([in] LONG port);
	[id(2), helpstring("method AddWatch")] HRESULT AddWatch([in] LONG pid);
};


// _IComUDPListenerEvents
[
	dispinterface,
	uuid("E2DF56F8-611D-45FE-A2F4-4A33E18C280A"),
	helpstring("_IComUDPListenerEvents Interface")
]
__interface _IComUDPListenerEvents
{
	[id(1), helpstring("method OnDatagramReceived")] HRESULT OnDatagramReceived([in] BSTR addr, [in] LONG port);
};


// CComUDPListener

[
	coclass,
	default(IComUDPListener, _IComUDPListenerEvents),
	threading(free),
	event_source(com),
	vi_progid("NetLogLib.ComUDPListener"),
	progid("NetLogLib.ComUDPListener.1"),
	version(1.0),
	uuid("93ABD1AB-CD4B-4E21-BF9B-5DE265FD91AA"),
	helpstring("ComUDPListener Class")
]
class ATL_NO_VTABLE CComUDPListener 
	:	public IComUDPListener
	,	public CThread
{
public:
///  Constructor
	CComUDPListener();

///  Destructor;
	~CComUDPListener();

///  Thread's entry point
	void Execute(void*);

	__event __interface _IComUDPListenerEvents;

///  Singleton object
	DECLARE_CLASSFACTORY_SINGLETON(CComUDPListener)

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

private:
///  UDP socket
	CSSocket			m_socket;
///  Internal buffer
	char				m_buffer[NETLOG_UDP_BUFFER_SIZE];
///  Critical section
	CRITICAL_SECTION	m_cs;
///  Is thread is started
	bool				m_started;
///  Process watchdog. (Watching for client processes to perform self unload then)
	CSuicideProcessWatchDog	m_watchDog;
public:
///  Listen UDP port and start thread
///  @param port - UDP port
	STDMETHOD(Listen)(LONG port);

///  Watch for process
///  When no more watching processes - die
	STDMETHOD(AddWatch)(LONG pid);

private:
///  Invokes events
///  @param id - event id
///  @param args - array of params
///  @param count - number of params
	HRESULT SafeInvokeEvent( int id, VARIANT* args, int count );

///  Invokes OnDatagramReceived event
///  @param addr - remote address
///  @param port - remote port
	HRESULT InvokeOnDatagramReceived( const tstring& addr, const unsigned int port );
};

