/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CDisplayLockFix.cpp
///
///  Fix for session brake during display lock
///
///  @author "Archer Software" Sogin M. @date 26.04.2007
///
////////////////////////////////////////////////////////////////////////

#include "CDisplayLockFix.h"
#include <AidLib/cTime/CTime.h>
#include <boost/bind.hpp>
#include "vncDesktopThread.h"
#include <AidLib/CThread/CThreadLs.h>

CDisplayLockFix::CDisplayLockFix(vncDesktop *desktop)
	:	m_fixActive(false),
		m_desktop(desktop),
		CThread(),
		m_lastGetWinlogonTime(0),
		m_desktopVNCHooksHandle(NULL)
{
TRY_CATCH

	if (desktop == NULL)
		throw MCException("desktop == NULL");

	m_capturedEvent.reset(CreateEvent(NULL, FALSE, FALSE, NULL),CloseHandle);
	if (NULL == m_capturedEvent.get())
		Log.WinError(_ERROR_,_T("Failed to CreateEvent"));

CATCH_LOG()
}

CDisplayLockFix::~CDisplayLockFix()
{
TRY_CATCH
	if (m_fixActive)
		DeactivateFix();
CATCH_LOG()
}

bool CDisplayLockFix::InputDesktopSelected()
{
TRY_CATCH
	if (vncService::InputDesktopSelected() == FALSE)
	{
		// Display is probably locked
		if (vncService::SelectDesktop(NULL) == FALSE) 
		{
			// Display is locked
			if (m_fixActive)
			{
				//Already in fix, refreshing screen
				DrawLogonScreen(m_hLockedDC.get());
				return true; 
			}
			// Activating fix...
			ActivateFix();
			m_fixActive = true;
			return true;
		} else
		{
			return false; //display isn't locked, doing how VNC wants
		}
	} else
	{
		/// Display isn't locked 
		if (m_fixActive)
		{
			//deactivating fix
			m_fixActive = false;
			DeactivateFix();
			return true;
		} else
		{
			// We are within fix, nothing to do
			return true;
		}
	}
	return false;
CATCH_THROW()
}

void CDisplayLockFix::ActivateFix()
{
TRY_CATCH

	///Starting internal thread
	Start();

	///Since vncHooks doesn't work in secure desktop, switching to pooling
	m_desktopVNCHooksHandle = m_desktop->hModule;
	m_desktop->hModule = NULL;

	m_hLockedDC.reset(CreateCompatibleDC(m_desktop->m_hrootdc), boost::bind(&CDisplayLockFix::DeleteDC, this, _1));
	if (m_hLockedDC.get() == NULL)
		throw MCException_Win("Failed to CreateCompatibleDC");

	CaptureWinLogonBitmap();
	if (NULL != m_winlogonBitmap.get())
	{
		m_hBitmap = m_winlogonBitmap;
	} else
	{
		// Creating compatible bitmap
		m_hBitmap.reset(CreateCompatibleBitmap(	m_hLockedDC.get(), 
												GetDeviceCaps(m_hLockedDC.get(), HORZRES), 
												GetDeviceCaps(m_hLockedDC.get(), VERTRES)), 
						DeleteObject); 
		if (NULL == m_hBitmap.get())
			throw MCException_Win("Failed to CreateCompatibleBitmap m_hbmScreen");
	}

	m_hBitmapOld = reinterpret_cast<HBITMAP>(SelectObject(m_hLockedDC.get(), m_hBitmap.get()));
	if (m_hBitmapOld == NULL)
		throw MCException_Win("Failed to SelectObject m_hbmScreen");

	DrawLogonScreen(m_hLockedDC.get());
	m_originalDC = m_desktop->m_hrootdc;
	m_desktop->m_hrootdc = m_hLockedDC.get();

	/// Forcing screen capture
	int cx = GetDeviceCaps(m_hLockedDC.get(), HORZRES); 
	int cy = GetDeviceCaps(m_hLockedDC.get(), VERTRES); 
	PostThreadMessage(m_desktop->m_thread->id() ,RFB_SCREEN_UPDATE, MAKELONG(0,0), MAKELONG(cx,cy));

CATCH_LOG()
}

void CDisplayLockFix::DeactivateFix()
{
TRY_CATCH

	///Restoring hooks / pooling mode
	m_desktop->hModule = m_desktopVNCHooksHandle;

	m_desktop->m_hrootdc = m_originalDC;
	/// Stopping internal thread
	Terminate();
	PostThreadMessage(GetTid(),WM_QUIT,0,0);
	WaitForSingleObject(hTerminatedEvent.get(), CAPTURE_TIMEOUT);
	m_winlogonDesktop.reset();

CATCH_LOG()
}

void CDisplayLockFix::DrawLogonScreen(HDC hDC)
{
TRY_CATCH

	//Log.Add(_MESSAGE_,_T("Refreshing logon screen...")); //TODO remove this

	int cx = GetDeviceCaps(hDC, HORZRES);
	int cy = GetDeviceCaps(hDC, VERTRES);

	CaptureWinLogonBitmap();
	if (NULL != m_winlogonBitmap.get())
	{
		if (NULL == SelectObject(hDC,m_winlogonBitmap.get()))
			Log.WinError(_ERROR_,_T("Failed to select winlogon desktop"));
		m_hBitmap = m_winlogonBitmap;
	} else
	{
		tstring text = _T("Your customer's computer seems to be locked.\nPlease ask the customer to unlock it, and try to access it again.");//Format(_T("Remote desktop is locked. Remote time is %s"),cDate().GetNow().FormatTime().c_str());
		HFONT hFontOld;
		LOGFONT logFont;
		memset(&logFont, 0, sizeof(logFont));
		_tcscpy_s(logFont.lfFaceName,sizeof(logFont.lfFaceName),_T("Arial"));
		logFont.lfHeight = -17;
		logFont.lfQuality = ANTIALIASED_QUALITY;
		HFONT hFont = CreateFontIndirect(&logFont);
		if (hFont)
		{
			hFontOld = (HFONT)SelectObject(hDC,hFont);
		} else
			Log.WinError(_ERROR_,_T("Failed to CreateFontIndirect "));
		if (SetTextColor(hDC, RGB(255,255,255)) == CLR_INVALID)
			Log.WinError(_ERROR_,_T("Failed to SetTextColor "));
		if (SetBkColor(hDC, RGB(0,0,0)) == CLR_INVALID)
			Log.WinError(_ERROR_,_T("Failed to SetBkColor "));

		SIZE sz;
		RECT rc;
		rc.top = 0;
		rc.left = 0;
		rc.right = cx;
		rc.bottom = cy;
		if (GetTextExtentPoint32( hDC, text.c_str(), text.length(), &sz ) != FALSE)
		{
			rc.top = cy/2 - sz.cy;
			rc.bottom = rc.top + sz.cy*2;
		} else
			Log.WinError(_ERROR_,"Failed to GetTextExtentPoint32 ");
		DrawText( hDC , text.c_str() , text.length() , &rc , DT_VCENTER | DT_CENTER );
		if (hFont)
			SelectObject(hDC,hFontOld);

		/// Forcing screen capture
		//PostThreadMessage(m_desktop->m_thread->id() ,RFB_SCREEN_UPDATE, MAKEWORD(rc.left,rc.left+sz.cx), MAKEWORD(rc.top,rc.top+sz.cy));
		PostThreadMessage(m_desktop->m_thread->id() ,RFB_SCREEN_UPDATE, MAKELONG(0,0), MAKELONG(cx,cy));
	}

CATCH_THROW()
}

void CDisplayLockFix::DeleteDC(HDC hDC)
{
TRY_CATCH
	if (m_hBitmapOld)
	{
		SelectObject(hDC, m_hBitmapOld);
	}
	DeleteObject(hDC);
CATCH_LOG()
}


void CDisplayLockFix::Execute(void*)
{
TRY_CATCH
	SET_THREAD_LS;
	MSG msg;
	HWND window = NULL;
	HDC dc = NULL;
	int cx = 0, cy = 0;
	boost::shared_ptr<boost::remove_pointer<HDC>::type> memoryDC;
	while(!Terminated())
	{
		WaitMessage();
		if (Terminated())
			return;
		if (FALSE != PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
			TRY_CATCH
				if (NULL == m_winlogonDesktop.get())
					throw MCException("NULL == m_winlogonDesktop.get()");

				if (FALSE == SetThreadDesktop(m_winlogonDesktop.get()))
					throw MCException_Win("Failed to SetThreadDesktop ");

				window = GetDesktopWindow();
				if (NULL == window)
					throw MCException_Win("NULL == window");

				dc = GetDC(window);
				if (NULL == dc)
					throw MCException_Win("NULL == dc.get()");

				// Create a memory device context compatible with the device. 
				memoryDC.reset(CreateCompatibleDC(dc),::DeleteDC);
				if (NULL == memoryDC.get())
					throw MCException_Win("NULL == memoryDC.get()");

				// Retrieve the width and height of window display elements.
				cx = GetDeviceCaps(dc, HORZRES);
				cy = GetDeviceCaps(dc, VERTRES);

				// Create a bitmap compatible with the device associated with the 
				// device context.
				m_winlogonBitmap.reset(CreateCompatibleBitmap (dc, cx, cy), DeleteObject);
				if (NULL == m_winlogonBitmap.get())
					throw MCException_Win("Failed to create bitmap for winlogon desktop");

				HBITMAP oldBitmap;
				if (NULL == (oldBitmap = reinterpret_cast<HBITMAP>(SelectObject(memoryDC.get(), m_winlogonBitmap.get()))))
					throw MCException_Win("Failed to select new bitmap to memory dc ");

				if (FALSE == BitBlt(memoryDC.get(), 0,0, cx, cy, dc, 0, 0, SRCCOPY))
					Log.WinError(_ERROR_,_T("Failed to BitBtl "));

				if (FALSE == SelectObject(memoryDC.get(), oldBitmap))
					Log.WinError(_WARNING_,_T("Failed to restore original birmap for memory dc"));

			CATCH_LOG()

			/// Releasing dc if needed
			if (NULL != window && NULL != dc)
				ReleaseDC(window,dc);

			/// Unblocking waiting thread
			SetEvent(m_capturedEvent.get());
		}
	}
CATCH_LOG()
}

void CDisplayLockFix::CaptureWinLogonBitmap()
{
TRY_CATCH

	/// Retriving handle to winlogon desktop
	if (NULL == m_desktop->m_server->GetSrvCommunicator())
		return;

	int timeDelta = timeGetTime() - m_lastGetWinlogonTime;
	if (NULL == m_winlogonDesktop.get() || timeDelta > REFRESH_WINLOGON_TIMEOUT)
	{
		m_winlogonDesktop.reset(reinterpret_cast<HDESK>(m_desktop->m_server->GetSrvCommunicator()->SendMsg(SRVCOMM_GET_DESKTOP,NULL,0)),CloseDesktop);
		m_lastGetWinlogonTime = timeGetTime();
	}

	if (NULL == m_winlogonDesktop.get())
	{
		Log.Add(_ERROR_,_T("Proxy application returned NULL instead of winlogon desktop"));
		return;
	}

	if (FALSE == PostThreadMessage(GetTid(), WM_USER, 0 ,0))
	{
		Log.WinError(_ERROR_,_T("PostThreadMessage to winlogon desktop capturing thread failed "));
		return;
	}

	/// Waiting while internal thread captures desktop
	if (WaitForSingleObject(m_capturedEvent.get(), CAPTURE_TIMEOUT) != WAIT_OBJECT_0)
		Log.WinError(_ERROR_,_T("Waiting for winlogon capture failed "));

CATCH_THROW()
}
