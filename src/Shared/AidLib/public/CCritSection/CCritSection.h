/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CCritSection.h
///
///  Implements CCritSection class - critical section wrapper
///
///  @author Dmitry Netrebenko @date 11.10.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/AidLIb.h>

///  Critical section wrapper
///  @remarks Class is not thread safe. Object instances cannot be used from different threads
///  different threads should use separate instances of that class
class AIDLIB_API CCritSection
{
protected:
	/// Pointer to critical section used for locks
	LPCRITICAL_SECTION m_cs;
	/// true if section is acquired
	bool m_locked;

	/// disable copy ctor
	CCritSection(const CCritSection&);

	/// disable = operator
	CCritSection& operator=(const CCritSection&);
	
public:
	/// ctor
	/// locks scope
	CCritSection(const LPCRITICAL_SECTION &cs) 
		:	m_cs(cs),
			m_locked(false)
	{
		Lock();
	}

	/// dtor
	/// unlocks scope
	virtual ~CCritSection()
	{
		Unlock();
	}

	/// Locks scope
	/// Remarks (does nothing if scope is locked)
	inline void Lock()
	{
		try
		{
			if (m_locked)
				return;
			EnterCriticalSection(m_cs);
			m_locked = true;
		}
		catch(...)
		{
		}
	}

	/// Remarks (does nothing if scope is unlocked)
	inline void Unlock()
	{
		try
		{
			if (!m_locked)
				return;
			LeaveCriticalSection(m_cs);
			m_locked = false;
		}
		catch(...)
		{
		}
	}
};
