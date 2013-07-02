/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CThreadLSInitializer.cpp
///
///  Implements CThreadLSInitializer class, responsible for initialization
///    Thread Local Storage for process
///
///  @author Dmitry Netrebenko @date 02.03.2007
///
////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <AidLib/CThread/CThreadLSInitializer.h>

CThreadLSInitializer::CThreadLSInitializer()
	:	m_tlsIndex(TLS_OUT_OF_INDEXES)
{
	try
	{
		/// Allocate TLS and store index of storage
		m_tlsIndex = TlsAlloc();
	}
	catch(...)
	{
	}
}

CThreadLSInitializer::~CThreadLSInitializer()
{
	try
	{
		/// Get thread's data and deallocate if not NULL
		LPVOID data = TlsGetValue(m_tlsIndex); 
		if(data)
			LocalFree((HLOCAL)data); 
		/// Deallocate TLS
		TlsFree(m_tlsIndex);
	}
	catch(...)
	{
	}
}

DWORD CThreadLSInitializer::Index() const 
{ 
	return m_tlsIndex; 
};
