//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CSubStreamDispatcher.h
///
///  Declares a CSubStreamDispatcher class
///  Creates and frees substreams. Dispatches substreams by their priority level.
///  Provides events for substream's service.
///  As a transport stream uses the CAbstractStream through CTransportAdapter.
///  
///  @author Alexander Novak @date 27.09.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/Multiplexer/CTransportAdapter.h>
#include <NWL/Multiplexer/CSubStreamRegistry.h>
#include <boost/type_traits/remove_pointer.hpp>
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class NWL_API CSubStreamDispatcher
{
	struct SServiceCmd
	{
		/// Service command
		unsigned int m_command;

		/// Substream identifier
		unsigned int m_serviceID;

		SServiceCmd(unsigned int command=0, unsigned int serviceID=0)
			:	m_command(command),
				m_serviceID(serviceID){}
	};
	volatile LONG m_transportBroken;

	/// Transport adapter
	CTransportAdapter m_transportAdapter;

	/// Substream's collection
	CSubStreamRegistry m_subStreamRegistry;
	
	/// Queue for service stream emulation
	COutgoingWaitQueue<SServiceCmd> m_serviceStream;
	
	CCritSectionSimpleObject m_csRegistryGuard;

	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > m_threadReceiveData;
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > m_threadSendData;
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > m_eventSendDatagram;
		
	CSubStreamDispatcher(const CSubStreamDispatcher&);
	CSubStreamDispatcher& operator=(const CSubStreamDispatcher&);

	/// Thread for receiving datagram from the transport stream
	static DWORD WINAPI ThreadReceiveData(LPVOID lpParameter);

	/// Thread for sending datagram to the transport stream
	static DWORD WINAPI ThreadSendData(LPVOID lpParameter);

	/// Sends service command into the service stream
	/// @param serviceCmd			Service command
	void SendServiceCmd(const SServiceCmd* serviceCmd);
public:

	/// Creates SubStreamDispatcher with transport stream 
	/// @param transportStream		The transport stream for substreams
	CSubStreamDispatcher(boost::shared_ptr<CAbstractStream> transportStream);

	virtual ~CSubStreamDispatcher();

	/// Create new substream
	/// @param serviceID			Service identifier
	/// @param priorityLevel		Priority level for the substream
	/// @param sizeBuffer			Size of buffer for the substream
	/// @remarks			Throw an exception if substream hasn't been created
	void RegisterSubStream(unsigned int serviceID, unsigned int priorityLevel, unsigned int sizeBuffer);

	/// Deletes substream
	/// @param serviceID			Service identifier
	/// @remarks			Throw an exception if substream hasn't been deleted
	void UnregisterSubStream(unsigned int serviceID);

	/// Sends data of the substream into the transport stream
	/// @param serviceID			Service identifier
	/// @param data					Pointer to the data
	/// @param sizeData				Size of data to sending
	/// @remarks			Throw an exception if error occurs
	void SendData(unsigned int serviceID, const void* data, unsigned int sizeData);

	/// Receives data of the substream from the transport stream
	/// @param serviceID			Service identifier
	/// @param data					Pointer to the buffer for receiving
	/// @param sizeData				Size of the buffer for receiving
	/// @param getAvailableOnly		Receive only available data from queue
	/// @return				Number of received data
	/// @remarks			Throw an exception if error occurs
	unsigned int ReceiveData(unsigned int serviceID, void* data, unsigned int sizeData, bool getAvailableOnly);

	/// Checks if substream has any data fo receiving
	/// @param serviceID			Service identifier
	/// @return				[true] - if data exists [false] in otherwise
	bool IsDataAvailable(unsigned int serviceID);

	/// Cancels receiving operaton
	/// @param serviceID			Service identifier
	/// @remarks			ReceiveData method will throw an exception if it's waiting for the data
	void CancelReceiveOperation(unsigned int serviceID);

	/// Called if substream was created on the other side
	/// @param serviceID			Service identifier
	virtual int OnSubStreamConnected(unsigned int serviceID);

	/// Called if substream was deleted on the other side
	/// @param serviceID			Service identifier
	virtual int OnSubStreamDisconnected(unsigned int serviceID);

	/// Called if transport stream was broken
	/// @param transportStream		The broken transport stream
	virtual int OnConnectionBroke(boost::shared_ptr<CAbstractStream> transportStream);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////
