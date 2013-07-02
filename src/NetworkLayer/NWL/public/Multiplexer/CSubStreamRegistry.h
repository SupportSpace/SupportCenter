//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CSubStreamRegistry.h
///
///  Declares a collection of the substreams their states and priority levels
///  
///  @author Alexander Novak @date 28.09.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/Multiplexer/SSubStreamEntry.h>
#include <boost/shared_ptr.hpp>
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CSubStreamRegistry
{
	unsigned int m_minPriorityLevel;
	std::vector< boost::shared_ptr<SSubStreamEntry> > m_vecStreams;
public:
	CSubStreamRegistry();

	/// Create new substream
	/// @param serviceID			Service identifier
	/// @param priorityLevel		Priority level of the substream
	/// @param stateFlags			Initial state of flags
	/// @param countQueueItem		Number items in substream's buffer
	/// @remarks			Throw an exception if error occurs
	void CreateSubStream(	unsigned int serviceID,
							unsigned int priorityLevel,
							unsigned int stateFlags,
							unsigned int countQueueItem);

	/// Delete substream
	/// @param serviceID			Service identifier
	void DeleteSubStream(unsigned int serviceID);

	/// Returns the substream's entry by service identifier
	/// @param serviceID			Service identifier
	/// @return				Pointer to entry of the substream
	/// @remarks			Throw an exception if error occurs
	boost::shared_ptr<SSubStreamEntry> GetSubStreamEntryByServiceID(unsigned int serviceID);

	/// Returns the substream's entry by index
	/// @param subStreamIndex		Index of the substream in collection
	/// @return				Pointer to entry of the substream
	/// @remarks			Throw an exception if error occurs
	boost::shared_ptr<SSubStreamEntry> GetSubStreamEntryByIndex(unsigned int subStreamIndex);

	/// Returns number of the substreams
	/// @return				Count of the substreams in collection
	unsigned int CountSubStream();

	/// Gets minimal priority level of the substreams in collection
	/// @return				Minimal priority level of the substreams
	unsigned int GetMinPriorityLevel();
};
//--------------------------------------------------------------------------------------------------------

inline boost::shared_ptr<SSubStreamEntry> CSubStreamRegistry::GetSubStreamEntryByIndex(unsigned int subStreamIndex)
{
	return m_vecStreams[subStreamIndex];
}
//--------------------------------------------------------------------------------------------------------

inline unsigned int CSubStreamRegistry::CountSubStream()
{
	return static_cast<unsigned int>(m_vecStreams.size());
}
//--------------------------------------------------------------------------------------------------------

inline unsigned int CSubStreamRegistry::GetMinPriorityLevel()
{
	return m_minPriorityLevel;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
