/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSocketServerConnectThread.h
///
///  Declares CSocketServerConnectThread class, responsible for socket server connection
///
///  @author Dmitry Netrebenko @date 11.10.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include "CSocketConnectThread.h"

/// forvard declaration
class CSSocket;


///  Class, responsible for socket server connection
///  Base class is CSocketConnectThread
class CSocketServerConnectThread :
	public CSocketConnectThread
{
private:
/// Prevents making copies of CSocketServerConnectThread objects.
	CSocketServerConnectThread( const CSocketServerConnectThread& );				
	CSocketServerConnectThread& operator=( const CSocketServerConnectThread& );	

protected:
	SPSocket			m_sClientSocket;

public:
///  Constructor
	CSocketServerConnectThread();

///  Thread's entry point
///  @param   Pointer to thread's parameters
	void Execute(void*);

///  Returns pointer to socket
///  @return  Pointer to CSSocket class
	SPSocket GetConnectedSocket() const;
};
