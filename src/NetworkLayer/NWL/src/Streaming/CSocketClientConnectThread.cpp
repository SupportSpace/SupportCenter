/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSocketClientConnectThread.cpp
///
///  Implements CSocketClientConnectThread class, responsible for socket client connection
///
///  @author Dmitry Netrebenko @date 11.10.2006
///
////////////////////////////////////////////////////////////////////////

#include <NWL/Streaming/CSSocket.h>
#include <NWL/Streaming/CSocketClientConnectThread.h>
#include <NWL/Streaming/CStreamException.h>
#include <AidLib/CThread/CThreadLS.h>

CSocketClientConnectThread::CSocketClientConnectThread()
: CSocketConnectThread()
{
TRY_CATCH

CATCH_THROW("CSocketClientConnectThread::CSocketClientConnectThread")
}

void CSocketClientConnectThread::Execute(void* sync_call)
{
	SET_THREAD_LS;

try
{
	// Check socket
	if ( !m_sSocket.get() )
		throw MCStreamException( _T("Socket object does not created") );

	while ( (!Terminated()) || sync_call )
	{

		if ( m_sSocket->Connect( (char*)m_strHost.c_str(), m_nPort ) )
		{
			// Raise "Connected" event
			if ( m_ConnectedEvent )
				m_ConnectedEvent( NULL );
			break;
		}
		else
		{
			DWORD dwError = WSAGetLastError(); 
			if (!Terminated() && !sync_call && WSAECONNREFUSED == dwError)
			{
				SetLastError(dwError);
				Log.WinError(_ERROR_, _T("Connect method failed. "));
				continue;
			}
			if ( !Terminated() || sync_call )
			{
				if ( WSAETIMEDOUT != dwError || sync_call )
				{
					m_sSocket->Close();
					throw MCStreamException_Win( _T("Connect method failed"), dwError );
				}
			}
			if (sync_call) 
				break;
		}
	}
	return;

}
catch(CStreamException &e)
{
	if (sync_call)
		throw e;
	else
		MLog_Exception(e);
}
catch(CExceptionBase &e)
{
	if (sync_call)
		throw e;
	else
		MLog_Exception(e);
}
catch(std::exception &e)
{
	if (sync_call)
		throw e;
	else
		MLog_Exception(MCStreamException(e.what()));
}
catch(...)
{
	if (sync_call)
		throw MCStreamException("Unknown exception");
	else
		MLog_Exception(MCStreamException("Unknown exception"));
}
	
	if ( m_ConnectErrorEvent && !Terminated() )
		m_ConnectErrorEvent( NULL, cerWinError );
}

SPSocket CSocketClientConnectThread::GetConnectedSocket() const
{
TRY_CATCH

	return m_sSocket;

CATCH_THROW("CSocketClientConnectThread::GetConnectedSocket")
}
