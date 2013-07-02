//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CWaitableSplittedStream.cpp
///
///  Implements CWaitableSplittedStream class
///  XXXXXX
///  
///  @author Alexander Novak @date XX.02.2008
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

//#include "..\public\CWaitableSplittedStream.h"		//TODO: Fix path
#include <FTUI\FTL\CWaitableSplittedStream.h>

// CWaitableSplittedStream [BEGIN] ///////////////////////////////////////////////////////////////////////

CWaitableSplittedStream::CWaitableSplittedStream(	unsigned int streamId,
													boost::shared_ptr<CSubStreamCacheSplitter> splitter,
													autohandle_t fireEvent)
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CWaitableSplittedStream::~CWaitableSplittedStream()
{
TRY_CATCH

CATCH_LOG()
}
//--------------------------------------------------------------------------------------------------------

void CWaitableSplittedStream::WaitForConnect(unsigned int timeOut)
{
TRY_CATCH

CATCH_THROW()
}
// CWaitableSplittedStream [END] /////////////////////////////////////////////////////////////////////////




