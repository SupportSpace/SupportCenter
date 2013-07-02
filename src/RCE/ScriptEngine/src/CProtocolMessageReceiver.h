/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CProtocolMessageReceiver.h
///
///  Declares CProtocolMessageReceiver class, responsible for thread for 
///    receiving protocol messages
///
///  @author Dmitry Netrebenko @date 16.11.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <AidLib/CThread/CThread.h>
#include "SScriptEngineMsg.h"
#include <NWL/Streaming/CAbstractStream.h>

///  CProtocolMessageReceiver class, responsible for thread for 
///    receiving protocol messages
///  Base class CThread - thread object from AidLib
class CProtocolMessageReceiver 
	:	public CThread
{
private:
/// Prevents making copies of CProtocolMessageReceiver objects
	CProtocolMessageReceiver(const CProtocolMessageReceiver&);
	CProtocolMessageReceiver& operator=(const CProtocolMessageReceiver&);
public:
/// Constructor
	CProtocolMessageReceiver();
/// Destructor
	virtual ~CProtocolMessageReceiver();
/// Initializes thread for receiving messages
/// @param stream - transport stream
	void InitReceiver(boost::shared_ptr<CAbstractStream> stream);
/// Stops receiving of protocol messages
	void StopReceiver();
/// Receive thread's entry point
	virtual void Execute(void *Params);
/// Checks up is receiver initialized
	bool IsInited() const
	{
		return m_inited;
	}
private:
/// Shared pointer to transport stream
	boost::shared_ptr<CAbstractStream>	m_stream;
/// Is receiver is initialized
	bool								m_inited;
protected:
/// Abstract method to handle received message
/// @param requestId - id of request
/// @param msgType - type of message
/// @param msgData - buffer with message's data
/// @param size - buffer size
	virtual void HandleProtocolMessage(unsigned int requestId, EScriptEngineMsgType msgType, boost::shared_array<char> msgData, const unsigned int size) = NULL;
/// Raised when reciever's thread is stopped
	virtual void OnReceiverStopped() {};
};
