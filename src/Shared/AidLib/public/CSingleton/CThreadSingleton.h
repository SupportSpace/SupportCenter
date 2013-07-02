/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CThreadSingleton.h
///
///  Declares CThreadSingleton template - responsible for single instancing
///    of object per thread
///
///  @author Dmitry Netrebenko @date 05.03.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <map>
#include <AidLib/CCritSection/CCritSectionObject.h>
#include "CSingleton.h"
#include <AidLib/CCritSection/CCritSection.h>

template<typename T>
class CThreadSingleton
{
private:
	CThreadSingleton();
	CThreadSingleton(const CThreadSingleton&);
	CThreadSingleton& operator=(const CThreadSingleton&);
public:
	static T& instance()
	{
		static std::map<unsigned int, T> objMap;
		CCritSection section(&CSingleton<CCritSectionObject<T> >::instance());
		return objMap[GetCurrentThreadId()];
	};
};
