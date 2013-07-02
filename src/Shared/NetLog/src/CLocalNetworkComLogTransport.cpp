/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CLocalNetworkComLogTransport.cpp
///
///  Implements CLocalNetworkComLogTransport class, transport layer for local 
///    network which uses additional COM object for UDP listening
///
///  @author Dmitry Netrebenko @date 30.03.2007
///
////////////////////////////////////////////////////////////////////////

#include "CLocalNetworkComLogTransport.h"
#include <NWL/Streaming/CStreamException.h>
#include <NetLog/SUDPListenerMsg.h>
#include <NetLog/CNetLog.h>

CLocalNetworkComLogTransport::CLocalNetworkComLogTransport(const tstring& name, CNetworkLog& netLog)
	:	CLogTransportLayer(name, netLog)
	,	CComUDPListenerEventHandler()
	,	m_tcpListener(name, netLog)
	,	m_socket(stUDP)
{
	InitTransport();
	
	HRESULT hResult = m_comUDPListener.CreateInstance(__uuidof(CComUDPListener), 0, CLSCTX_LOCAL_SERVER);
	if(S_OK != hResult)
		throw MCSocketStreamException(_T("Can not create instance of ComUDPListener Object"));

	hResult = DispEventAdvise(m_comUDPListener);
	if(S_OK != hResult)
		throw MCSocketStreamException(_T("Can not advise to ComUDPListener events"));

	if(S_OK != m_comUDPListener->Listen(NETLOG_UDP_PORT))
		throw MCSocketStreamException(_T("Can not start ComUDPListener thread"));

	m_tcpListener.Start();
}

CLocalNetworkComLogTransport::CLocalNetworkComLogTransport(IComUDPListenerPtr comListener, const tstring& name, CNetworkLog& netLog)
	:	CLogTransportLayer(name, netLog)
	,	CComUDPListenerEventHandler()
	,	m_tcpListener(name, netLog)
	,	m_socket(stUDP)
	,	m_comUDPListener(comListener)
{
	InitTransport();

	HRESULT hResult = DispEventAdvise(m_comUDPListener);
	if(S_OK != hResult)
		throw MCSocketStreamException(_T("Can not advise to ComUDPListener events"));

	try
	{
		if(S_OK != m_comUDPListener->Listen(NETLOG_UDP_PORT))
			throw MCSocketStreamException(_T("Can not start ComUDPListener thread"));
	}
	catch(...)
	{
		DispEventUnadvise(m_comUDPListener);
		m_socket.Close();
		throw;
	}

	m_tcpListener.Start();
}

void CLocalNetworkComLogTransport::InitTransport()
{
	// Create socket
	if(!m_socket.Create())
		throw MCSocketStreamException(_T("Can not create UDP socket."));
	// Bind socket
	if(!m_socket.Bind())
		throw MCSocketStreamException(_T("Can not bind UDP socket."));

	// Get process name
	TCHAR buf[MAX_PATH];
	GetModuleFileName(NULL, buf, MAX_PATH);
	m_processName = buf;

	tstring::size_type index = m_processName.find_last_of(_T("\\"));
	if(-1 != index)
		m_processName.erase(0, index + 1);

	m_processId = GetCurrentProcessId();

	m_tcpListenerPort = m_tcpListener.GetPort();
}

CLocalNetworkComLogTransport::~CLocalNetworkComLogTransport()
{
	try
	{
		DispEventUnadvise(m_comUDPListener);
		m_tcpListener.Terminate();
		m_socket.Close();
	}
	catch(...)
	{
	}
}

void CLocalNetworkComLogTransport::SendResponseTo(const tstring& addr, const unsigned int port)
{
	try
	{
		// Calculate size of response message
		unsigned int size = NETLOG_UDP_MESSAGE_HEADER_SIZE + static_cast<unsigned int>(m_processName.length());
		// Allocate memory for message
		SPUDPListenerMsg msg(reinterpret_cast<SUDPListenerMsg*>(new char[size]));
		// Set size
		msg->m_size = size;
		// Set process id
		msg->m_process = m_processId;
		// Set port of TCPListener
		msg->m_tcpPort = m_tcpListenerPort;
		// Copy process name to m_data field
		memcpy(msg->m_data, m_processName.c_str(), m_processName.length());
		// Send message through UDP
		m_socket.SendTo(addr, port, reinterpret_cast<char*>(msg.get()), msg->m_size);
	}
	catch(...)
	{
	}
}

HRESULT __stdcall CLocalNetworkComLogTransport::OnDatagramReceived (BSTR addr, long port)
{
	try
	{
		USES_CONVERSION;
		tstring tmp(OLE2T(addr));
		SendResponseTo(tmp, port);
		return S_OK;
	}
	catch(...)
	{
		return E_FAIL;
	}
}
