/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CLogTransportFactory.cpp
///
///  Implements CLogTransportFactory class, transport layer factory for NetLog
///
///  @author Dmitry Netrebenko @date 30.03.2007
///
////////////////////////////////////////////////////////////////////////

#include "CLogTransportFactory.h"
#include "CLocalNetworkLogTransport.h"
#include "CLocalNetworkComLogTransport.h"

CLogTransportFactory::CLogTransportFactory()
{
}

CLogTransportFactory::~CLogTransportFactory()
{
}

SPLogTransportLayer CLogTransportFactory::GetTransportLayer(const tstring& name, CNetworkLog& netLog, tstring& errorString)
{
	HRESULT hResult = S_OK;
	try
	{
		errorString.clear();
		IComUDPListenerPtr comUDPListener; 
		hResult = comUDPListener.CreateInstance(__uuidof(CComUDPListener), 0, CLSCTX_LOCAL_SERVER);
		if ( S_OK == hResult )
		{
			comUDPListener->AddWatch(GetCurrentProcessId());
			return SPLogTransportLayer(new CLocalNetworkComLogTransport(comUDPListener, name, netLog));
		}
		else
		{
			return SPLogTransportLayer(new CLocalNetworkLogTransport(name, netLog));
		}
	}
	catch(CExceptionBase &e)
	{
		errorString = Format(_T("\nComError:\n%d\nSockError:\n%s"),hResult, e.m_strWhat.c_str());
		return SPLogTransportLayer();
	}
	catch(...)
	{
		errorString = Format(_T("\nComError:\n%d\nSockError:\n%s"),hResult, _T("Unkwnown exception"));
		return SPLogTransportLayer();
	}
}
