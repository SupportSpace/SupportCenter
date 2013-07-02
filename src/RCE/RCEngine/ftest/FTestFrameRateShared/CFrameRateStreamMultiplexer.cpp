
#include "CFrameRateStreamMultiplexer.h"

// CFrameRateStreamMultiplexer [BEGIN] ///////////////////////////////////////////////////////////////////

int CFrameRateStreamMultiplexer::OnSubStreamConnected(unsigned int serviceID)
{
TRY_CATCH

	if ( m_requestedStreamID==serviceID )
		SetEvent(m_SubStreamConnected.get());

CATCH_LOG()

	return 0;
}
//--------------------------------------------------------------------------------------------------------

CFrameRateStreamMultiplexer::CFrameRateStreamMultiplexer(boost::shared_ptr<CAbstractStream> transport)
	:	CStreamMultiplexerBase(transport),
		m_inUse(FALSE),
		m_SubStreamConnected(CreateEvent(NULL,FALSE,FALSE,NULL),CloseHandle),
		m_requestedStreamID(0)
{
TRY_CATCH
CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CFrameRateStreamMultiplexer::~CFrameRateStreamMultiplexer()
{
TRY_CATCH
CATCH_LOG()
}
//--------------------------------------------------------------------------------------------------------

boost::shared_ptr<CAbstractStream> CFrameRateStreamMultiplexer::GetSubStream(	unsigned int serviceID,
																				unsigned int priorityLevel,
																				unsigned int timeout,
																				unsigned int sizeBuffer)
{
TRY_CATCH

	boost::shared_ptr<CAbstractStream> substream;
	
	if ( !InterlockedCompareExchange(&m_inUse,TRUE,FALSE) )
	{
		m_requestedStreamID = serviceID;

		substream = CStreamMultiplexerBase::GetSubStream(	serviceID,	
															priorityLevel,
															sizeBuffer);
		DWORD waitResult = WaitForSingleObject(m_SubStreamConnected.get(),timeout);

		m_requestedStreamID = 0;
		InterlockedExchange(&m_inUse,FALSE);

		if ( waitResult!=WAIT_OBJECT_0 )
			throw MCStreamException(_T("Timeout for stream connection is expired "));
	}
	else
		throw MCStreamException(_T("StreamMultiplexer is connecting another stream, try later."));
	
	return substream;

CATCH_THROW()
}
// CFrameRateStreamMultiplexer [END] /////////////////////////////////////////////////////////////////////
