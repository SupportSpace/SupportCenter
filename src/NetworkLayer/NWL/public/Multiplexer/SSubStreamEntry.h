//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  SSubStreamEntry.h
///
///  Declares SSubStreamEntry structure
///  The internal collection for a substream realization
///  
///  @author Alexander Novak @date 28.09.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/Multiplexer/SDatagram.h>
#include <NWL/Multiplexer/CIncomingWaitQueue.h>
#include <NWL/Multiplexer/COutgoingWaitQueue.h>
//========================================================================================================

//State flags for substreams
enum EStateSubStream
{
	E_SS_DISCONNECTED	= 0x1,			//Substream is disconnected
	E_SS_OVERFLOW		= 0x2			//Substream has been overflowed
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////

struct SSubStreamEntry
{
	unsigned int m_serviceID;
	unsigned int m_priorityLevel;
	/// Offset to an incompletely received data
	unsigned int m_paddingOffset;
	volatile unsigned int m_stateFlags;
	
	CIncomingWaitQueue<SDatagram> m_incomingDataQueue;
	COutgoingWaitQueue<SDatagram> m_outgoingDataQueue;
	
	/// Instances new substream's items
	/// @param serviceID			Service identifier
	/// @param priorityLevel		Priority level for the substream
	/// @param stateFlags			Initial substream's state
	/// @param countQueueItem		Number of the elements in queues
	SSubStreamEntry(unsigned int serviceID,
					unsigned int priorityLevel,
					unsigned int stateFlags,
					unsigned int countQueueItem);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////
