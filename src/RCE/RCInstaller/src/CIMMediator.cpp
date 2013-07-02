/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CIMMediator.cpp
///
///  Implements CIMMediator class, responsible for IMMediator ActiveX
///
///  @author Dmitry Netrebenko @date 26.12.2006
///
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CIMMediator.h"
#include <AidLIb/CCritSection/CCritSection.h>

// CIMMediator interface ----------------------------------------------------

STDMETHODIMP CIMMediator::HandleMsgInternal(BSTR peerId, BSTR msg)
{
TRY_CATCH
	CSingleton<CCrtExeptionHook>::instance();
	// Add message to internal map
	USES_CONVERSION;
	AddMsgToMap( OLE2T(peerId), OLE2T(msg) );

	// Set event
	SetEvent( m_hEvent );

	return S_OK;

CATCH_LOG("CIMMediator::HandleMsgInternal")

	return AtlReportError ( GetObjectCLSID(), _T("HandleMsgInternal failed") );
}

STDMETHODIMP CIMMediator::SendMsg(BSTR peerId, BSTR msg)
{
TRY_CATCH

	// Raise OnSendMessage event
	PostMessage(m_msgFireEventOtherThreadsSendMsg, reinterpret_cast<WPARAM>(new CComBSTR(peerId)), reinterpret_cast<LPARAM>(new CComBSTR(msg)));

CATCH_LOG("CIMMediator::SendMessage")

	return S_OK;
}

STDMETHODIMP CIMMediator::HandleMsg(BSTR peerId, BSTR* msg)
{
TRY_CATCH

	USES_CONVERSION;

	// Get peer
	tstring peer( OLE2T(peerId) );

	// Get message from map
	tstring message = GetMsgFromMapByPeerId( peer );

	*msg = ::SysAllocString( T2OLE( message.c_str() ) );

	return S_OK;

CATCH_LOG("CIMMediator::HandleMsg")

	return AtlReportError ( GetObjectCLSID(), _T("HandleMsg failed") );
}

STDMETHODIMP CIMMediator::ResetMap()
{
TRY_CATCH

	// Clear map
	ClearMap();
	// Reset event
	ResetEvent( m_hEvent );

	return S_OK;

CATCH_LOG("CIMMediator::ResetMap")

	return AtlReportError ( GetObjectCLSID(), _T("ResetMap failed") );
}

STDMETHODIMP CIMMediator::GetEvent(/*HANDLE*/LONGLONG* hEvent)
{
TRY_CATCH

	*hEvent = reinterpret_cast<LONGLONG>(m_hEvent);

	return S_OK;

CATCH_LOG("CIMMediator::GetEvent")

	return AtlReportError ( GetObjectCLSID(), _T("GetEvent failed") );
}

//<-----------------------------------------------------------------------------

CIMMediator::CIMMediator()
	:	CInstanceTracker(_T("IMMediator ActiveX")),
		m_msgFireEventOtherThreadsSendMsg(::RegisterWindowMessage(_T("CIMMediator::FireEventOtherThreadsSendMsg")))
{
TRY_CATCH

	REPORT_MODULE(RCINSTALLER_NAME);

	// Init critical section
	InitializeCriticalSection( &m_MapSection );
	m_bWindowOnly=TRUE;//for fire event in other thread

	// Create event object
	m_hEvent = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		NULL );
 
	if ( !m_hEvent )
		throw MCException( _T("Event creation failed") );

CATCH_THROW("CIMMediator::CIMMediator")
}

CIMMediator::~CIMMediator()
{
TRY_CATCH

	// Delete critical section
	DeleteCriticalSection( &m_MapSection );

	// Destroy event
	CloseHandle( m_hEvent );
	
	//TCHAR threadId[200];
	//_stprintf_s(threadId,_T("%x %x"),::GetCurrentProcessId(),::GetCurrentThreadId());
	//::MessageBox(NULL,_T("CIMMediator::~CIMMediator()"),threadId,0);

CATCH_LOG("CIMMediator::~CIMMediator")
}

void CIMMediator::AddMsgToMap( const tstring& peer, const tstring& msg )
{
TRY_CATCH

	// Enter critical section
	CCritSection section( &m_MapSection );

	m_MessagesMap[LowerCase( peer )] = msg;

CATCH_THROW("CIMMediator::AddMsgToMap")
}

tstring CIMMediator::GetMsgFromMapByPeerId( const tstring& peer )
{
TRY_CATCH

	// Enter critical section
	CCritSection section( &m_MapSection );

	// Search for peer
	MsgMap::iterator index = m_MessagesMap.find( LowerCase( peer ) );

	// Exit if not exists
	if( index == m_MessagesMap.end() )
		return _T("");

	return index->second;

CATCH_THROW("CIMMediator::GetMsgFromMapByPeerId")
}

LRESULT CIMMediator::FireEventOtherThreadsSendMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	OnSendMessage( *reinterpret_cast<CComBSTR*>(wParam)/*peerId*/, *reinterpret_cast<CComBSTR*>(lParam)/*msg*/ );	
	delete reinterpret_cast<CComBSTR*>(wParam);
	delete reinterpret_cast<CComBSTR*>(lParam);	

CATCH_LOG("CIMMediator::FireEventOtherThreadsSendMsg")

	return 0;
}

void CIMMediator::ClearMap()
{
TRY_CATCH

	// Enter critical section
	CCritSection section( &m_MapSection );

	// Clear map
	m_MessagesMap.clear();

CATCH_THROW("CIMMediator::ClearMap")
}
