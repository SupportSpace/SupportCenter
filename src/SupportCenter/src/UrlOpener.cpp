#include "StdAfx.h"
#include "UrlOpener.h"
#include <AidLib/Logging/cLog.h> 
#include "AidLib/CException/CException.h"
#include "SupportMessenger.h"

extern CSupportMessengerApp theApp;

CUrlOpener::CUrlOpener()
{
	m_hwndExpertDesktop = NULL;

	if (theApp.m_cSettings.m_sWorkbenchBrowser == "")
	{
		m_strBrowser = "iexplore.exe";
	}
	else
	{
		m_strBrowser = theApp.m_cSettings.m_sWorkbenchBrowser;
	}
}

void CUrlOpener::Open(LPCTSTR lpszURL, bool bNewWindow)
{
TRY_CATCH
/*
	if (m_bUseIE == TRUE)
	{
		//todo
		//ShellExecute(NULL, _T("open"), _T("iexplore.exe"), "-k http://www.ynet.co.il", "", SW_SHOWNORMAL);
		//todo try this: scrollbars=0,fullscreen=1
		HINSTANCE lRes = ShellExecute(NULL, _T("open"), m_strBrowser, lpszURL, "", SW_SHOWNORMAL);
		Log.Add(_MESSAGE_, _T("ShellExecute returned %d"), lRes);
	}
	else
	{
		if ( bNewWindow )
			::ShellExecute(NULL, NULL, m_strBrowser, lpszURL, NULL, SW_SHOWNORMAL);
		else
			::ShellExecute(NULL, NULL, lpszURL, NULL, NULL, SW_SHOWNORMAL);
	}
*/
	HINSTANCE lRes = ShellExecute(NULL, _T("open"), m_strBrowser, lpszURL, "", SW_SHOWNORMAL);
	Log.Add(_MESSAGE_, _T("ShellExecute returned %d"), lRes);
CATCH_THROW(_T("CUrlOpener::Open"))
}

LPCTSTR CUrlOpener::GetBrowser(void)
{
TRY_CATCH

	// Do we have the default browser yet?
	if (m_strBrowser.IsEmpty())
	{
		// Get the default browser from HKCR\http\shell\open\command
		HKEY hKey = NULL;
		// Open the registry
		if (::RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("http\\shell\\open\\command"), 0,
			KEY_READ, &hKey) == ERROR_SUCCESS)
		{
			// Data size
			DWORD cbData = 0;
			// Get the default value
			if (::RegQueryValueEx(hKey, NULL, NULL, NULL, NULL, &cbData) == ERROR_SUCCESS && cbData > 0)
			{
				// Allocate a suitable buffer
				TCHAR* psz = new TCHAR [cbData];
				// Success?
				if (psz != NULL)
				{
					if (::RegQueryValueEx(hKey, NULL, NULL,
						NULL, (LPBYTE)psz, &cbData) ==
						ERROR_SUCCESS)
					{
						// Success!
						m_strBrowser = psz;
					}
					delete [] psz;
				}
			}
			::RegCloseKey(hKey);
		}
		// Do we have the browser?
		if (m_strBrowser.GetLength() > 0)
		{
			// Strip the full path from the string
			int nStart = m_strBrowser.Find('"');
			int nEnd = m_strBrowser.ReverseFind('"');
			// Do we have either quote?
			// If so, then the path contains spaces
			if (nStart >= 0 && nEnd >= 0)
			{
				// Are they the same?
				if (nStart != nEnd)
				{			
					// Get the full path
					m_strBrowser = m_strBrowser.Mid(nStart + 1, nEnd - nStart - 1);
				}
			}
			else
			{
				// We may have a pathname with spaces but
				// no quotes (Netscape), e.g.:
				//   C:\PROGRAM FILES\NETSCAPE\COMMUNICATOR\PROGRAM\NETSCAPE.EXE -h "%1"
				// Look for the last backslash
				int nIndex = m_strBrowser.ReverseFind('\\');
				// Success?
				if (nIndex > 0)
				{
					// Look for the next space after the final
					// backslash
					int nSpace = m_strBrowser.Find(' ', nIndex);
					// Do we have a space?
					if (nSpace > 0)
						m_strBrowser = m_strBrowser.Left(nSpace);				
				}
			}
		}
	}
	// Done
	return m_strBrowser;

CATCH_THROW(_T("CUrlOpener::GetBrowser"))
}

/**
 * 
 * You can't use ShellExecute to Launch IE Browser from
 * OnBeforeNavigate2 or OnDocumentComplere Events!!!
 * Because it can result in a viscious loop and freeze!!!
 * Use this method to launch IE Browser from WebBrowser Control
 */
BOOL CUrlOpener::LauchIEBrowser(CString csUrlPage)
{
	try
	{
		//
		//	limit m_hwndExpertDesktop by one instance - not for Closed beta
		//
		/*
		if( m_hwndExpertDesktop != NULL ){
		//::SetForegroundWindow(hwnd); 
		//FindWindow();
		if(IsWindow(m_hwndExpertDesktop)==TRUE)
		{
		::ShowWindow(m_hwndExpertDesktop, SW_SHOWMAXIMIZED);
		return TRUE; 
		}
		else
		{
		m_hwndExpertDesktop = NULL;
		}
		}
		*/
		IWebBrowser2* m_pBrowserApp = NULL;

		// Use ColeDispatchDriver to create automation object
		// This also takes care of releasing IDispatch pointer
		// TODO press F1 and add try catch
		COleDispatchDriver dispIE;
		dispIE.CreateDispatch(CLSID_InternetExplorer);

		// Get IWebBrowser2 interface
		if(dispIE.m_lpDispatch==NULL)
		{
			AfxMessageBox(_T("dispIE.CreateDispatch(CLSID_InternetExplorer) failed"));//todo log
			return FALSE;
		}

		HRESULT hr = dispIE.m_lpDispatch->QueryInterface(IID_IWebBrowser2, reinterpret_cast<void **> (&m_pBrowserApp));

		if(FAILED(hr)){
			AfxMessageBox(_T("Failed to get IWebBrowser2 interface. HRESULT"));//todo log
			return FALSE;
		}
		
		if(m_pBrowserApp != NULL)
		{
			COleVariant vaFlag, vaPostData, vaHeaders;
			COleVariant vaURL(csUrlPage);
			COleVariant vaTargetFrameName;
			//
			// todo not for closed Betta	
			// known problem is that if at this momen one or more IE already opened
			// then new IE will be opened, but the problem is that when close our IE
			// it will also close First opened IE
			// this not happened me from clear test of m_pBrowserApp->Navigate2 function
			// not inside SupportMessnger code
			// navOpenInNewWindow 
			// http://msdn2.microsoft.com/en-US/library/aa768360.aspx
			// vaFlag = navOpenInNewWindow | navNoReadFromCache | navNoWriteToCache | navBrowserBar | navHyperlink;
			// http://support.microsoft.com/kb/315762
			VARIANTARG vWorkaround;
			VariantInit(&vWorkaround);
			vWorkaround.vt = VT_I4;
			vWorkaround.lVal = navNewWindowsManaged;//navOpenInNewWindow; //navNoReadFromCache ;

			hr = m_pBrowserApp->Navigate2(&vaURL, &vWorkaround, &vaTargetFrameName, &vaPostData, &vaHeaders);

			m_pBrowserApp->get_HWND((long *)&m_hwndExpertDesktop);

			if(m_hwndExpertDesktop != NULL)
			{
				//::SetForegroundWindow(hwnd); 
				
				//SendMessage(m_hwndExpertDesktop, WM_KEYDOWN, VK_F11, 5701633); 
				//PostMessage(m_hwndExpertDesktop, WM_KEYDOWN, VK_F11, 5701633); 
				//keybd_event(VK_F11, 0, 0, 0);  //working as well
				::ShowWindow(m_hwndExpertDesktop, SW_SHOWMAXIMIZED);
				m_pBrowserApp->put_Visible(TRUE);
				m_pBrowserApp->put_TheaterMode(VARIANT_TRUE);
			}
			dispIE.m_lpDispatch->Release();
			m_pBrowserApp->Release();
			CloseHandle(m_hwndExpertDesktop);
		}
	}
	catch(...)
	{
		return FALSE;//todo log
	}

	return TRUE;
}

