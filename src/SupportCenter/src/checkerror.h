//*****************************************************************************
//
//      Author:         Kenny Kerr
//      Date created:   8 June 2004
//      Description:    The CheckError functions are used to provide consistent 
//                      handling of errors when using Win32 and COM.
//
//*****************************************************************************

#pragma once

#include <atlbase.h>

inline HRESULT CheckError(HRESULT result)
{
	if (FAILED(result))
	{
		AtlThrow(result);
	}

	return result;
}

inline void CheckError(BOOL result)
{
	if (!result)
	{
		AtlThrowLastWin32();
	}
}

inline void CheckError(DWORD result)
{
	if (ERROR_SUCCESS != result)
	{
		AtlThrow(HRESULT_FROM_WIN32(result));
	}
}

