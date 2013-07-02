#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CWorkbench.h
///
///  Workbench application for customer side
///
///  @author "Archer Software" Sogin M. @date 08.04.2008
///
////////////////////////////////////////////////////////////////////////
#include <atlbase.h>
#include <atlwin.h>
#include <AidLib/CException/CException.h>
#include <Exdisp.h>
#include <exdispid.h>
#include "CSecurityManager.h"
#include "CWebPage.h"

/// GUID of library with web browser
struct __declspec(uuid("eab22ac0-30c1-11cf-a7eb-0000c05bae0b")) __WEBBROWSER_LIB;

/// Workbench application window
/// Hosts WebBrowser control
class CWorkbench 
	:	
		public CWindowImpl<CWorkbench, CAxWindow>,
		public IDispEventImpl<0, CWorkbench, &DIID_DWebBrowserEvents2, &__uuidof(__WEBBROWSER_LIB), 1, 0>,
		public CComObjectRoot
{
protected:
	CComObject<CSecurityManager> m_securityManager;
	CComPtr<IWebBrowser2> m_browser;
	CWebPage m_webPage;
	BOOL	m_bNavigateError;
	BOOL	m_bPageDownloadComplete;
	tstring m_url;

public:
	/// Run workbench application
	int Run(HINSTANCE hInstance, const tstring& url);

	DECLARE_WND_SUPERCLASS(_T("CWorkBench"), CAxWindow::GetWndClassName())

	BEGIN_MSG_MAP(CWorkbench)	
		TRY_CATCH
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)
		CATCH_LOG()
	END_MSG_MAP()

	//empty sink map
	BEGIN_SINK_MAP(CWorkbench)
		SINK_ENTRY_EX(0, DIID_DWebBrowserEvents2, DISPID_TITLECHANGE, OnTitleChange)
		SINK_ENTRY_EX(0, DIID_DWebBrowserEvents2, DISPID_NAVIGATEERROR, OnNavigateError)
		SINK_ENTRY_EX(0, DIID_DWebBrowserEvents2, DISPID_DOCUMENTCOMPLETE, OnDocumentComplete)
		
	END_SINK_MAP()

	/// WM_DESTROY message handler
	/// @param uMsg message code (expected to be WM_DESTROY)
	/// @param This parameter is not used.
	/// @param This parameter is not used.
	/// @param bHandled set to TRUE if message should not be further processed
	/// @return return value is obsolete
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);


	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);


	LRESULT OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	bool	IsPageLoaded();
	LRESULT	OpenURLWithIE();

protected:


// Event handlers------------------------------------------------------------

	/// DWebBrowserEvents2::TitleChange event handler
	HRESULT __stdcall OnTitleChange(BSTR Text);

	/// DWebBrowserEvents2::NavigateError Event event handler
	/// Fires when an error occurs during navigation.
	HRESULT __stdcall OnNavigateError(	IDispatch *pDisp,
									VARIANT *URL,
									VARIANT *TargetFrameName,
									VARIANT *StatusCode,
									VARIANT_BOOL *&Cancel );
	/// DWebBrowserEvents2::OnDocumentComplete Event event handler 
	//	Fires when page loaded. is called several times
	HRESULT __stdcall OnDocumentComplete(IDispatch *pDisp,VARIANT *URL);

};

