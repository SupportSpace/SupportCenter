/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSocketClientConnectThread.h
///
///  Declares CSocketClientConnectThread class, responsible for socket client connection
///
///  @author Dmitry Netrebenko @date 11.10.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include "CSocketConnectThread.h"

/// forvard declaration
class CSSocket;

///  Class, responsible for socket client connection
///  Base class is CSocketConnectThread
class CSocketClientConnectThread :
	public CSocketConnectThread
{
private:
/// Prevents making copies of CSocketClientConnectThread objects.
	CSocketClientConnectThread( const CSocketClientConnectThread& );				
	CSocketClientConnectThread& operator=( const CSocketClientConnectThread& );	

public:
///  Constructor
	CSocketClientConnectThread();

///  Thread's entry point
///  @param   Pointer to thread's parameters
	void Execute(void*);

///  Returns pointer to socket
///  @return Pointer to VSocket class
	SPSocket GetConnectedSocket() const;
};
