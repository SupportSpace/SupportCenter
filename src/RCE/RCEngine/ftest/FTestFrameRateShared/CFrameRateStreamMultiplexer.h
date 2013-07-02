//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CFrameRateStreamMultiplexer.h
///
///  CFrameRateStreamMultiplexer realization for functional Frame Rate test
///
///  @author Alexander Novak @date 11.10.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/Multiplexer/CStreamMultiplexerBase.h>

//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CFrameRateStreamMultiplexer
	:	public CStreamMultiplexerBase
{
	volatile LONG m_inUse;
	unsigned int m_requestedStreamID;
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > m_SubStreamConnected;
	virtual int OnSubStreamConnected(unsigned int serviceID);
public:
	
	/// Creates stream multiplexer object
	/// @param transport		Stream for data transport
	CFrameRateStreamMultiplexer(boost::shared_ptr<CAbstractStream> transport);
	~CFrameRateStreamMultiplexer();
	
	/// Returns already connected substream
	/// @param serviceID			Service identifier
	/// @param priorityLevel		Priority level
	/// @param timeout				Timeout for substream creation
	/// @param sizeBuffer			Buffer size for a substream
	/// @return				Pointer to the substream
	/// @remarks			Throw an exception if error occurs
	boost::shared_ptr<CAbstractStream> GetSubStream(unsigned int serviceID,
													unsigned int priorityLevel,
													unsigned int timeout,
													unsigned int sizeBuffer = SUBSTREAM_BUFFERSIZE);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////
