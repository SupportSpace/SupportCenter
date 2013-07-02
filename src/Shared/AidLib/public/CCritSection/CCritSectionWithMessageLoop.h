/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CCritSectionWithMessageLoop.h
///
///  Implements CCritSectionWithMessageLoop class - critical section wrapper + message loop
///
///  @author Kirill Solovyov @date 27.05.2008
///
////////////////////////////////////////////////////////////////////////

#pragma once

//#include <AidLib/AidLIb.h>

///  Critical section wrapper
///  @remarks Class is not thread safe. Object instances cannot be used from different threads
///  different threads should use separate instances of that class
//class AIDLIB_API CCritSectionWithMessageLoop
class CCritSectionWithMessageLoop
{
	enum { TIMEOUT=100};
protected:
	/// Pointer to critical section used for locks
	LPCRITICAL_SECTION m_cs;
	/// true if section is acquired
	bool m_locked;

	/// disable copy ctor
	CCritSectionWithMessageLoop(const CCritSection&);

	/// disable = operator
	CCritSectionWithMessageLoop& operator=(const CCritSection&);
	
public:
	/// ctor
	/// locks scope
	CCritSectionWithMessageLoop(const LPCRITICAL_SECTION &cs) 
		:	m_cs(cs),
			m_locked(false)
	{
		Lock();
	}

	/// dtor
	/// unlocks scope
	virtual ~CCritSectionWithMessageLoop()
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

			DWORD dwRet;
			MSG msg;

			while(1)
			{
				if(TryEnterCriticalSection(m_cs))
				{
					m_locked = true;
					break;
				}
				// There is one or more window message available. Dispatch them
				while(PeekMessage(&msg,0,0,0,PM_NOREMOVE))
				{
					// check for unicode window so we call the appropriate functions
					BOOL bUnicode = ::IsWindowUnicode(msg.hwnd);
					BOOL bRet;

					if (bUnicode)
						bRet = ::GetMessageW(&msg, NULL, 0, 0);
					else
						bRet = ::GetMessageA(&msg, NULL, 0, 0);

					if (bRet > 0)
					{
						::TranslateMessage(&msg);

						if (bUnicode)
							::DispatchMessageW(&msg);
						else
							::DispatchMessageA(&msg);
					}
				}
				
				dwRet = MsgWaitForMultipleObjects(0, NULL, FALSE, TIMEOUT, QS_ALLINPUT);

				if (WAIT_OBJECT_0!=dwRet&&WAIT_TIMEOUT!=dwRet)
					Sleep(TIMEOUT);
			}

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