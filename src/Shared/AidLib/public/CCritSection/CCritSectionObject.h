/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CCritSectionObject.h
///
///  Declares CCritSectionObject template - wrapper for Windows critical
///    section object
///
///  @author Dmitry Netrebenko @date 05.03.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>

///  CCritSectionObject template - wrapper for Windows critical
///    section object
template<typename T>
class CCritSectionObject
	:	public CRITICAL_SECTION
{
public:
/// Constructor
	CCritSectionObject()
	{
		try
		{
			InitializeCriticalSection(this);
		}
		catch(...)
		{
		}
	};

/// Destructor
	~CCritSectionObject()
	{
		try
		{
			DeleteCriticalSection(this);
		}
		catch(...) 
		{
		}
	};
};

/// Defines CCritSectionSimpleObject type
typedef CCritSectionObject<void> CCritSectionSimpleObject;
