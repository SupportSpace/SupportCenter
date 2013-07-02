/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CService.h
///
///  CService object declaration. The object implement wrapper for Service COM object (creation, events and others)
///
///  @author Kirill Solovyov @date 12.11.2007
///
////////////////////////////////////////////////////////////////////////
#pragma once
#include <atlbase.h>
#include <atlctl.h>
#include "CAvailableServices.h"
#include "IBrokerClient.h"
#include <boost/shared_ptr.hpp>
//#include <NWL/Streaming/CAbstractStream.h>
#include <NWL/Multiplexer/CSubStream.h>
#include <AidLib/Logging/CInstanceTracker.h>
class CService;
#include "CServicesMgr.h"

///// C_IBrokerClientEvents
//class ATL_NO_VTABLE C_IBrokerClientEvents :
//	public CComObjectRootEx<CComMultiThreadModelNoCS>,
//	public CComCoClass<C_IBrokerClientEvents>,
//	public IDispEventImpl<0, C_IBrokerClientEvents, &__uuidof(_IBrokerClientEvents), &IID_NULL, 1, 0>
//{
//public:
//	C_IBrokerClientEvents();
//	DECLARE_PROTECT_FINAL_CONSTRUCT()
//	HRESULT FinalConstruct();
//	void FinalRelease();
//	BEGIN_COM_MAP(C_IBrokerClientEvents)
//		COM_INTERFACE_ENTRY_IID(__uuidof(_IBrokerClientEvents), C_IBrokerClientEvents)
//	END_COM_MAP()
//
//	BEGIN_SINK_MAP(C_IBrokerClientEvents)
//		SINK_ENTRY_EX(0, __uuidof(_IBrokerClientEvents), 1, RequestSent)
//		SINK_ENTRY_EX(0, __uuidof(_IBrokerClientEvents), 2, GetSubStream)
//	END_SINK_MAP()
//	/// owner of this event receiver
//	CService *m_owner;
//
//	// _IBrokerClientEvents Methods
//public:
//	STDMETHOD(RequestSent)(BSTR dstUserId, ULONG dstSvcId, BSTR srcUserId, ULONG srcSvcId, ULONG rId, ULONG rType, ULONG param, BSTR params);
//	STDMETHOD(GetSubStream)(BSTR sId, USHORT streamId, ULONG priorityLevel);
//};

/// C_IBrokerClientEvents
class ATL_NO_VTABLE C_IBrokerClientEvents :
	public CComObjectRootEx<CComMultiThreadModelNoCS>,
	//public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<C_IBrokerClientEvents>,
	public _IBrokerClientEvents,
	CInstanceTracker
	//public IDispEventImpl<0, C_IBrokerClientEvents, &__uuidof(_IBrokerClientEvents), &IID_NULL, 1, 0>
{
public:
	C_IBrokerClientEvents();
	DECLARE_PROTECT_FINAL_CONSTRUCT()
	HRESULT FinalConstruct();
	void FinalRelease();
	virtual ~C_IBrokerClientEvents(void);

	BEGIN_COM_MAP(C_IBrokerClientEvents)
		COM_INTERFACE_ENTRY_IID(__uuidof(_IBrokerClientEvents), C_IBrokerClientEvents)
	END_COM_MAP()

	/// owner of this event receiver
	CService *m_owner;

	// _IBrokerClientEvents Methods
public:
	STDMETHOD(RequestSent)(BSTR dstUserId, ULONG dstSvcId, BSTR srcUserId, ULONG srcSvcId, ULONG rId, ULONG rType, ULONG param, BSTR params);
	STDMETHOD(GetSubStream)(BSTR dstUserId, ULONG dstSvcId, ULONG streamId, ULONG priorityLevel);
};




/// CService 
class CService:
	public CInstanceTracker
{
	friend class C_IBrokerClientEvents;
protected:
	/// owner Service manager object
	CServicesMgr *m_owner;
	/// Service COM object pointer
	//CComPtr<IBrokerClient> m_service;
	CComGITPtr<IBrokerClient> m_service;

	/// Service host DHTML object
	//CComPtr<IDispatch> m_host;
	/// Service DHTML object
	//CComPtr<IHTMLObjectElement> m_object;
	CComGITPtr<IHTMLElement> m_hostElement;
	/// Service events object
	CComPtr<CComObject<C_IBrokerClientEvents> > m_events;

	/// The method is called by Service COM object when it wants send a request.
	/// for parameters detail see Request topic of Infrastructure document (current (file creation) state see on top of CSessionMgr.h file).
	void RequestSent(const tstring& dstUserId, unsigned long dstSvcId, const tstring& srcUserId, unsigned long srcSvcId, unsigned long rId, unsigned long rType, unsigned long param,const tstring& params);

	/// The method is called by Service COM object when it wants sub stream.
	/// @param dstUserId unique destination user identifier
	/// @param dstSvcId unique per UserId destination service identifier
	/// @param streamId unique for this service stream identifier. it is not real sub stream identifier of Stream Multimplexer
	/// @param priorityLevel Priority level for the substream
	void GetSubStream(const tstring& dstUserId, unsigned long dstSvcId, unsigned long streamId, unsigned long priorityLevel);


public:
	CService(CServicesMgr *onwer=NULL);
	virtual ~CService(void);

	/// Create service ActiveX COM object which host on DHTML object.
	/// @param svcType service type (see EBrokerServicesTypes).
	/// @param host DHTML host object
	void CreateToolService(EBrokerServicesTypes svcType, IDispatch* host);

	/// Create service COM object
	/// @param svcType service type (see EBrokerServicesTypes).
	void CreateToolService(EBrokerServicesTypes svcType);

	/// Destroy Service object
	void DestroyToolService();

	/// parameters of service
	struct
	{
		tstring m_userId;                 /// user user identifier
		tstring m_remoteUserId;           /// remote user identifier for the service
		unsigned long m_svcId;            /// service identifier
		unsigned long m_remoteSvcId;      /// remote service identifier for the service
		EBrokerServicesTypes m_svcType;   /// service type
	} m_params;

	
	/// The method retrieve service type
	/// @return service type
	EBrokerServicesTypes GetSvcType();

	/// The method pass request to Service COM object for handling
	/// for parameters detail see Request topic of Infrastructure document (current (file creation) state see on top of CSessionMgr.h file).
	void HandleRequest(const tstring& dstUserId, unsigned long dstSvcId, const tstring& srcUserId, unsigned long srcSvcId, unsigned long rId, unsigned long rType, unsigned long param,const tstring& params);

	/// The method pass stream to the Service COM object
	/// @param dstUserId unique destination user identifier
	/// @param dstSvcId unique per UserId destination service identifier
	/// @param streamId unique for this service stream identifier. it is not real sub stream identifier of Stream Multimplexer
	/// @param stream new sub stream
	void SetSubStream(const tstring& dstUserId, unsigned long dstSvcId, unsigned long streamId, boost::shared_ptr<CAbstractStream> stream);


};
