/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CThreadLSInitializer.h
///
///  Declares CThreadLSInitializer class, responsible for initialization
///    Thread Local Storage for process
///
///  @author Dmitry Netrebenko @date 02.03.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/AidLib.h>

/// CThreadLSInitializer class, responsible for initialization
///    Thread Local Storage for process
class AIDLIB_API CThreadLSInitializer
{
private:
/// Index of storage
	DWORD	m_tlsIndex;

public:
/// Constructor
	CThreadLSInitializer();

/// Destructor
	~CThreadLSInitializer();

/// Returns index of storage
	DWORD Index() const;
};

#define ThreadInitializer CProcessSingleton<CThreadLSInitializer>::instance()
