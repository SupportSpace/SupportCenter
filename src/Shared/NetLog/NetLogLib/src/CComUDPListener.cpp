/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CComUDPListener.cpp
///
///  Implements CComUDPListener class, UDP listener COM object 
///
///  @author Dmitry Netrebenko @date 02.04.2007
///
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CComUDPListener.h"
#include <NWL/Streaming/CStreamException.h>
#include <AidLib/CThread/CThreadLS.h>
#include <AidLib/CCritSection/CCritSection.h>
#include <comdef.h>
#include <NWL/Streaming/CFirewallConfigurator.h>
#include <AidLib/CSingleton/CSingleton.h>

CComUDPListener::CComUDPListener()
	:	CThread()
	,	m_socket(stUDP)
	,	m_started(false)
{
	try
	{
		// Configure firewall for incoming connections
		FIREWALL_CONFIGURATOR_INSTANCE.AllowIncoming();
	}
	catch(...)
	{
	}

	// Create socket
	if(!m_socket.Create())
		throw MCSocketStreamException(_T("Can not create UDP socket."));

	// Clear buffer
	memset(m_buffer, 0, NETLOG_UDP_BUFFER_SIZE);

	InitializeCriticalSection(&m_cs);
}

CComUDPListener::~CComUDPListener()
{
	try
	{
		DeleteCriticalSection(&m_cs);
	}
	catch(...)
	{
	}
}

void CComUDPListener::Execute(void*)
{
	DISABLE_TRACE;
	SET_THREAD_LS;
	
	try
	{
		while(!Terminated())
		{
			// Wait for incoming messages
			if(!m_socket.ReadSelect(NETLOG_UDP_SELECT_TIME))
				continue;

			tstring remoteAddr;
			unsigned int remotePort;
			memset(m_buffer, 0, NETLOG_UDP_BUFFER_SIZE);

			// Receive message
			unsigned int received = m_socket.ReceiveFrom(remoteAddr, remotePort, m_buffer, NETLOG_UDP_BUFFER_SIZE);

			if(!received)
				continue;

			// Validating message
			tstring msg(m_buffer);
			if(msg == NETLOG_UDP_SERVER_REQUEST)
			{
				try
				{
					// Standard event mechanism
					//USES_CONVERSION;
					// Raise event
					//BSTR addr = ::SysAllocString(T2OLE(remoteAddr.c_str()));
					//OnDatagramReceived(addr, remotePort);
					//SysFreeString(addr);

					InvokeOnDatagramReceived(remoteAddr, remotePort);
				}
				catch(...)
				{
				}
			}
		}
	}
	catch(...)
	{
	}
}


STDMETHODIMP CComUDPListener::Listen(LONG port)
{
	try
	{
		// Enter critical section
		CCritSection section(&m_cs);

		if(m_started)
			return S_OK;

		// Bind socket
		if(!m_socket.Bind(port))
			throw MCSocketStreamException(_T("Can not bind UDP socket."));

		// Start thread
		Start();

		m_started = true;

		return S_OK;
	}
	catch(...)
	{
		return E_FAIL;
	}
}

STDMETHODIMP CComUDPListener::AddWatch(LONG pid)
{
	try
	{
		m_watchDog.AddClient(pid);
		return S_OK;
	}
	catch(...)
	{
		return E_FAIL;
	}
}

HRESULT CComUDPListener::SafeInvokeEvent(int id, VARIANT* args, int count)
{
	try
	{
		HRESULT hr = S_OK;
		CComUDPListener* _this = static_cast<CComUDPListener*>(this);
		// Get connection point
		IConnectionPointImpl<CComUDPListener, &__uuidof(_IComUDPListenerEvents), CComDynamicUnkArray>* p = _this;
		_this->Lock();
		try
		{
			for(IUnknown** pp = p->m_vec.begin();
				pp < p->m_vec.end();
				++pp)
			{
				if (*pp != NULL) 
				{
					IDispatch* pDispatch = (IDispatch*) *pp;
					DISPPARAMS disp = { args, NULL, count, 0 };
					VARIANT ret_val;
					hr = __ComInvokeEventHandler(pDispatch, id, 1, &disp, &ret_val);
					// Unadvise
					if (FAILED(hr)) 
					{
						*pp = NULL;
					}
				}
			}

		}
		catch(...)
		{
		}
		_this->Unlock();
		return hr;
	}
	catch(...)
	{
		return E_FAIL;
	}
}

HRESULT CComUDPListener::InvokeOnDatagramReceived(const tstring& addr, const unsigned int port)
{
	try
	{
		USES_CONVERSION;

		// Prepare array of params in reverse order
		const int count = 2;
		VARIANT rgvars[2];

		for(int i = 0; i < count; ++i)
			::VariantInit(&rgvars[i]);

		BSTR bstrAddr = ::SysAllocString(T2OLE(addr.c_str()));

		rgvars[1].vt = VT_BSTR;
		V_BSTR(&rgvars[1]) = bstrAddr;
		rgvars[0].vt = VT_I4;
		V_I4(&rgvars[0]) = port;

		// Invoke event
		HRESULT hResult = SafeInvokeEvent(1, rgvars, count);

		// Free array of params
		for(int i = 0; i < count; ++i)
			::VariantClear(&rgvars[i]);

		return hResult;
	}
	catch(...)
	{
		return E_FAIL;
	}
}

