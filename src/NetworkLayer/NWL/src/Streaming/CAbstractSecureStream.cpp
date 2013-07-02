/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CAbstractSecureStream.cpp
///
///  Implements CAbstractSecureStream class, interface for secure connection
///
///  @author Dmitry Netrebenko @date 17.09.2007
///
////////////////////////////////////////////////////////////////////////

#include <NWL/Streaming/CAbstractSecureStream.h>
#include <AidLib/CException/CException.h>
#include <AidLib/Logging/cLog.h>

CAbstractSecureStream::CAbstractSecureStream()
{
TRY_CATCH
CATCH_THROW()
}

CAbstractSecureStream::~CAbstractSecureStream()
{
TRY_CATCH
CATCH_LOG()
}
