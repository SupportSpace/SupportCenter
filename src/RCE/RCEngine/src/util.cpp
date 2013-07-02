/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  util.cpp
///
///  
///
///  @author "Archer Software"  @date 11.12.2006
///
////////////////////////////////////////////////////////////////////////
#include <AidLib/Logging/cLog.h>
#include <AidLib/CException/CException.h>
#include "util.h"

ATOM RegisterClassForced(WNDCLASSEX& wcx)
{
TRY_CATCH
	ATOM wndClass;
	if (!(wndClass = RegisterClassEx(&wcx)))
	{
		if (/*Class already exists*/1410 == GetLastError())
		{
			if (!UnregisterClass(wcx.lpszClassName, GetModuleHandle(NULL)/*handle to application instance*/))
			{
				throw MCException_Win("Failed to UnregisterClass class");
			} else
				Log.Add(_MESSAGE_,_T("\"%s\" class unregistered before repeated registration"),wcx.lpszClassName);
			if (!(wndClass = RegisterClassEx(&wcx)))
			{
				throw MCException_Win("Failed to registre class");
			}
		}
	}
	return wndClass;
CATCH_THROW()
}

ATOM RegisterClassForced(WNDCLASS& wcx)
{
TRY_CATCH
	ATOM wndClass;
	if (!(wndClass = RegisterClass(&wcx)))
	{
		if (/*Class already exists*/1410 == GetLastError())
		{
			if (!UnregisterClass(wcx.lpszClassName, GetModuleHandle(NULL)/*handle to application instance*/))
			{
				throw MCException_Win("Failed to UnregisterClass class");
			} else
				Log.Add(_MESSAGE_,_T("\"%s\" class unregistered before repeated registration"),wcx.lpszClassName);
			if (!(wndClass = RegisterClass(&wcx)))
			{
				throw MCException_Win("Failed to registre class");
			}
		}
	}
	return wndClass;
CATCH_THROW()
}