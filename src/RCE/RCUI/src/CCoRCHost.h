/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CCoRCHost.h
///
///  CCoRCHost object declaration (COM object wrapper of RCHost)
///
///  @author Kirill Solovyov @date 05.11.2007
///
////////////////////////////////////////////////////////////////////////
// CCoRCHost.h : Declaration of the CCoRCHost

#pragma once
#include "resource.h"       // main symbols

#include <AidLib/logging/clog.h>
#include <AidLib/CThread/CThread.h>
#include <RCEngine/CRCHost.h>
#include <AidLib/CSingleton/CSingleton.h>
#include <AidLib/Logging/CInstanceTracker.h>

#include "..\..\Brokers\Shared\IBrokerClient.h"
#include "..\..\Brokers\Shared\CAvailableServices.h"
#include "..\..\Brokers\Shared\BrokersTypes.h"

#ifndef BOOST_SIGNAL_USE_LIB 
	#define BOOST_SIGNALS_USE_LIB
#endif
#ifndef BOOST_SIGNALS_NO_LIB
	#define BOOST_SIGNALS_NO_LIB
#endif
#include <boost/signal.hpp>


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

/// Implements start/stop callbacks from CRCHost
class CRCHostImpl : public CRCHost
{
public:
	/// Host events (passed as event param1)
	enum EHostEvent
	{
		HE_STARTED,
		HE_STOPPED
	};

	/// Event listener delegate type
	typedef boost::function<void (EHostEvent eventType, const int param1, const int param2)> eventListener;

protected:
	/// A virtual method that notifies session has started.
	/// @param clientId corresponding cliend identifier
	virtual void NotifySessionStarted(const int clientId);

	/// A virtual method that notifies session has stopped and why 
	/// @param ReasonCode stop reason (Stop() called; Remote Stop() called; Stream signaled end-of-file; etc.)
	/// @param clientId corresponding cliend identifier
	virtual void NotifySessionStopped(const int clientId, ESessionStopReason ReasonCode);

	/// Critical section for m_listeners protection
	CCritSectionSimpleObject m_signalCS;

	/// Proper storage for delegates
	boost::signal<void (EHostEvent eventType, const int param1, const int param2)> m_signal;

	
	/// Removes all threads, not related to protected windows
	/// Relation is defined by some descendant class
	/// in base class no filtering is performed
	/// Could be used to protect only specific windows within process - for example some
	/// IE tab page
	virtual void FilterProtectedThreads(std::set<DWORD> &protectedThreads);

public:
	/// Add events listener delegate
	/// Returns newly created connection, which could be used during unsubscribtion
	boost::signals::connection SubscribeEventsListener(const eventListener listener);

	/// Unsubscribes event listener
	void UnsubscribeEventListener(boost::signals::connection &connection);
};

#define RCHOST_INSTANCE CProcessSingleton<CRCHostImpl>::instance()

// ICoRCHost
[
	object,
	uuid("76E9AFD4-FF2A-4116-81DA-38633977FA1A"),
	dual,	helpstring("ICoRCHost Interface"),
	pointer_default(unique)
]
__interface ICoRCHost : IDispatch
{
};


// CCoRCHost

[
	coclass,
	default(ICoRCHost),
	//threading(apartment),
	threading(neutral),
	vi_progid("RCUI.CoRCHost"),
	progid("RCUI.CoRCHost.1"),
	version(1.0),
	uuid("9DC84E6D-3B40-4AEF-BF51-F36E87B02F61"),
	helpstring("CoRCHost Class")
]
class ATL_NO_VTABLE CCoRCHost :
	public ICoRCHost,
	public IBrokerClient,
	protected CThread,
	public CInstanceTracker
{
public:
	/// ActiveX friendly name, use by registration
	static const TCHAR* GetObjectFriendlyName() 
	{
		return _T("Support Platform Remote Control Host");
	}
	CCoRCHost();
	virtual ~CCoRCHost(void);
	DECLARE_PROTECT_FINAL_CONSTRUCT()
	HRESULT FinalConstruct();
	void FinalRelease();
protected:
	/// borker intercommunication
	CComGITPtr<_IBrokerClientEvents> m_brokerEvents;
	
	/// service sub stream, it's used for peer communication
	boost::shared_ptr<CAbstractStream> m_svcStream;

	/// the service stream connected event. set when service stream is connected
	CEvent m_svcStreamConnectedEvent;

	/// The method is entry point of service sub stream creation and recieve handling
	virtual void Execute(void *Params);

	/// The method start RC session
	void InitiateRCConnectAndStart(void);

	/// The method is called when message come through service sub stream
	void HandleCoViewerCommand(ULONG buf);

	/// current session client id
	int m_clientId;

	/// Connection to RCHost events
	boost::signals::connection m_RCHostEnvetConnection;

	/// Host event handler
	void OnHostEvent(CRCHostImpl::EHostEvent eventType, const int param1, const int param2);

	/// Host activity event handler
	void OnHostActivity(CActivityMonitor::EActivityType activityType);

public:
	/// The method initialize viewer by _IBrokerClientEvents interface
	STDMETHOD(Init)(IUnknown *events);
	
	/// The method handle request sent by Broker
	STDMETHOD(HandleRequest)(BSTR dstUserId,ULONG dstSvcId,BSTR srcUserId,ULONG srcSvcId,ULONG rId,ULONG rType,ULONG param,BSTR params);

	/// The method call when asked sub stream connected
	STDMETHOD(SetSubStream)(BSTR dstUserId, ULONG dstSvcId, ULONG streamId, ULONG pointer_shared_ptr_stream);

};

