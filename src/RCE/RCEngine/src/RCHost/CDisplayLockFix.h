#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CDisplayLockFix.h
///
///  Fix for session brake during display lock
///
///  @author "Archer Software" Sogin M. @date 26.04.2007
///
////////////////////////////////////////////////////////////////////////
#include "vncDesktop.h"
#include "vncService.h"
#include <AidLib/Logging/cLog.h>
#include <AidLib/CException/CException.h>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <AidLib/CThread/CThread.h>

/// Timeout for one display capturing operation
#define CAPTURE_TIMEOUT 3000

/// Timeout beetween getting winlogon desktops
#define REFRESH_WINLOGON_TIMEOUT 3000

class vncDesktop;

/// Class, fixing bug when session brake during display lock
class CDisplayLockFix : private CThread
{
private:
	/// DC, where we'll draw something like "desktop is locked. Remote control isn't possible"
	boost::shared_ptr< boost::remove_pointer<HDC>::type> m_hLockedDC;
	boost::shared_ptr< boost::remove_pointer<HBITMAP>::type> m_hBitmap;
	HBITMAP m_hBitmapOld;
	/// Original vncDesktop::m_hrootDC
	HDC m_originalDC;
	/// true if we're now showing locked DC
	bool m_fixActive;
	/// pointer to VNC desktop
	vncDesktop *m_desktop;

	/// Current bitmap of winlogon desktop
	boost::shared_ptr< boost::remove_pointer<HBITMAP>::type> m_winlogonBitmap;
	/// Current winlogon desktop handle
	boost::shared_ptr<boost::remove_pointer<HDESK>::type> m_winlogonDesktop;
	/// Event to synchronize with internal thread
	boost::shared_ptr<boost::remove_pointer<HANDLE>::type> m_capturedEvent;

	/// Internal thread entry point
	void Execute(void*);

	/// Refreshes m_winlogonBitmap
	void CaptureWinLogonBitmap();

	/// Last time we get winlogon
	DWORD m_lastGetWinlogonTime;

	HMODULE m_desktopVNCHooksHandle;

public:
	/// Initializes object instance
	/// @param desktop pointer to vncDesktop instance
	CDisplayLockFix(vncDesktop *desktop);

	/// Destroys object instance
	virtual ~CDisplayLockFix();

	/// Should be called instead of vncService::InputDesktopSelected
	bool InputDesktopSelected();

	/// Returns true if fix active now
	inline bool Active()
	{
		return m_fixActive;
	}
private:
	/// Handles display was locked 
	void HandleDisplayLocked();

	/// Fixes locked desktop
	void ActivateFix();

	/// UnFixes locked desktop
	void DeactivateFix();

	/// Draws logon screen on hdc (used when we cannot capture logon screen)
	/// @param hDC device context object to draw on
	void DrawLogonScreen(HDC hDC);

	/// Deletes hdc
	/// @param hDC device context object to delete
	void DeleteDC(HDC hDC);
};
