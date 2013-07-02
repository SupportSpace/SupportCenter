/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSocketConnectThread.h
///
///  Declares CSocketConnectThread class, responsible for socket connection
///
///  @author Dmitry Netrebenko @date 11.10.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/CThread/CThread.h>
#include <NWL/Events/Events.h>
#include <AidLib/Strings/tstring.h>
#include <boost/shared_ptr.hpp>

/// forvard declaration
class CSSocket;

///  Socket shared pointer
typedef boost::shared_ptr<CSSocket> NWL_API SPSocket;

///  Class, responsible for socket connection
///  Base class is CThread
class CSocketConnectThread :
	public CThread
{
private:
/// Prevents making copies of CSocketConnectThread objects.
	CSocketConnectThread( const CSocketConnectThread& );				
	CSocketConnectThread& operator=( const CSocketConnectThread& );	

public:
///  Constructor
	CSocketConnectThread();
	
///  Destructor
	~CSocketConnectThread();

protected:
	NotifyEvent			m_ConnectedEvent;
	ConnectErrorEvent	m_ConnectErrorEvent;
	tstring				m_strHost;
	unsigned int		m_nPort;
	SPSocket			m_sSocket;

public:
///  Initializes thread
///  @param   Pointer to CSSocket object
///  @param   connected event wrapper
///  @param   error occured event wrapper
///  @param   Host name
///  @param   Port
	void Init( SPSocket, NotifyEvent, ConnectErrorEvent, const tstring&, const unsigned int );

///  Unbinds events
///  @remarks
	void UnBindEvents();

///  Returns pointer to socket
///  @return  Pointer to VSocket class
	virtual SPSocket GetConnectedSocket() const = NULL;
};
