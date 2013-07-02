//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CSubStreamRegistry.cpp
///
///  Implements a collection of the substreams their states and priority levels
///  
///  @author Alexander Novak @date 01.10.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <NWL/Multiplexer/CSubStreamRegistry.h>

// CSubStreamRegistry [BEGIN] ////////////////////////////////////////////////////////////////////////////

CSubStreamRegistry::CSubStreamRegistry()
	:	m_minPriorityLevel(1)
{
}
//--------------------------------------------------------------------------------------------------------

void CSubStreamRegistry::CreateSubStream(	unsigned int serviceID,
											unsigned int priorityLevel,
											unsigned int stateFlags,
											unsigned int countQueueItem)
{
TRY_CATCH

	for (unsigned int i=0; i < m_vecStreams.size(); i++)
		if ( m_vecStreams[i]->m_serviceID == serviceID )
			throw MCStreamException(_T("Can't create substream. Substream already exists by this service identifier"));

	boost::shared_ptr<SSubStreamEntry> newSubStream( new SSubStreamEntry(serviceID,priorityLevel,stateFlags,countQueueItem) );

	m_vecStreams.push_back(newSubStream);

	if ( priorityLevel < m_minPriorityLevel)
		m_minPriorityLevel = priorityLevel;
	
CATCH_THROW();
}
//--------------------------------------------------------------------------------------------------------

void CSubStreamRegistry::DeleteSubStream(unsigned int serviceID)
{
TRY_CATCH

	std::vector< boost::shared_ptr<SSubStreamEntry> >::iterator i=m_vecStreams.begin();

	while ( i < m_vecStreams.end() && (*i)->m_serviceID != serviceID )
		i++;

	if ( i != m_vecStreams.end() )
	{
		unsigned int deletedPriorityLevel = (*i)->m_priorityLevel;

		m_vecStreams.erase(i);

		//Calculate new minimal priority level
		if ( deletedPriorityLevel == m_minPriorityLevel )
		{
			m_minPriorityLevel = -1;

			for ( unsigned int i=0; i < m_vecStreams.size(); i++ )
				if ( m_vecStreams[i]->m_priorityLevel < m_minPriorityLevel )
					m_minPriorityLevel = m_vecStreams[i]->m_priorityLevel;
		}
	}
	else
		throw MCStreamException(_T("Can't delete substream. Substream not exists by requested service identifier"));
	
CATCH_THROW();
}
//--------------------------------------------------------------------------------------------------------

boost::shared_ptr<SSubStreamEntry> CSubStreamRegistry::GetSubStreamEntryByServiceID(unsigned int serviceID)
{
TRY_CATCH

	for ( unsigned int i=0; i < m_vecStreams.size(); i++ )
		if ( m_vecStreams[i]->m_serviceID == serviceID )
			return m_vecStreams[i];

	throw MCStreamException(_T("SubStream not exists by requested service identifier"));

CATCH_THROW();
}
// CSubStreamRegistry [END] //////////////////////////////////////////////////////////////////////////////
