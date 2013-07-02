//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CStreamMultiplexerBase.cpp
///
///  Implements a CStreamMultiplexerBase class
///  Creates substreams and provides events for substream's service.
///  As a transport stream uses the CAbstractStream
///  
///  @author Alexander Novak @date 28.09.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <NWL/Multiplexer/CStreamMultiplexerBase.h>
#include <NWL/Multiplexer/CSubStream.h>

// CStreamMultiplexerBase [BEGIN] ////////////////////////////////////////////////////////////////////////

CStreamMultiplexerBase::CStreamMultiplexerBase(boost::shared_ptr<CAbstractStream> transportStream)
	:	CSubStreamDispatcher(transportStream)
{
TRY_CATCH

	m_dispatcher.reset();

CATCH_THROW();
}
//--------------------------------------------------------------------------------------------------------

CStreamMultiplexerBase::~CStreamMultiplexerBase()
{
TRY_CATCH
CATCH_LOG();
}
//--------------------------------------------------------------------------------------------------------

boost::shared_ptr<CAbstractStream> CStreamMultiplexerBase::GetSubStream(unsigned int serviceID,
																		unsigned int priorityLevel,
																		unsigned int sizeBuffer)
{
TRY_CATCH

	if (!m_dispatcher.lock())
		throw MCStreamException(_T("StreamMultiplexer hasn't been initialized. Create StreamMultiplexer through GetInstance()"));

	RegisterSubStream(serviceID,priorityLevel,sizeBuffer);

	boost::shared_ptr<CSubStream> substream(new CSubStream());

	substream->m_serviceID	= serviceID;
	substream->m_dispatcher	= m_dispatcher.lock();

	return substream;

CATCH_THROW();
}
//--------------------------------------------------------------------------------------------------------

int CStreamMultiplexerBase::OnSubStreamConnected(unsigned int serviceID)
{
	return 0;
}
//--------------------------------------------------------------------------------------------------------

int CStreamMultiplexerBase::OnSubStreamDisconnected(unsigned int serviceID)
{
	return 0;
}
//--------------------------------------------------------------------------------------------------------

int CStreamMultiplexerBase::OnConnectionBroke(boost::shared_ptr<CAbstractStream> transportStream)
{
	return 0;
}
// CStreamMultiplexerBase [END] //////////////////////////////////////////////////////////////////////////
