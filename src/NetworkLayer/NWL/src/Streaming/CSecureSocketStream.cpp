/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSecureSocketStream.cpp
///
///  Implements CSecureSocketStream class, socket stream with abstract
///    security connection
///
///  @author Dmitry Netrebenko @date 17.09.2007
///
////////////////////////////////////////////////////////////////////////


#include <NWL/Streaming/CSecureSocketStream.h>
#include <AidLib/CException/CException.h>
#include <AidLib/Logging/cLog.h>

CSecureSocketStream::CSecureSocketStream()
	:	CSocketStream()
	,	CAbstractSecureStream()

{
TRY_CATCH
CATCH_THROW()
}

CSecureSocketStream::~CSecureSocketStream()
{
TRY_CATCH
CATCH_LOG()
}
