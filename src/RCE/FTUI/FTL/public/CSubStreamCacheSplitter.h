//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CSubStreamCacheSplitter.h
///
///  Declares CSubStreamCacheSplitter class
///  XXXXXX
///  
///  @author Alexander Novak @date XX.02.2008
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "SCacheStreamItem.h"
#include <AidLib\CThread\CThread.h>
#include <NWL\Multiplexer\CSubStreamDispatcher.h>
#include <AidLib\CCritSection\CCritSectionObject.h>
#include <AidLib\CException\CException.h>
#include <list>
//========================================================================================================

class CWaitableSplittedStream;
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CSubStreamCacheSplitter
	:	public CSubStreamDispatcher,
		CThread
{
	CCritSectionSimpleObject m_guardCache;
	std::list<SCacheStreamItem> m_subStreamCache;

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	virtual void Execute(void* Params);

public:
	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	CSubStreamCacheSplitter(boost::shared_ptr<CAbstractStream> carrierStream);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	virtual ~CSubStreamCacheSplitter();

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	boost::shared_ptr<CWaitableSplittedStream> CreateStream(unsigned int streamId,
															unsigned int priorityLevel,
															unsigned int sizeBuffer);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	boost::shared_ptr<CWaitableSplittedStream> GetSuspendedStream(unsigned int streamId = 0);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	void ReleaseStream(unsigned int streamId);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	void KillStream(unsigned int streamId);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	void SetStreamError(unsigned int streamId, DWORD internalError);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	DWORD GetStreamError(unsigned int streamId);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	void WakeUpStream(unsigned int streamId, DWORD internalError);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////
