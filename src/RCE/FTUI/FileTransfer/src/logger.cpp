/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  logger.cpp
///
///  Provides logging facilities.
///
///  @author Pavel Ivashkov @date Friday, August 11, 2006
///  
/////////////////////////////////////////////////////////////////////////
#include "logger.h"

#ifdef _DEBUG

#pragma warning( disable: 4996 )//<func> was declared deprecated

void DebugOnlyLog( LPCTSTR message, ... )
{
	va_list args;
	va_start( args, message );
	TCHAR buf[4096];
	_vsntprintf( buf, sizeof(buf)/sizeof(buf[0]), message, args );
	DebugPrint( "FileTransfer [P%i T%i] %s\n", GetCurrentProcessId(), GetCurrentThreadId(), buf );
}

void DebugPrint( LPCTSTR message, ... )
{
	va_list args;
	va_start( args, message );
	DebugPrintV( message, args );
}

void DebugPrintV( LPCTSTR message, va_list args )
{
	TCHAR buf[4096];
	_vsntprintf( buf, sizeof(buf)/sizeof(buf[0]), message, args );
	OutputDebugString( buf );
}

#endif // _DEBUG
