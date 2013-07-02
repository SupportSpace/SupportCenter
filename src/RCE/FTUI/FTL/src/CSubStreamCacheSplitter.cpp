//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CSubStreamCacheSplitter.cpp
///
///  Implements CSubStreamCacheSplitter class
///  XXXXXX
///  
///  @author Alexander Novak @date XX.02.2008
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

//#include "..\public\CSubStreamCacheSplitter.h"			//TODO: Fix path
//#include "..\public\CWaitableSplittedStream.h"			//TODO: Fix path
#include <FTUI\FTL\CSubStreamCacheSplitter.h>
#include <FTUI\FTL\CWaitableSplittedStream.h>

// CSubStreamCacheSplitter [BEGIN] ///////////////////////////////////////////////////////////////////////

void CSubStreamCacheSplitter::Execute(void* Params)
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CSubStreamCacheSplitter::CSubStreamCacheSplitter(boost::shared_ptr<CAbstractStream> carrierStream)
	:	CSubStreamDispatcher(carrierStream)
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CSubStreamCacheSplitter::~CSubStreamCacheSplitter()
{
TRY_CATCH

CATCH_LOG()
}
//--------------------------------------------------------------------------------------------------------

boost::shared_ptr<CWaitableSplittedStream> CSubStreamCacheSplitter::CreateStream(	unsigned int streamId,
																					unsigned int priorityLevel,
																					unsigned int sizeBuffer)
{
TRY_CATCH

	boost::shared_ptr<CWaitableSplittedStream> wss;
	return wss;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

boost::shared_ptr<CWaitableSplittedStream> CSubStreamCacheSplitter::GetSuspendedStream(unsigned int streamId)
{
TRY_CATCH

	boost::shared_ptr<CWaitableSplittedStream> wss;
	return wss;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CSubStreamCacheSplitter::ReleaseStream(unsigned int streamId)
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CSubStreamCacheSplitter::KillStream(unsigned int streamId)
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CSubStreamCacheSplitter::SetStreamError(unsigned int streamId, DWORD internalError)
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

DWORD CSubStreamCacheSplitter::GetStreamError(unsigned int streamId)
{
TRY_CATCH

	return  NULL;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CSubStreamCacheSplitter::WakeUpStream(unsigned int streamId, DWORD internalError)
{
TRY_CATCH

CATCH_THROW()
}
// CSubStreamCacheSplitter [END] /////////////////////////////////////////////////////////////////////////
