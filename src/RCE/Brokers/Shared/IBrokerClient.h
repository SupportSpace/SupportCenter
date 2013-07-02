/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  IBrokerClient.h
///
///  IBrokerClient and _IBrokerClientEvents declaration, interfaces which must be inherit by service for use it by VBroker/Broker
///
///  @author Kirill Solovyov @date 12.11.2007
///
////////////////////////////////////////////////////////////////////////
#pragma once
#include <boost/shared_ptr.hpp>
#include <NWL/Streaming/CAbstractStream.h>
//#include <NWL/Multiplexer/CSubStream.h>

// IBrokerClient
[
	object,
	uuid("D8B1B541-E523-424E-BA9B-5078FFDC2749"),
	//dual,
	oleautomation,
	helpstring("IBrokerClient Interface"),
	pointer_default(unique),
	library_block
]
//__interface IBrokerClient : public IDispatch
__interface IBrokerClient : public IUnknown
{
	[id(1), helpstring("method Init")] HRESULT Init([in] IUnknown* events);
	//[id(1), helpstring("method Init")] HRESULT Init([in] ULONG events);
	[id(2), helpstring("method HandleRequest")] HRESULT HandleRequest([in] BSTR dstUserId, [in] ULONG dstSvcId, [in] BSTR srcUserId, [in] ULONG srcSvcId, [in] ULONG rId, [in] ULONG rType, [in] ULONG param, [in] BSTR params);

	/// The method set sub stream to the service. The method is answer on call of GetSubStream() method of IBrokerClient
	/// @param dstUserId unique destination user identifier
	/// @param dstSvcId unique per UserId destination service identifier
	/// @param streamId unique for this service stream identifier. it is not real sub stream identifier of Stream Multimplexer
	/// @param pointer_shared_ptr_stream raw pointer on shared_ptr object which is pointer on substream. Service must assign with refrence count increase (boost::shared_ptr<CAbstractStream> m_stream;\n m_stream=*reinterpret_cast<boost::shared_ptr<CAbstractStream>*>(pointer_shared_ptr_stream);
	[id(3), helpstring("method SetSubStream")] HRESULT SetSubStream([in] BSTR dstUserId, [in] ULONG dstSvcId, [in] ULONG streamId, [in] ULONG pointer_shared_ptr_stream);
};

// _IBrokerClientEvents
[
	object,
	uuid("92855FF9-79E9-41A4-9A8B-0F080B59FB8F"),
	//dispinterface,
	oleautomation,
	helpstring("_IBrokerClientEvents Interface"),
	library_block
]
__interface _IBrokerClientEvents : public IUnknown
{
	/// The method handle the request. It is called by owner when client must handle request
	/// See Infrastructure document Reqeust topic for parameters details.
	/// @param dstUserId unique destination user identifier
	/// @param dstSvcId unique per UserId destination service identifier
	/// @param srcUserId unique source user identifier
	/// @param srcSvcId unique per UserId source service identifier
	/// @param rId unique request identifier
	/// @param rType action (request) type
	/// @param param rType depended parameter
	/// @param params packed to string rType depended parameters
	[id(1), helpstring("method RequestSent")] HRESULT RequestSent([in] BSTR dstUserId, [in] ULONG dstSvcId, [in] BSTR srcUserId, [in] ULONG srcSvcId, [in] ULONG rId, [in] ULONG rType, [in] ULONG param, [in] BSTR params);

	/// The method get sub stream to the service. The method is asynchronous, real sub stream get via call SetSubStream() method of IBrokerClient
	/// @param dstUserId unique destination user identifier
	/// @param dstSvcId unique per UserId destination service identifier
	/// @param streamId unique for this service stream identifier. it is not real sub stream identifier of Stream Multimplexer
	/// @param priorityLevel Priority level for the substream
	[id(2), helpstring("method GetSubStream")] HRESULT GetSubStream([in] BSTR dstUserId, [in] ULONG dstSvcId, [in] ULONG streamId, [in] ULONG priorityLevel);
};

/// Crush for SetProtectedProcess method of RCHost
class COriginalIEPIDWrapper
{
private:
	DWORD m_pid;
public:
	COriginalIEPIDWrapper() : m_pid(-1)
	{
	}

	void SetPid(const DWORD pid)
	{
		m_pid = pid;
	}

	DWORD GetPid() const
	{
		return m_pid;
	}
};