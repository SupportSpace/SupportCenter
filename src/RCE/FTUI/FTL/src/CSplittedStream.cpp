//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CSplittedStream.cpp
///
///  Implements CSplittedStream class
///  XXXXXX
///  
///  @author Alexander Novak @date XX.02.2008
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

//#include "..\public\CSplittedStream.h"			//TODO: Fix path
#include <FTUI\FTL\CSplittedStream.h>

// CSplittedStream [BEGIN] ///////////////////////////////////////////////////////////////////////////////

CSplittedStream::CSplittedStream()
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CSplittedStream::~CSplittedStream()
{
TRY_CATCH

CATCH_LOG()
}
//--------------------------------------------------------------------------------------------------------

void CSplittedStream::Send(const void* data, unsigned int sizeData)
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CSplittedStream::Receive(void* data, unsigned int sizeData)
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CSplittedStream::CancelReceiveOperation()
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

unsigned int CSplittedStream::GetStreamID()
{
TRY_CATCH

	return 0;

CATCH_THROW()
}
// CSplittedStream [END] /////////////////////////////////////////////////////////////////////////////////













/*
//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  XXXXXX.cpp
///
///  Implements XXXXXX class
///  XXXXXX
///  
///  @author Alexander Novak @date XX.02.2008
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

// XXXXXX [BEGIN] ////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------------------------
// XXXXXX [END] //////////////////////////////////////////////////////////////////////////////////////////

TRY_CATCH

CATCH_THROW()

TRY_CATCH

CATCH_LOG()
*/