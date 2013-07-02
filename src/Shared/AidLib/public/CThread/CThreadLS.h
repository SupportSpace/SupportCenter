/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CThreadLS.h
///
///  Declares CThreadLS class, responsible for allocation and filling
///    local storage for thread
///
///  @author Dmitry Netrebenko @date 02.03.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include "CThreadLSInitializer.h"
#include <AidLib/Strings/tstring.h>

///  CThreadLS class, responsible for allocation and filling local storage for thread
class CThreadLS
{
private:
/// Poiter to local storage
	LPVOID m_data;

public:
/// Constructor
/// @param Func - function name
	CThreadLS(const TCHAR* Func)
		:	m_data(NULL)	
	{
		try
		{
			/// Extract class name
			tstring tmp(Func);
			tstring::size_type index = tmp.find(_T("::"));
			if(-1 != index)
				tmp = tmp.substr(0, index);
			/// Get pointer to local storage
			LPVOID threadLS = TlsGetValue(ThreadInitializer.Index());
			/// Allocate memory for storage if it is not allocated
			if (!threadLS)
				m_data = threadLS = (LPVOID) LocalAlloc(LPTR, MAX_PATH); 
			if(threadLS)
			{
				/// Store class name into local storage
				memset(threadLS, 0, MAX_PATH);
				memcpy(threadLS, tmp.c_str(), min(tmp.length(), MAX_PATH - 1));
				TlsSetValue(ThreadInitializer.Index(), threadLS); 
			}
		}
		catch(...)
		{
		}
	}

/// Destructor
	~CThreadLS()
	{
		try
		{
			/// Deallocate local storage if it is allocated
			if(m_data)
			{
				LocalFree((HLOCAL)m_data); 
				TlsSetValue(ThreadInitializer.Index(), NULL);
			}
		}
		catch(...)
		{
		}
	}
};

#define SET_THREAD_LS CThreadLS LocalTLS(_T(__FUNCTION__))
