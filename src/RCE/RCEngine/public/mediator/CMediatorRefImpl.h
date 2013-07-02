/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CMediatorRefImpl.h
///
///  Declares CMediatorRefImpl template
///
///  @author Dmitry Netrebenko @date 26.12.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <atlctl.h>
#include <NWL/Streaming/CStreamFactoryRelayedImpl.h>
#include <NWL/Streaming/CStreamException.h>
#include "rcinstaller.tlh"

#ifdef 	USEIMSTUB
	#include <Streaming/CIMStub.h>
#endif

[
	uuid(BF539F54-A219-479c-AC0A-1D963102F34D),
	object,
	dual,
	pointer_default(unique),
	helpstring("IMediatorRef Interface")
]
__interface IMediatorRef : IDispatch
{
	[propget, id(1), helpstring("property Mediator")] HRESULT Mediator([out, retval] IUnknown** mediator);
	[propput, id(1), helpstring("property Mediator")] HRESULT Mediator([in] IUnknown* mediator);
};

template <typename interfaceT>
class ATL_NO_VTABLE CMediatorRefImpl
	:	public CStreamFactoryRelayedImpl
	,	public interfaceT
{
protected:
	///  Pointer to IMMediator ActiveX object
	IIMMediatorPtr m_Mediator;

	/// TODO: Remove this after sprint5

	/// Sends a message to the specified destination peer (user ID).  
	/// @param peerId destination peer
	/// @param messageData Message data is an arbitrary string.
	//virtual void SendMsg(const tstring& peerId, const tstring& messageData)
	//{
	//TRY_CATCH
	//#ifdef 	USEIMSTUB
	//	CIMStub im(peerId);
	//	im.SendMsg(messageData);
	//#else
	//	if (NULL == m_Mediator) throw MCStreamException("m_Mediator == NULL");
	//	m_Mediator->SendMsg(peerId.c_str(),messageData.c_str());
	//#endif
	//CATCH_THROW("CMediatorRefImpl::SendMsg")
	//}

	/// Handles an incoming message from a specified source peer (user ID).
	/// @param peerId source peer
	/// @param messageData Message data is an arbitrary string.
	//virtual void HandleMsg(const tstring& peerId, tstring& messageData)
	//{
	//TRY_CATCH
	//#ifdef 	USEIMSTUB	
	//	CIMStub im(m_sourcePeerId);
	//	im.HandleMsg(messageData);
	//#else	
	//	if (NULL == m_Mediator) throw MCStreamException("m_Mediator == NULL");
	//	USES_CONVERSION;
	//	WCHAR *buf = m_Mediator->HandleMsg(peerId.c_str());
	//	messageData = OLE2T(buf);
	//#endif
	//CATCH_THROW("CMediatorRefImpl::HandleMsg")
	//}

	/// Returns event handle, which handles incomming IM messages
	/// @return event handle, which handles incomming IM messages
	//virtual HANDLE GetHandleMsgEvent()
	//{
	//TRY_CATCH
	//#ifdef 	USEIMSTUB
	//	return NULL;
	//#else
	//	if (NULL == m_Mediator) throw MCStreamException("m_Mediator == NULL");
	//	if (S_OK != m_Mediator->ResetMap()) throw MCStreamException("Failed to m_Mediator->ResetMap()");
	//	return reinterpret_cast<HANDLE>(m_Mediator->GetEvent());
	//#endif
	//CATCH_THROW("CMediatorRefImpl::GetHandleMsgEvent")
	//}

public:

	/// Returns pointer to interface of IMMediator ActiveX object
	STDMETHOD(get_Mediator)(IUnknown** mediator)
	{
	TRY_CATCH
		*mediator = m_Mediator;
	CATCH_LOG("CMediatorRefImpl::get_Mediator")
		return S_OK;
	}

	/// Stores pointer to interface of IMMediator ActiveX object into internal memeber
	STDMETHOD(put_Mediator)(IUnknown* mediator)
	{
	TRY_CATCH
		m_Mediator = mediator;
		m_Mediator->AddRef();
	CATCH_LOG("CMediatorRefImpl::put_Mediator")
		return S_OK;
	}

};

