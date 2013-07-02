//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CSubStreamDispatcher.cpp
///
///  Implements a CSubStreamDispatcher class
///  Creates and frees substreams. Dispatches substreams by their priority level.
///  Provides events for substream's service.
///  As a transport stream uses the CAbstractStream through CTransportAdapter.
///  
///  @author Alexander Novak @date 02.10.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <NWL/Multiplexer/CSubStreamDispatcher.h>
#include <AidLib/CThread/CThreadLs.h>
//========================================================================================================

#define DISPATCHER_SERVICEID							0x0
#define DISPATCHER_MAXNUMBER_SERVICECMD					64

#define DISPATCHER_TIMEFORSHUTDOWN						10000

#define DISPATCHER_CMD_SUBSTREAM_OVERFLOW				0x1
#define DISPATCHER_CMD_SUBSTREAM_NOTFOUND				0x2
#define DISPATCHER_CMD_SUBSTREAM_CONNECT				0x3
#define DISPATCHER_CMD_SUBSTREAM_CONNECTED				0x4
#define DISPATCHER_CMD_SUBSTREAM_DISCONNECT				0x5
//========================================================================================================

// CSubStreamDispatcher [BEGIN] //////////////////////////////////////////////////////////////////////////

DWORD WINAPI CSubStreamDispatcher::ThreadReceiveData(LPVOID lpParameter)
{
TRY_CATCH
	SET_THREAD_LS;

	CSubStreamDispatcher* dispatcher=static_cast<CSubStreamDispatcher*>(lpParameter);
	
	for (;;)
	{
		SDatagram datagram;
		try
		{
			dispatcher->m_transportAdapter.ReceiveDatagram(&datagram);
		}
		catch (CStreamException& exception)
		{
			Log.Add(exception);

			//Lock the other attempts of data transfer
			if ( !InterlockedCompareExchange(&dispatcher->m_transportBroken,TRUE,FALSE) )
				 dispatcher->OnConnectionBroke(dispatcher->m_transportAdapter.GetTransportStream());

			//Sets substreams state to disconnected
			CCritSection regGuard(&dispatcher->m_csRegistryGuard);

			for (unsigned int i=0; i < dispatcher->m_subStreamRegistry.CountSubStream(); i++)
			{
				boost::shared_ptr<SSubStreamEntry> subStream = dispatcher->m_subStreamRegistry.GetSubStreamEntryByIndex(i);	

				subStream->m_stateFlags |= E_SS_DISCONNECTED;
				subStream->m_incomingDataQueue.DropIncomingWaiting();
			}
			// Dispatcher is being terminated or transport stream was broken - terminate thread	 
			return 0;
		}
		// Check if it is service datagram and process it
		if ( datagram.m_serviceID == DISPATCHER_SERVICEID )
		{
			unsigned int* datagramData = reinterpret_cast<unsigned int*>(datagram.m_data);
			unsigned int command	= datagramData[0];
			unsigned int serviceID	= datagramData[1];

			switch ( command )
			{
				case DISPATCHER_CMD_SUBSTREAM_OVERFLOW:
				{
					CCritSection regGuard(&dispatcher->m_csRegistryGuard);

					TRY_CATCH

						boost::shared_ptr<SSubStreamEntry> subStream = dispatcher->m_subStreamRegistry.GetSubStreamEntryByServiceID(serviceID);
						subStream->m_stateFlags |= E_SS_OVERFLOW;
						subStream->m_outgoingDataQueue.Clear();

					CATCH_LOG();

					break;
				}
				case DISPATCHER_CMD_SUBSTREAM_NOTFOUND:
				case DISPATCHER_CMD_SUBSTREAM_DISCONNECT:
				{
					dispatcher->OnSubStreamDisconnected(serviceID);

					CCritSection regGuard(&dispatcher->m_csRegistryGuard);

					TRY_CATCH

						boost::shared_ptr<SSubStreamEntry> subStream = dispatcher->m_subStreamRegistry.GetSubStreamEntryByServiceID(serviceID);
						subStream->m_stateFlags |= E_SS_DISCONNECTED;
						subStream->m_incomingDataQueue.DropIncomingWaiting();
						subStream->m_outgoingDataQueue.Clear();

					CATCH_LOG();

					break;
				}
				case DISPATCHER_CMD_SUBSTREAM_CONNECT:
				case DISPATCHER_CMD_SUBSTREAM_CONNECTED:
				{
					boost::shared_ptr<SSubStreamEntry> subStream;

					CCritSection regGuard(&dispatcher->m_csRegistryGuard);

					TRY_CATCH

						subStream = dispatcher->m_subStreamRegistry.GetSubStreamEntryByServiceID(serviceID);

						if ( subStream->m_stateFlags )
							subStream->m_stateFlags = 0;
						else
							subStream.reset(); // We've already been at here. Prevents twice calling OnSubStreamConnected

					CATCH_LOG();

					regGuard.Unlock();

					if ( subStream.get() )
					{
						if ( command == DISPATCHER_CMD_SUBSTREAM_CONNECT)
						{
							dispatcher->SendServiceCmd( &SServiceCmd(DISPATCHER_CMD_SUBSTREAM_CONNECTED,serviceID) );
							SetEvent(dispatcher->m_eventSendDatagram.get());		//Raise event for ThreadSendData
						}
						dispatcher->OnSubStreamConnected( serviceID );
					}
					break;
				}
			}// switch ( command )
		}
		else
		{
			// Push datagram into substream queue
			boost::shared_ptr<SSubStreamEntry> subStream;

			CCritSection regGuard(&dispatcher->m_csRegistryGuard);

			TRY_CATCH

				subStream = dispatcher->m_subStreamRegistry.GetSubStreamEntryByServiceID(datagram.m_serviceID);

				if ( subStream->m_stateFlags )
				{
					dispatcher->SendServiceCmd( &SServiceCmd(DISPATCHER_CMD_SUBSTREAM_DISCONNECT,datagram.m_serviceID) );
					SetEvent(dispatcher->m_eventSendDatagram.get());				//Raise event for ThreadSendData
				}
				else if (!subStream->m_incomingDataQueue.Push(&datagram) )
				{
					// Substream's queue is overflow. Notify other side and set state flag
					dispatcher->SendServiceCmd( &SServiceCmd(DISPATCHER_CMD_SUBSTREAM_OVERFLOW,datagram.m_serviceID) );
					SetEvent(dispatcher->m_eventSendDatagram.get());				//Raise event for ThreadSendData
					subStream->m_stateFlags |= E_SS_OVERFLOW;
					subStream->m_incomingDataQueue.DropIncomingWaiting();
				}

			CATCH_LOG();

			regGuard.Unlock();

			if ( !subStream.get() )
			{
				dispatcher->SendServiceCmd( &SServiceCmd(DISPATCHER_CMD_SUBSTREAM_NOTFOUND,datagram.m_serviceID) );
				SetEvent(dispatcher->m_eventSendDatagram.get());					//Raise event for ThreadSendData
			}
		}
	}// for (;;)

CATCH_LOG();

	return 0;
}
//--------------------------------------------------------------------------------------------------------

DWORD WINAPI CSubStreamDispatcher::ThreadSendData(LPVOID lpParameter)
{
TRY_CATCH
	SET_THREAD_LS;

	CSubStreamDispatcher* dispatcher=static_cast<CSubStreamDispatcher*>(lpParameter);

	for (;;)
	{
		if ( WaitForSingleObject(dispatcher->m_eventSendDatagram.get(),INFINITE) != WAIT_OBJECT_0 )
			throw MCStreamException_Win(_T("Wait operation failed"),GetLastError());

		bool queuesNotEmpty = true;
		while ( queuesNotEmpty )
		{
			if ( dispatcher->m_transportBroken )
				// Dispatcher is being terminated or transport stream was broken - terminate thread 
				return 0;

			// Processing service commands they have the highest priority
			while ( const SServiceCmd* serviceCmd = dispatcher->m_serviceStream.Top() )
			{
				SDatagram datagram;
				unsigned int* datagramData	= reinterpret_cast<unsigned int*>(datagram.m_data);
				datagram.m_serviceID		= DISPATCHER_SERVICEID;
				datagram.m_dataSize			= sizeof(unsigned int)*2;
				datagramData[0]				= serviceCmd->m_command;
				datagramData[1]				= serviceCmd->m_serviceID;
				try
				{
					dispatcher->m_transportAdapter.SendDatagram(&datagram);
					dispatcher->m_serviceStream.Pop();
				}
				catch (CStreamException& exception)
				{
					Log.Add(exception);

					// Lock the other attempts of data transfer
					if ( !InterlockedCompareExchange(&dispatcher->m_transportBroken,TRUE,FALSE) )
						 dispatcher->OnConnectionBroke(dispatcher->m_transportAdapter.GetTransportStream());
						
					dispatcher->m_serviceStream.DropOutgoingWaiting();
					break;
				}
			}
			// Processing substreams
			CCritSection regGuard(&dispatcher->m_csRegistryGuard);

			unsigned int i=0;
			while ( i < dispatcher->m_subStreamRegistry.CountSubStream() )
			{
				boost::shared_ptr<SSubStreamEntry> substream = dispatcher->m_subStreamRegistry.GetSubStreamEntryByIndex(i);

				unsigned int countToSend = substream->m_priorityLevel/dispatcher->m_subStreamRegistry.GetMinPriorityLevel();

				regGuard.Unlock();

				const SDatagram* datagram;
				while ( countToSend-- && (datagram = substream->m_outgoingDataQueue.Top()) )
				{
					try
					{
						dispatcher->m_transportAdapter.SendDatagram(datagram);
						substream->m_outgoingDataQueue.Pop();
					}
					catch (CStreamException& exception)
					{
						Log.Add(exception);

						// Lock the other attempts of data transfer
						if ( !InterlockedCompareExchange(&dispatcher->m_transportBroken,TRUE,FALSE) )
							 dispatcher->OnConnectionBroke(dispatcher->m_transportAdapter.GetTransportStream());

						// Sets substreams state to disconnected
						regGuard.Lock();

						for (unsigned int i=0; i < dispatcher->m_subStreamRegistry.CountSubStream(); i++)
						{
							boost::shared_ptr<SSubStreamEntry> subStream = dispatcher->m_subStreamRegistry.GetSubStreamEntryByIndex(i);	
							
							subStream->m_stateFlags |= E_SS_DISCONNECTED;
							subStream->m_outgoingDataQueue.DropOutgoingWaiting();
						}
						
						regGuard.Unlock();
						// Transport stream was broken - terminate thread
						return 0;
					}
				}// while ( countToSend-- && (datagram = substream->m_outgoingDataQueue.Top()) )
				i++;

				regGuard.Lock();

			}// while (i < dispatcher->m_subStreamRegistry.CountSubStream())

			// Check if queues are empty
			queuesNotEmpty = false;

			for (unsigned int i=0; i < dispatcher->m_subStreamRegistry.CountSubStream(); i++)
			{
				boost::shared_ptr<SSubStreamEntry> subStream = dispatcher->m_subStreamRegistry.GetSubStreamEntryByIndex(i);	
				if ( !subStream->m_outgoingDataQueue.IsEmpty() )
				{
					queuesNotEmpty = true;
					break;
				}
			}
			regGuard.Unlock();

			if ( !queuesNotEmpty )
				queuesNotEmpty = !dispatcher->m_serviceStream.IsEmpty();		// Check if service stream are empty
		}// while ( queuesNotEmpty )
	}// for (;;)

CATCH_LOG();

	return 0;
}
//--------------------------------------------------------------------------------------------------------

void CSubStreamDispatcher::SendServiceCmd(const SServiceCmd* serviceCmd)
{
	if ( !m_transportBroken )
		m_serviceStream.Push(serviceCmd);
}
//--------------------------------------------------------------------------------------------------------

CSubStreamDispatcher::CSubStreamDispatcher(boost::shared_ptr<CAbstractStream> transportStream)
	:	m_transportAdapter(transportStream),
		m_serviceStream(DISPATCHER_MAXNUMBER_SERVICECMD),
		m_transportBroken(FALSE)
{
TRY_CATCH

	m_eventSendDatagram.reset(CreateEvent(NULL,FALSE,FALSE,NULL), CloseHandle);

	m_threadReceiveData.reset(CreateThread(NULL,0,ThreadSendData,this,0,NULL), CloseHandle);
	m_threadSendData.reset(CreateThread(NULL,0,ThreadReceiveData,this,0,NULL), CloseHandle);

	Log.Add(_MESSAGE_,_T("CSubStreamDispatcher has been created"));
CATCH_THROW();
}
//--------------------------------------------------------------------------------------------------------

CSubStreamDispatcher::~CSubStreamDispatcher()
{
TRY_CATCH

	InterlockedExchange(&m_transportBroken,TRUE);
	m_transportAdapter.DropTransportReceive();
	SetEvent(m_eventSendDatagram.get());

	HANDLE arrHandle[2]={m_threadReceiveData.get(),m_threadSendData.get()};

	if ( WaitForMultipleObjects(2,arrHandle,TRUE,DISPATCHER_TIMEFORSHUTDOWN)!=WAIT_OBJECT_0 )
	{
		Log.Add(_MESSAGE_,_T("Threads have been terminated forcibly"));
		
		TerminateThread(m_threadReceiveData.get(),0);
		TerminateThread(m_threadSendData.get(),0);
	}

	Log.Add(_MESSAGE_,_T("CSubStreamDispatcher was deleted"));
CATCH_LOG();
}
//--------------------------------------------------------------------------------------------------------

void CSubStreamDispatcher::RegisterSubStream(unsigned int serviceID, unsigned int priorityLevel, unsigned int sizeBuffer)
{
TRY_CATCH

	if ( (serviceID==DISPATCHER_SERVICEID) || !priorityLevel )
		throw MCStreamException(_T("Invalid input parameters"));

	CCritSection regGuard(&m_csRegistryGuard);

	m_subStreamRegistry.CreateSubStream(serviceID,priorityLevel,E_SS_DISCONNECTED,sizeBuffer/sizeof(SDatagram));

	//Send notification to the other side
	SendServiceCmd( &SServiceCmd(DISPATCHER_CMD_SUBSTREAM_CONNECT,serviceID) );
	SetEvent(m_eventSendDatagram.get());					//Raise event for ThreadSendData

	Log.Add(_MESSAGE_,_T("SubStream has been created. ServiceID is %u"),serviceID);

CATCH_THROW();
}
//--------------------------------------------------------------------------------------------------------

void CSubStreamDispatcher::UnregisterSubStream(unsigned int serviceID)
{
TRY_CATCH

	CCritSection regGuard(&m_csRegistryGuard);

	boost::shared_ptr<SSubStreamEntry> subStream = m_subStreamRegistry.GetSubStreamEntryByServiceID(serviceID);

	m_subStreamRegistry.DeleteSubStream(serviceID);			//Remove from dispatching

	if ( !subStream->m_stateFlags )
	{
		//If SubStream was connected then send notification to the other side
		SendServiceCmd( &SServiceCmd(DISPATCHER_CMD_SUBSTREAM_DISCONNECT,serviceID) );
		SetEvent(m_eventSendDatagram.get());				//Raise event for ThreadSendData
	}
	subStream->m_stateFlags |= E_SS_DISCONNECTED;

	subStream->m_incomingDataQueue.DropIncomingWaiting();	//Drop waiting on receiving if exists
	subStream->m_outgoingDataQueue.DropOutgoingWaiting();	//Drop waiting on sending if exists

	Log.Add(_MESSAGE_,_T("SubStream has been deleted. ServiceID is %u"),serviceID);

CATCH_THROW();
}
//--------------------------------------------------------------------------------------------------------

void CSubStreamDispatcher::SendData(unsigned int serviceID, const void* data, unsigned int sizeData)
{
TRY_CATCH

	CCritSection regGuard(&m_csRegistryGuard);

	boost::shared_ptr<SSubStreamEntry> subStream = m_subStreamRegistry.GetSubStreamEntryByServiceID(serviceID);

	regGuard.Unlock();		// To release substream's registry as quick as possible

	const BYTE* ptrData = static_cast<const BYTE*>(data);

	SDatagram datagram;
	datagram.m_serviceID = serviceID;

	unsigned int totalSend = 0;

	while ( totalSend < sizeData )
	{
		if ( subStream->m_stateFlags & E_SS_OVERFLOW )
			throw MCStreamException(_T("SubStream has been disconnected on overflow"));
		else if ( subStream->m_stateFlags )
			throw MCStreamException(_T("SubStream is disconnected"));

		unsigned int sz_min = min(sizeData - totalSend, MAX_DATAGRAM_SIZE);
		datagram.m_dataSize = sz_min;

		CopyMemory(datagram.m_data, ptrData + totalSend, sz_min);

		if ( !subStream->m_outgoingDataQueue.Push(&datagram) )
			throw MCStreamException(_T("Send operation has been canceled"));

		SetEvent(m_eventSendDatagram.get());			//Raise event for ThreadSendData

		totalSend += sz_min;
	}

CATCH_THROW();
}
//--------------------------------------------------------------------------------------------------------

unsigned int CSubStreamDispatcher::ReceiveData(unsigned int serviceID, void* data, unsigned int sizeData, bool getAvailableOnly)
{
TRY_CATCH

	CCritSection regGuard(&m_csRegistryGuard);

	boost::shared_ptr<SSubStreamEntry> subStream = m_subStreamRegistry.GetSubStreamEntryByServiceID(serviceID);

	regGuard.Unlock();		// To release substream's registry as quick as possible

	BYTE* ptrData = static_cast<BYTE*>(data);
	unsigned int totalReceive = 0;

	while ( ( !getAvailableOnly || !subStream->m_incomingDataQueue.IsEmpty() ) && sizeData - totalReceive )
	{
		if ( subStream->m_stateFlags & E_SS_OVERFLOW )
			throw MCStreamException(_T("SubStream has been disconnected on overflow"));
		else if ( subStream->m_stateFlags )
			throw MCStreamException(_T("SubStream is disconnected"));

		const SDatagram* datagram = subStream->m_incomingDataQueue.Top();
		if ( !datagram || subStream->m_stateFlags )
			throw MCStreamException(_T("Receive operation has been canceled"));

		unsigned int sz_min = min(sizeData - totalReceive, datagram->m_dataSize - subStream->m_paddingOffset);

		CopyMemory(ptrData + totalReceive, datagram->m_data + subStream->m_paddingOffset, sz_min);

		subStream->m_paddingOffset += sz_min;
		totalReceive += sz_min;

		if ( datagram->m_dataSize == subStream->m_paddingOffset )
		{
			subStream->m_paddingOffset = 0;

			if ( !subStream->m_incomingDataQueue.Pop() )
				throw MCStreamException(_T("Receive operation has been canceled"));
		}
	}

	if (getAvailableOnly && !totalReceive)
	{
		if ( subStream->m_stateFlags & E_SS_OVERFLOW )
			throw MCStreamException(_T("SubStream has been disconnected on overflow"));
		else if ( subStream->m_stateFlags )
			throw MCStreamException(_T("SubStream is disconnected"));
		if ( subStream->m_incomingDataQueue.WaitDropped())
			throw MCStreamException(_T("Receive operation has been canceled"));
	}

	return totalReceive;

CATCH_THROW();
}
//--------------------------------------------------------------------------------------------------------

bool CSubStreamDispatcher::IsDataAvailable(unsigned int serviceID)
{
TRY_CATCH

	CCritSection regGuard(&m_csRegistryGuard);

	boost::shared_ptr<SSubStreamEntry> subStream = m_subStreamRegistry.GetSubStreamEntryByServiceID(serviceID);

	regGuard.Unlock();		// To release substream's registry as quick as possible

	return !subStream->m_incomingDataQueue.IsEmpty();

CATCH_THROW();
}
//--------------------------------------------------------------------------------------------------------

void CSubStreamDispatcher::CancelReceiveOperation(unsigned int serviceID)
{
TRY_CATCH

	CCritSection regGuard(&m_csRegistryGuard);

	boost::shared_ptr<SSubStreamEntry> subStream = m_subStreamRegistry.GetSubStreamEntryByServiceID(serviceID);

	regGuard.Unlock();		// To release substream's registry as quick as possible

	subStream->m_incomingDataQueue.DropIncomingWaiting();
	subStream->m_outgoingDataQueue.DropOutgoingWaiting();

CATCH_THROW();
}
//--------------------------------------------------------------------------------------------------------

int CSubStreamDispatcher::OnSubStreamConnected(unsigned int serviceID)
{
	return 0;
}
//--------------------------------------------------------------------------------------------------------

int CSubStreamDispatcher::OnSubStreamDisconnected(unsigned int serviceID)
{
	return 0;
}
//--------------------------------------------------------------------------------------------------------

int CSubStreamDispatcher::OnConnectionBroke(boost::shared_ptr<CAbstractStream> transportStream)
{
	return 0;
}
// CSubStreamDispatcher [END] ////////////////////////////////////////////////////////////////////////////
