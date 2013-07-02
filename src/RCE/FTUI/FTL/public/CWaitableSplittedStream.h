//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CWaitableSplittedStream.h
///
///  Declares CWaitableSplittedStream class
///  XXXXXX
///  
///  @author Alexander Novak @date XX.02.2008
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CSplittedStream.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CWaitableSplittedStream
	:	public CSplittedStream
{
	autohandle_t m_fireEvent;
public:
	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	CWaitableSplittedStream(unsigned int streamId,
							boost::shared_ptr<CSubStreamCacheSplitter> splitter,
							autohandle_t fireEvent);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	virtual ~CWaitableSplittedStream();

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	void WaitForConnect(unsigned int timeOut);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////
