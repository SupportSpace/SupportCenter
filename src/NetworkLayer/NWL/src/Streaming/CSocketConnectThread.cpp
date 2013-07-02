/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSocketConnectThread.cpp
///
///  Implements CSocketConnectThread class, responsible for socket connection
///
///  @author Dmitry Netrebenko @date 11.10.2006
///
////////////////////////////////////////////////////////////////////////

#include <NWL/Streaming/CSocketConnectThread.h>
#include <NWL/Streaming/CStreamException.h>

CSocketConnectThread::CSocketConnectThread()
	:	m_strHost( _T("") )
	,	m_nPort( 0 )
	,	m_sSocket()
	,	m_ConnectedEvent( NULL )
	,	m_ConnectErrorEvent( NULL )
	,	CThread()
{
TRY_CATCH

CATCH_THROW("CSocketConnectThread::CSocketConnectThread")
}

CSocketConnectThread::~CSocketConnectThread()
{
TRY_CATCH

CATCH_LOG("CSocketConnectThread::~CSocketConnectThread")
}

void CSocketConnectThread::Init( SPSocket ssocket, NotifyEvent ConnectedWrap, ConnectErrorEvent ErrorOcurred, const tstring& host, const unsigned int port )
{
TRY_CATCH

	if ( !ssocket )
		throw MCStreamException( _T("Socket does not created") );

	m_strHost = host;
	m_nPort = port;
	m_sSocket = ssocket;
	m_ConnectedEvent = ConnectedWrap;
	m_ConnectErrorEvent = ErrorOcurred;

CATCH_THROW("CSocketConnectThread::Init")
}

void CSocketConnectThread::UnBindEvents()
{
TRY_CATCH

	m_ConnectedEvent = NULL;
	m_ConnectErrorEvent = NULL;

CATCH_THROW("CSocketConnectThread::UnBindEvents")
}
