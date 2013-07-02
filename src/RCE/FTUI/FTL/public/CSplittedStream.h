//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CSplittedStream.h
///
///  Declares CSplittedStream class
///  XXXXXX
///  
///  @author Alexander Novak @date XX.02.2008
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CSubStreamCacheSplitter.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CSplittedStream
{
protected:
	unsigned int m_streamId;
	boost::shared_ptr<CSubStreamCacheSplitter> m_splitter;

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	CSplittedStream();
public:
	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	virtual ~CSplittedStream();

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	void Send(const void* data, unsigned int sizeData);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	void Receive(void* data, unsigned int sizeData);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	void CancelReceiveOperation();

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	unsigned int GetStreamID();
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////

















/*
//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  XXXXXX.h
///
///  Declares XXXXXX class
///  XXXXXX
///  
///  @author Alexander Novak @date XX.02.2008
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

//========================================================================================================

//////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
*/