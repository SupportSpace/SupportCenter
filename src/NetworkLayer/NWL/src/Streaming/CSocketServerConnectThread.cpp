/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSocketServerConnectThread.cpp
///
///  Implements CSocketServerConnectThread class, responsible for socket server connection
///
///  @author Dmitry Netrebenko @date 11.10.2006
///
////////////////////////////////////////////////////////////////////////

#include <NWL/Streaming/CSSocket.h>
#include <NWL/Streaming/CSocketServerConnectThread.h>
#include <NWL/Streaming/CStreamException.h>
#include <AidLib/CThread/CThreadLS.h>

CSocketServerConnectThread::CSocketServerConnectThread()
: CSocketConnectThread(), m_sClientSocket()
{
TRY_CATCH

CATCH_THROW("CSocketServerConnectThread::CSocketServerConnectThread")
}

void CSocketServerConnectThread::Execute(void*)
{
	SET_THREAD_LS;

TRY_CATCH

	// Check socket
	if ( !m_sSocket.get() )
		throw MCStreamException( _T("Socket object does not created") );

	// Bind socket
	if ( !m_sSocket->Bind( m_nPort ) )
		throw MCSocketStreamException( _T("Bind method failed") );

	// Listen socket
	if ( !m_sSocket->Listen() )
		throw MCSocketStreamException( _T("Listen method failed") );

	while ( !Terminated() )
	{

		// Set accept timeout
		if ( m_sSocket->ReadSelect( 100 ) )
		{
			if ( Terminated() )
				break;

			// Check client connection
			SPSocket sock(m_sSocket->Accept());
			
			if ( sock.get() )
			{
				m_sClientSocket = sock;
				// Raise "Connect" event
				if ( m_ConnectedEvent )
					m_ConnectedEvent( NULL );
				m_sSocket->Close();			
				break;
			}
			else
				if (!Terminated()) throw MCSocketStreamException( _T("Accept method failed") );
		}
	}
	//if ( Terminated() )
	//{
	//	if ( m_ConnectErrorEvent )
	//		m_ConnectErrorEvent( NULL );
	//}
	return;

CATCH_LOG("CSocketServerConnectThread::Execute")

	if ( m_ConnectErrorEvent && !Terminated() )
		m_ConnectErrorEvent( NULL, cerWinError );
}

SPSocket CSocketServerConnectThread::GetConnectedSocket() const
{
TRY_CATCH

	return m_sClientSocket;

CATCH_THROW("CSocketServerConnectThread::GetConnectedSocket")
}
