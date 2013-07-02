/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSocketProxyClientConnectThread.cpp
///
///  Implements CSocketProxyClientConnectThread class, responsible for socket 
///    client connection through HTTP proxy
///
///  @author Dmitry Netrebenko @date 16.10.2006
///
////////////////////////////////////////////////////////////////////////

#include <NWL/Streaming/CSSocket.h>
#include <NWL/Streaming/CSocketProxyClientConnectThread.h>
#include <NWL/Streaming/CStreamException.h>
#include <AidLib/CThread/CThreadLS.h>

CSocketProxyClientConnectThread::CSocketProxyClientConnectThread()
: CSocketClientConnectThread(), m_ProxySettings()
{
TRY_CATCH

CATCH_THROW("CSocketProxyClientConnectThread::CSocketProxyClientConnectThread")
}

void CSocketProxyClientConnectThread::Execute(void* sync_call)
{
	SET_THREAD_LS;

try
{
	// Check socket
	if ( !m_sSocket.get() )
		throw MCStreamException( _T("Socket object does not created") );

	while ( !Terminated() || sync_call )
	{

		if ( m_sSocket->Connect( (char*)m_ProxySettings.ProxyURL.c_str(), m_ProxySettings.ProxyPort ) )
		{
			ConnectThroughProxy(reinterpret_cast<bool>(sync_call));

			if ( Terminated() && !sync_call )
				break;

			// Raise "Connected" event
			if ( m_ConnectedEvent )
				m_ConnectedEvent( NULL );
			break;
		}
		else
		if ( !Terminated() || sync_call )
		{
			DWORD dwError = WSAGetLastError();
			if ( WSAETIMEDOUT != dwError || sync_call )
			{
				m_sSocket->Close();
				throw MCStreamException_Win( _T("Connect to proxy method failed"), dwError );
			}
		}
		if (sync_call) break;
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

	m_sSocket->Close();

	if ( m_ConnectErrorEvent && !Terminated() )
		m_ConnectErrorEvent( NULL, cerWinError );

}

SHTTPProxySettings& CSocketProxyClientConnectThread::GetProxySettings()
{
TRY_CATCH

	return m_ProxySettings;

CATCH_THROW("CSocketProxyClientConnectThread::GetProxySettings")
}
	
void CSocketProxyClientConnectThread::SetProxySettings( const SHTTPProxySettings& proxy )
{
TRY_CATCH

	m_ProxySettings = proxy;

CATCH_THROW("CSocketProxyClientConnectThread::SetProxySettings")
}

void CSocketProxyClientConnectThread::ConnectThroughProxy(bool sync_call)
{
TRY_CATCH

	tstring strRequest = Format( _T("CONNECT %s:%d HTTP/1.0\r\n\r\n"), m_strHost.c_str(), m_nPort  );
	unsigned int len = (unsigned int)strRequest.length();
	const int BUF_SIZE = 20;//8192;
	tstring strAnswer( _T("HTTP/1.0 200") );
	tstring strEndHeader( _T("\r\n\r\n") );

	char buf[BUF_SIZE];
	unsigned int bytes_num = 0;
	unsigned int count = 0;
	bool found = false;
	bool isfirst = true;
	const tstring::size_type npos = -1;
	tstring::size_type index;

	if ( !m_sSocket->Send( strRequest.c_str(), len ) )
		throw MCStreamException( _T("Send to proxy method failed") );

	// Wait for proxy answer
	while ( (!Terminated() || sync_call) && !m_sSocket->ReadSelect( 0 ) )
		Sleep( 100 );

	// Check thread termination
	if ( Terminated() && !sync_call )
		return;

	do {
		// Clear buffer
		FillMemory( buf, BUF_SIZE, 0 );
		// Peeks incoming data
		bytes_num = m_sSocket->Peek( buf, BUF_SIZE - 1 );

		// Raise exception if there is no incoming data
		if ( !bytes_num )
			throw MCStreamException( _T("Receive from proxy failed") );

		tstring strBuf( buf );
	
		// For first step
		if ( isfirst )
		{
			// Find proxy answer in the incoming data
			found = ( 0 == strBuf.find( strAnswer ) );
			isfirst = false;
		}

		// If OK answer found
		if ( found )
		{
			// Find header end
			index = strBuf.find( strEndHeader );
			if ( npos == index )
				// End of header is not found.
				// Receive all incoming data
				count = m_sSocket->Receive( buf, bytes_num );
			else
				// End of header is found.
				// Receive header only
				count = m_sSocket->Receive( buf, (unsigned int)(index + strEndHeader.length()) );

			// Raise exception if can not receive data
			if ( !count )
				throw MCStreamException( _T("Receive from proxy failed") );
		}

	// Recieve the rest of header
	} while ( (!Terminated() || sync_call ) && ( bytes_num == BUF_SIZE - 1 ) && found );

	// Proxy answer not found
	if ( !found )
		throw MCStreamException( _T("Connection through proxy failed") );

CATCH_THROW("CSocketProxyClientConnectThread::ConnectThroughProxy")
}
