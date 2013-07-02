/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  logger.h
///
///  Provides logging facilities.
///
///  @author Pavel Ivashkov @date Friday, August 11, 2006
///  
/////////////////////////////////////////////////////////////////////////
#pragma once

#include <windows.h>
#include <tchar.h>
#include <stdio.h>


#ifdef _DEBUG
#  define Log DebugOnlyLog

void DebugOnlyLog( LPCTSTR message, ... );
void DebugPrint( LPCTSTR message, ... );
void DebugPrintV( LPCTSTR message, va_list args );

#else
#  define Log __noop
#endif
