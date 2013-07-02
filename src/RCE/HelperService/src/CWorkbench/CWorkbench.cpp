/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CWorkbench.cpp
///
///  Workbench application for customer side
///
///  @author "Archer Software" Sogin M. @date 08.04.2008
///
////////////////////////////////////////////////////////////////////////
#include "CWorkbench.h"
#include "../resource.h"

#include <cctype>		// for toupper
#include <algorithm>	// for transform

#define WORKBENCH_WIDTH 280L
#define CHECK_PAGE_LOADING_STATE 2000
#define CHECK_PAGE_LOADING_STATE_TIME_INTERVAL 40*1000

std::string util_ToUtf8FromUtf16(const std::wstring& widestring);
std::string util_ReplaceAll(const std::string& input, const std::string& sold, const std::string& snew);

int CWorkbench::Run(HINSTANCE hInstance, const tstring& url)
{
TRY_CATCH

	RECT rc = {0,0,150,450};
	RECT scr_workarea_rect = {0,0,0,0};
	SystemParametersInfo(SPI_GETWORKAREA, 0, &scr_workarea_rect, 0);//work more correct then GetSystemMetrics SM_CXFULLSCREEN

	rc.left=scr_workarea_rect.right-WORKBENCH_WIDTH;
	rc.right=scr_workarea_rect.right;
	rc.top=0;
	rc.bottom=scr_workarea_rect.bottom;	

	m_bNavigateError = false;
	m_url = url;
	m_bPageDownloadComplete = false;

	//rc.left=GetSystemMetrics(SM_CXFULLSCREEN)-WORKBENCH_WIDTH;
	//rc.right=GetSystemMetrics(SM_CXFULLSCREEN);
	//rc.top=0;
	//rc.bottom=GetSystemMetrics(SM_CYFULLSCREEN);	//rc.bottom=GetSystemMetrics(SM_CYSCREEN);

	Create(GetDesktopWindow()/*Parent*/, rc, NULL, WS_OVERLAPPEDWINDOW|WS_VISIBLE);
	if (!IsWindow())
		throw MCException_Win("Failed to Create workbench window");

	//set supportspace application icon 
	HICON appIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDR_MAINFRAME));
	if(appIcon!=NULL)
		SetIcon(appIcon, true);

	/// Setting site in oreder to replace SecurityManager in IE further
	CComPtr<IObjectWithSite> objectWithSite;
	HRESULT hr = QueryHost(IID_IObjectWithSite, (void**)&objectWithSite);
	if (S_OK == hr)
	{
		hr = objectWithSite->SetSite(static_cast<IServiceProvider*>(&m_securityManager));
		m_securityManager.AddRef();
		if (S_OK != hr)
		{
			SetLastError(hr);
			Log.WinError(_ERROR_,_T("Failed to objectWithSite->SetSite "));
		}
	} else
	{
		SetLastError(hr);
		Log.WinError(_ERROR_,_T("Failed to QueryHost(IID_IObjectWithSite, (void**)&objectWithSite) "));
	}

	hr = CreateControl(L"about:blank");
	if (S_OK != hr)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Failed to create IE object ")),hr);

	/// Navigating url
	
	hr = QueryControl(IID_IWebBrowser2, (void**)&m_browser);
	if (S_OK != hr)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Failed to get IWebBrowser2 interface")),hr);

	/// Subscribing events
	hr = Advise(m_browser);
	if (S_OK != hr)
	{
		SetLastError(hr);
		Log.WinError(_ERROR_,_T("Failed to advise to browser events"));
	}

	/// Preventing from errors and security popups TODO or not todo?
	hr = m_browser->put_Silent(VARIANT_TRUE);
	if (S_OK != hr)
	{
		SetLastError(hr);
		Log.WinError(_ERROR_,_T("Failed to browser->put_Silent"));
	}

	//
	// getVersion and OS 
	// in case of ie6 and vista settings of URLZONE_TRUSTED in MapUrlToZone solve us some problems
	Log.Add(_MESSAGE_,_T("Navigate to url:%s"), url.c_str()); 

	/// Navigating onto page
	hr = m_browser->Navigate(CComBSTR(url.c_str()), NULL, NULL, NULL, NULL);
	if (S_OK != hr)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Failed navigate to %s "),url.c_str()),hr);

	// to avoid issue with keystoke like Delete button need to call pIOIPAO->TranslateAccelerator later
	CComPtr<IOleInPlaceActiveObject> pIOIPAO;
	hr = m_browser->QueryInterface(IID_IOleInPlaceActiveObject, (void**)&pIOIPAO);
	if (!SUCCEEDED(hr))
	{
		Log.Add(_ERROR_,_T("QueryInterface IID_IOleInPlaceActiveObject failed. issue with Delete button and other")); 
	}

	UINT uDownloadTimer = SetTimer(CHECK_PAGE_LOADING_STATE, CHECK_PAGE_LOADING_STATE_TIME_INTERVAL, NULL);

	MSG msg;
	HACCEL hAccelTable = NULL;

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if(msg.message==WM_TIMER)
		{
			switch (msg.wParam) 
			{ 
				case CHECK_PAGE_LOADING_STATE: 
					KillTimer(uDownloadTimer);
					if(m_bPageDownloadComplete==false)
					{
						Log.Add(_ERROR_,_T("Open in browser. After timeout")); 
						::MessageBox(NULL, _T("Please, report error code 302 to your expert. Press OK to continue"), _T("SupportSpaceTools warning"), MB_OK);
						//OpenURLWithIE();
					}
					else
					{
						Log.Add(_MESSAGE_,_T("The page was opened after timeout")); 					
					}
					break;
				default:
					break;
			}
		}

		if(msg.wParam == VK_F5)
		{	
			Log.Add(_MESSAGE_,_T("F5 pressed. It is blocked")); 
			continue;
		}

		//TranslateAccelerator solve issue with Delete and Tab not working with Editbox and cause VK_BACK twise called 
		if(msg.wParam != VK_BACK && msg.message == WM_KEYDOWN)
		{
			if(pIOIPAO)
				hr = pIOIPAO->TranslateAccelerator(&msg);
		}
	
		// We have a right click	
		if ((WM_RBUTTONDOWN == msg.message) ||
			(WM_RBUTTONDBLCLK == msg.message))
		{
			Log.Add(_MESSAGE_,_T("We have a right click	with menu pressed. It is blocked"));
		}
		
		//only 3 messages should be sent to javascript TODO seems we do not need this after added pIOIPAO->TranslateAccelerator(&msg)
		if( (msg.wParam == VK_F7 || msg.wParam == VK_F8 || msg.wParam == VK_F9) && (msg.message == WM_KEYDOWN))
		{
			CComVariant pVarResult;
			TCHAR szBuf[256] = {'\0'};
			sprintf_s(szBuf, 256, "0x%x",msg.wParam, szBuf );
			Log.Add(_MESSAGE_,_T("Key pressed: %s"), szBuf); 
			m_webPage.CallJScript(_T("ShortKeyPressed"), szBuf, &pVarResult);
		}

		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	if(pIOIPAO)
		pIOIPAO.Release();

	return 0;
CATCH_LOG()
	return GetLastError();
}

LRESULT CWorkbench::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH
	//do not destroy appplicatio here. notify javascirpt that will call 
	//document.title = "INTERFACE_CLOSE_APPLICATION"; this will result in Destroy from application
	Log.Add(_MESSAGE_,_T("CWorkbench::OnClose")); 
	CComVariant pVarResult;
	m_webPage.CallJScript(_T("OnCustomerPageDestroy"), _T(""), &pVarResult);
	return FALSE;
CATCH_THROW()
}

//http://tech.groups.yahoo.com/group/wtl/message/15040
//OnDestroy is called twice
LRESULT CWorkbench::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("CWorkbench::OnDestroy")); 
	
	if(m_browser)
		m_browser.Release();

	PostQuitMessage(0);
	SetMsgHandled(TRUE);
	return FALSE;
CATCH_THROW()
}

LRESULT CWorkbench::OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH
	//Log.Add(_MESSAGE_,_T("CWorkbench::OnGetMinMaxInfo")); 
	((MINMAXINFO*)lParam)->ptMinTrackSize.x = WORKBENCH_WIDTH;
	//((MINMAXINFO*)lParam)->ptMinTrackSize.y = ;

	return FALSE;
CATCH_THROW()
}

HRESULT CWorkbench::OnTitleChange(BSTR Text)
{
TRY_CATCH
	USES_CONVERSION;
	tstring title = NULL==Text?_T(""):W2T(Text);
	
	if(title == _T("about:blank"))
	   title = _T("Loading chat page...");

	if(title==_T("INTERFACE_CLOSE_APPLICATION"))
	{
		//todo here ....need to close application
		Log.Add(_MESSAGE_,_T("OnTitleChange with parameter INTERFACE_CLOSE_APPLICATION. DestroyWindow called")); 
		DestroyWindow();
		return S_OK;
	}

	if(title.find("INTERFACE_CLOSE_APPLICATION_OPEN_URL")==0)
	{
		Log.Add(_MESSAGE_,_T("OnTitleChange with parameter INTERFACE_CLOSE_APPLICATION_OPEN_URL. DestroyWindow called")); 
		tstring url = title.substr(strlen("INTERFACE_CLOSE_APPLICATION_OPEN_URL#"));
		ShellExecute(NULL, _T("open"), _T("iexplore.exe"), url.c_str(), "", SW_SHOWNORMAL);
		DestroyWindow();
		return S_OK;
	}

	SetWindowText(title.c_str());
CATCH_LOG()
	return S_OK;
}

HRESULT __stdcall CWorkbench::OnNavigateError(	IDispatch *pDisp,
									VARIANT *URL,
									VARIANT *TargetFrameName,
									VARIANT *StatusCode,
									VARIANT_BOOL *&Cancel )
{
TRY_CATCH
	::MessageBox(NULL, _T("Please, report error code 301 to your expert. Press OK to continue"), _T("SupportSpaceTools warning"), MB_OK);
	m_bNavigateError = true;
	//OpenURLWithIE();
CATCH_LOG()
	return S_OK;
}

HRESULT __stdcall CWorkbench::OnDocumentComplete(IDispatch *pDisp,VARIANT *URL)
{
TRY_CATCH

	CComPtr<IDispatch> ppDisp;
	m_browser->get_Document(&ppDisp);

	std::string  s_str = util_ToUtf8FromUtf16((*URL).bstrVal);
	
	std::transform(s_str.begin(), s_str.end(), s_str.begin(), tolower);//all string to lower case 
	std::transform(m_url.begin(), m_url.end(), m_url.begin(), tolower);//all string to lower case 
	
	s_str = util_ReplaceAll(s_str," ","%20");//replace all occuriencies of ' '  

	Log.Add(_MESSAGE_,_T("OnDocumentComplete. Dest url: %s to compare with Src url: %s"), m_url.c_str(), s_str.c_str()); 

	int pos = s_str.find(m_url.c_str());
	
	if(pos!=-1)
	{
		Log.Add(_MESSAGE_,_T("Found url required...")); 
		if(IsPageLoaded())
		{
			Log.Add(_MESSAGE_,_T("Page loaded")); 
			m_bPageDownloadComplete = true;
		}
		else
			Log.Add(_MESSAGE_,_T("Page not loaded")); 
	}
		
	if(pDisp != NULL)
	{
		if(m_webPage.SetDocument(ppDisp)==false)
		{
			return S_OK;
		}
	}

CATCH_LOG()
	return S_OK;
}

bool CWorkbench::IsPageLoaded()
{
    READYSTATE result;
    m_browser->get_ReadyState(&result);
    if(result != READYSTATE_COMPLETE)
		return false;
	else
		return true;
}

HRESULT CWorkbench::OpenURLWithIE()
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("OpenURLWithIE")); 
	ShellExecute(NULL, _T("open"), _T("iexplore.exe"), m_url.c_str(), "", SW_SHOWNORMAL);
	DestroyWindow();
CATCH_LOG()
	return S_OK;
}

std::string util_ToUtf8FromUtf16(const std::wstring& widestring)
{
TRY_CATCH

	int utf8size = ::WideCharToMultiByte(CP_UTF8, 0, widestring.c_str(), -1, NULL, 0, NULL, NULL);
	if (utf8size == 0)
	{
		throw std::exception("Error in conversion ToUtf8FromUtf16. step 1");
	}

	std::vector<char> resultstring(utf8size);

	int convresult = ::WideCharToMultiByte(CP_UTF8, 0, widestring.c_str(), -1, &resultstring[0], utf8size, NULL, NULL);

	if (convresult != utf8size)
	{
		throw std::exception("Error in conversion ToUtf8FromUtf16. step 2");
	}
	return std::string(&resultstring[0]);
CATCH_LOG()
	return _T("");
}

std::string util_ReplaceAll(const std::string& input, const std::string& sold, const std::string& snew)
{
	 std::string output = input;
     int pos = 0; 
     int lpos = 0;
	 while ((pos = output.find(sold, lpos)) != std::string::npos)
     {
           output.replace(pos, sold.length(), snew);
           lpos = pos+1 + snew.length(); 
     }
     return output;
}


