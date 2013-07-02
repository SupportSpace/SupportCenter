/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CStreamFactoryRelayedImpl.h
///
///  Declares CStreamFactoryRelayedImpl class, responsible for steam factory
///    with initial message exchange through relay server
///
///  @author Dmitry Netrebenko @date 25.09.2005
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include "CStreamFactory.h"
#include <NWL/NetworkLayer.h>
#include <AidLib/Strings/tstring.h>
#include "CAbstractNetworkStream.h"
#include <boost/shared_ptr.hpp>
#include <NWL/Streaming/CRelayedNetworkStream.h>
#include <NWL/Streaming/CRSASocketStream.h>

#define MAX_INITIAL_MSG_SIZE 4096

///  CStreamFactoryRelayedImpl class, responsible for steam factory
///    with initial message exchange through relay server
///  Base class - CStreamFactory (abstract stream factory)
class NWL_API CStreamFactoryRelayedImpl
	:	public CStreamFactory
{
private:
	/// Prevents making copies of CStreamFactoryRelayedImpl objects.
	CStreamFactoryRelayedImpl(const CStreamFactoryRelayedImpl&);
	CStreamFactoryRelayedImpl& operator=(const CStreamFactoryRelayedImpl&);

public:
	/// Constructor
	CStreamFactoryRelayedImpl();
	/// Destructor
	virtual ~CStreamFactoryRelayedImpl();

protected:
	/// Sends a message to the specified destination peer (user ID).  
	/// @param peerId destination peer
	/// @param messageData Message data is an arbitrary string.
	virtual void SendMsg(const tstring& peerId, const tstring& messageData);

	/// Handles an incoming message from a specified source peer (user ID).
	/// @param peerId source peer
	/// @param messageData Message data is an arbitrary string.
	virtual void HandleMsg(const tstring& peerId, tstring& messageData);

	/// Returns event handle, which handles incomming IM messages
	/// @return event handle, which handles incomming IM messages
	virtual HANDLE GetHandleMsgEvent(); 

	/// Initializes message exchanger
	/// @param sourcePeerId source peerId
	/// @param destPeerId destination peedId
	/// @param timeOut connection timeout
	/// @param masterRole peer's role
	virtual void InitExchanger(const tstring& sourcePeerId, const tstring& destPeerId, int timeOut, bool masterRole);

	/// Closes message exchanger
	/// @param masterRole peer's role
	virtual void CloseExchanger(bool masterRole);

	/// Aborts exchanger
	virtual void AbortExchanger();

	/// Stream to exchange of initial messages through relay server
	CRelayedNetworkStream<CRSASocketStream>	m_initialStream;
};
