//////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  COpenPortRequest.h
///
///  Declares COpenPortRequest class
///  Allow testing by server the open port on client side
///
///  @author Alexander Novak @date 26.06.2007
///
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/Streaming/CSSocket.h>
#include <NWL/Streaming/CSocketStream.h>
#include <NWL/Streaming/relay_messages.h>
#include <NWL/Streaming/CNetworkLayer.h>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/shared_ptr.hpp>
//========================================================================================

///  For internal use only. Thread for incoming connect from server side
class CListenPortThread: public CThread
{
	CSSocket		m_socket;
	unsigned short	m_port;
	bool			m_badCommunicationSequence;
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > m_hEvntDone;
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > m_hEvntBind;
protected:
	/// Redefine ThreadProc and provide communications protocol
	void Execute(void*);
public:
	CListenPortThread();

	/// Start to listen socket
	/// @param portNumber	Port which will be tested by server side
	/// @remarks	Don't call StartForCommunication without pair call WaitForCommunication
	void StartForCommunication(unsigned short portNumber);

	/// Wait for server response
	/// @param timeout		Timeout for server answer
	/// @return		Return true if comunication was successful
	bool WaitForCommunication(int timeout=INFINITE);
};
//////////////////////////////////////////////////////////////////////////////////////////

class NWL_API COpenPortRequest
{
	tstring				m_server_address;
	unsigned short		m_server_port;
	CSocketStream		m_socket_authentication;
	CListenPortThread	m_thread_listen_port;
	
	COpenPortRequest(const COpenPortRequest&);
	COpenPortRequest& operator=(const COpenPortRequest&);
public:
	COpenPortRequest(const tstring& serverAddress, unsigned short serverPort);
	~COpenPortRequest();

	/// Open internal port for listening and wait for server connect via external port
	/// @param userID		User identifier
	/// @param password		User password
	/// @param inPort		The internal port number what will be checked
	/// @param exPort		The external port number what will be checked
	void CheckPortAvailability(	const tstring& userID,
								const tstring& password,
								unsigned short inPort,
								unsigned short exPort);
};
//////////////////////////////////////////////////////////////////////////////////////////
