// DHtmlDialogEx2.cpp : implementation file
//
#include "stdafx.h"
#include "DHtmlDialogEx.h"
#include <AidLib/Logging/cLog.h> 
#include "AidLib/CException/CException.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDHtmlDialogEx dialog

BEGIN_DHTML_EVENT_MAP(CDHtmlDialogEx)
END_DHTML_EVENT_MAP()

// TODO:anatoly needed to prevent IE features like F5 pressed
//
BEGIN_EVENTSINK_MAP(CDHtmlDialogEx, CDHtmlDialog)
	//ON_EVENT(CDHtmlDialogEx, AFX_IDC_BROWSER, DISPID_BEFORENAVIGATE2, OnBeforeNavigate2, VTS_DISPATCH VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PBOOL)
	//ON_EVENT(CDHtmlDialogEx, AFX_IDC_BROWSER, DISPID_NAVIGATECOMPLETE2, OnNavigateComplete2, VTS_DISPATCH VTS_PVARIANT)
	//ON_EVENT(CDHtmlDialogEx, AFX_IDC_BROWSER, DISPID_NEWWINDOW2, OnNewWindow2, VTS_PDISPATCH VTS_PBOOL)
END_EVENTSINK_MAP()

BEGIN_MESSAGE_MAP(CDHtmlDialogEx, CDHtmlDialog)
	//}}AFX_MSG_MAP
	ON_WM_ERASEBKGND() 
	ON_MESSAGE(WM_USER_BORDER_MOVE, OnUserBorderMove)
	ON_WM_CREATE() 
	ON_WM_MOVING()
    ON_WM_NCHITTEST()
END_MESSAGE_MAP()

CDHtmlDialogEx::CDHtmlDialogEx(UINT nIDTemplate, UINT nHtmlResID, CWnd* pParent)
	: CDHtmlDialog(nIDTemplate, nHtmlResID, pParent)
{
TRY_CATCH

// When you drag the skin around is the problem of a LARGE rectangle that appears around the skin
// as you drag it thanks to Microsoft's lack of attention to making things look good.  
// To solve this problem you need to the "SetViewWhileDrag" property and to make sure there is a 
// registry entry for it as well.  This is done as follows: 
// Set registry "HKEY/Control Panel/Desktop/DragFullWindows"
// with string value "1" and set "SystemParametersInfo"
	m_pTransWindow = NULL;
	SystemParametersInfo(SPI_SETDRAGFULLWINDOWS,
                     1,
                     NULL,
                     SPIF_SENDWININICHANGE);

	m_bEnableF5 = FALSE;
	m_bEnableCtrlF4 = FALSE;
	m_bEnableWorkbenchMode = FALSE;

CATCH_THROW(_T("CDHtmlDialogEx::CDHtmlDialogEx"))
}

void CDHtmlDialogEx::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);
}

BOOL CDHtmlDialogEx::OnEraseBkgnd(CDC* pDC)
{
TRY_CATCH
/*
	CRect rcClient;
	GetClientRect( &rcClient );
	pDC->FillRect(rcClient, &CBrush(RGB(255,255,255)));
*/
CATCH_THROW(_T("CDHtmlDialogEx::OnEraseBkgnd"))
	return TRUE; // tell Windows we handled it
}

// CDHtmlDialogEx message handlers
BOOL CDHtmlDialogEx::OnInitDialog()
{
TRY_CATCH

	CDHtmlDialog::OnInitDialog();
	//
	// Needed todo anatoly
	// How to disable drop targets feature of IE?
	// Drop target enabled. You can take any HTML file and drop into the dialog to view it.
	// call m_pBrowserApp->put_RegisterAsDropTarget(VARIANT_FALSE);
	// In addition, I would also try to hook the browser's OnBeforeNavigate2 event to cancel any 
	// undesired navigation, and set the Silent property to TRUE in Release builds.
	m_pBrowserApp->put_RegisterAsDropTarget(VARIANT_FALSE);

CATCH_THROW(_T("CDHtmlDialogEx::OnInitDialog"))
	  return TRUE;  // return TRUE  unless you set the focus to a control
}

int CDHtmlDialogEx::OnCreate(LPCREATESTRUCT lpRes)
{
	int	RetVal = 0;
TRY_CATCH
	RetVal = CDHtmlDialog::OnCreate(lpRes);
CATCH_THROW(_T("CDHtmlDialogEx::OnCreate"))
	return RetVal;
}
/*
	TODO  for integration with Parent Window
	Parent window will Move child itself without sending UserBorderMove
	this funciton may be deleted
*/
long CDHtmlDialogEx::OnUserBorderMove( WPARAM wParam, LPARAM lParam )
{
TRY_CATCH

	RECT	rc;
	int		width;
	int		height;

	GetWindowRect(&rc);
	width = rc.right-rc.left;
	height = rc.bottom - rc.top;

	MoveWindow(((LPRECT)lParam)->left,((LPRECT)lParam)->top,width,height,TRUE);

CATCH_THROW(_T("CDHtmlDialogEx::OnUserBorderMove"))

	return 0;
}

void CDHtmlDialogEx::OnMoving(UINT uint,LPRECT rc)
{
TRY_CATCH

	if( this->m_hWndParent)
	{
		static RECT rect;
		RtlCopyMemory(&rect,rc,sizeof(RECT));
		//CTransparentWindow::OnUserMove(this->m_hWndParent ,rc);
		::PostMessage(this->m_hWndParent, WM_USER_BORDER_MOVE, NULL, (LPARAM)&rect);
	}

CATCH_THROW(_T("CDHtmlDialogEx::OnMoving"))
		
}
/*
	needed todo:anatoly
	We can disable the pop-up menu if InterNetExplorer by overriding ShowContextMenu
	may be we would like event to show our popup menu
*/
HRESULT STDMETHODCALLTYPE CDHtmlDialogEx::ShowContextMenu(DWORD /*dwID*/, POINT *ppt, IUnknown* /*pcmdtReserved*/, IDispatch* /*pdispReserved*/)
{
	return S_OK;
}

void CDHtmlDialogEx::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{
TRY_CATCH

	Log.Add(_MESSAGE_, _T("CDHtmlDialogEx::OnDocumentComplete. Starting"));

	CDHtmlDialog::OnDocumentComplete(pDisp, szUrl);

	CString sDestUrlWithSpaces = m_sDestUrl;
	sDestUrlWithSpaces.Replace(" ","%20"); // Replace all occurrences of " " with "%20"

	if(szUrl==NULL || szUrl[0] == '\0' )
	{
		Log.Add(_MESSAGE_, _T("CDHtmlDialogEx::OnDocumentComplete. srcUrl is NULL"));
		return;
	}

	CString sSrcUrl(szUrl); 
	int iUrlLen = sSrcUrl.GetLength();
	if(sSrcUrl.Right(1)=='#') // Delete last char "fragment identifier" in URL
	{
		Log.Add(_WARNING_, _T("CDHtmlDialogEx::OnDocumentComplete. srcUrl contains # in last character %s"), szUrl);
		sSrcUrl.Delete(iUrlLen - 1, 1);
	}

	if(m_sDestUrl.Compare(sSrcUrl)!=0 && sDestUrlWithSpaces.Compare(sSrcUrl)!=0)
	{
		Log.Add(_MESSAGE_, _T("CDHtmlDialogEx::OnDocumentComplete. srcUrl!=m_sDestUrl. srcUrl: %s. m_sDestUrl: %s"),
			(CString)sSrcUrl, m_sDestUrl);
		return;
	}

	if(this->m_spHtmlDoc== NULL)
	{
		Log.Add(_WARNING_, _T("CDHtmlDialogEx::OnDocumentComplete. m_spHtmlDoc==NULL. szUrl: %s"),(CString)sSrcUrl);
		
		if(szUrl[0]!='\0')
		{
			AfxMessageBox("Code#253. Please restart SupportCenter");
			Log.Add(_ERROR_, _T("CDHtmlDialogEx::OnDocumentComplete. Ops...m_spHtmlDoc==NULL. szUrl: %s"),(CString)sSrcUrl);			
		}

		return;
	}
	else
	{
		Log.Add(_MESSAGE_, _T("CDHtmlDialogEx::OnDocumentComplete with m_spHtmlDoc!=NULL. szUrl: %s"),(CString)sSrcUrl);	
	}

	m_webPage.SetDocument(this->m_spHtmlDoc);
	m_cHTMLInterface.SetDocument(m_webPage);

	Log.Add(_MESSAGE_, _T("CDHtmlDialogEx::OnDocumentComplete. Finished"));

	//pDisp->Release();

CATCH_THROW(_T("CDHtmlDialogEx::OnDocumentComplete"))
}

void CDHtmlDialogEx::OnNewWindow2(LPDISPATCH FAR* ppDisp, BOOL FAR* Cancel) 
{
	Log.Add(_MESSAGE_, _T("CDHtmlDialogEx::OnNewWindow2... Do nothing"));
//	*Cancel = TRUE; // cancel when needed
}

void CDHtmlDialogEx::OnNavigateComplete2(LPDISPATCH pDisp, VARIANT FAR* URL)
{
TRY_CATCH
	CString str(V_BSTR(URL));
	
	if(this->m_spHtmlDoc== NULL)
	{
		Log.Add(_MESSAGE_, _T("CDHtmlDialogEx::OnNavigateComplete2 called. m_spHtmlDoc==NULL. URL: %s"), str);
	}
	else
	{
		Log.Add(_MESSAGE_, _T("CDHtmlDialogEx::OnNavigateComplete2 called. m_spHtmlDoc!=NULL. URL: %s"), str);
	}

	//CDHtmlDialog::_OnNavigateComplete2(pDisp, URL);
	CDHtmlDialog::OnNavigateComplete(pDisp, str);
CATCH_THROW(_T("CDHtmlDialogEx::OnNavigateComplete2"))
}

void CDHtmlDialogEx::OnBeforeNavigate2(LPDISPATCH pDisp, VARIANT FAR* URL, VARIANT FAR* Flags, VARIANT FAR* TargetFrameName, VARIANT FAR* PostData, VARIANT FAR* Headers, BOOL FAR* Cancel)
{
TRY_CATCH

	BOOL	m_bReload = true;
	CString str(V_BSTR(URL));
	
	Log.Add(_MESSAGE_, _T("CDHtmlDialogEx::OnBeforeNavigate2 called. URL: %s"), str);

	str.MakeLower();
	CString sUrl1=m_strCurrentUrl;
	sUrl1.MakeLower();
	CString sUrl2=m_sDestUrl;
	sUrl2.MakeLower();
	if(sUrl1==sUrl2)
	{
		if(m_bReload)
		{
			CDHtmlDialog::OnBeforeNavigate( pDisp, str );
			//CDHtmlDialog::_OnBeforeNavigate2( pDisp, URL, Flags, TargetFrameName, PostData, Headers, Cancel );
			m_bReload = false;
		}
		else
		{
			//Keep the current only one target page when user pressed BACK
			//disable F5 key for refreshing
			*Cancel = TRUE; // cancel when needed
		}
	}
	else
	{
		CDHtmlDialog::OnBeforeNavigate( pDisp, str );
		//CDHtmlDialog::_OnBeforeNavigate2( pDisp, URL, Flags, TargetFrameName, PostData, Headers, Cancel );
	}

CATCH_THROW(_T("CDHtmlDialogEx::OnBeforeNavigate2"))

//	CRegionDHtmlDialog::OnBeforeNavigate(pDisp, str);
//	*Cancel = TRUE; // cancel when needed
}

/* Anatoly todo needed 
Ban some keyboard accelerators in browser control and some app level accelerators
Kill some defects on banning accelerators. 
Use <CSupportMessengerDlg ::PreTranslateMessage>, delete <CSupportMessengerDlg ::TranslateAccelerator>.
Behavior inside edit control is normal now.
Context menu is available only in edit control. 
Text selection is available only in edit control. 
Dropping file is not available. But you can drag and drop text between edit controls. 
All the function keys F1-F12 and Escape key do not work. 
Ctrl-P for printing is banned. 
User can not press Ctrl-N to open a new browser window for the current URL.
*/
BOOL CDHtmlDialogEx::PreTranslateMessage(MSG* pMsg)
{
TRY_CATCH

	if(pMsg->message == WM_SYSKEYDOWN && pMsg->wParam == VK_F4 && m_bEnableCtrlF4)
	{
		PostMessage(WM_SYSCOMMAND,SC_CLOSE,0); 
		return true;
		//return CDHtmlDialog::PreTranslateMessage(pMsg);	
	}

	if(pMsg->wParam == VK_RETURN) // ON Enter pass msg to the parent Window - ActiveX control TODO - to ckeck if Ok
	{
		if(m_bEnableWorkbenchMode==TRUE)
			return CDHtmlDialog::PreTranslateMessage(pMsg);//solve Workbench issue STL-616
		else
			return CWnd::PreTranslateMessage(pMsg);//solve SupportCenter STL-682
	}

	if(	( pMsg->message == WM_KEYDOWN )	|| ( pMsg->message == WM_KEYUP ) ||	
		( pMsg->message == WM_SYSKEYDOWN )	|| ( pMsg->message == WM_SYSKEYUP )	)
	{
		CString sMsg;
		sMsg.Format(_T("Key pressed pMsg->wParam=0x%X\r\n"), pMsg->wParam);
		Log.Add(_CALL_, sMsg);
	}

	BOOL	bEnableNavigatingKeys = 0;

	if( ! bEnableNavigatingKeys )
	{
		//ban function keys
		if(
			(
				( pMsg->message == WM_KEYDOWN )
				||
				( pMsg->message == WM_KEYUP )
				||
				( pMsg->message == WM_SYSKEYDOWN )
				||
				( pMsg->message == WM_SYSKEYUP )
			)
			&&
			(
				( pMsg -> wParam == VK_F1 )
				||
				( pMsg -> wParam == VK_HELP )
				||
				( pMsg -> wParam == VK_F2 )
				||
				( pMsg -> wParam == VK_F3 )
				||
				( pMsg -> wParam == VK_F4 )
				||
				( pMsg -> wParam == VK_F5 && m_bEnableF5 == FALSE)
				||
				( pMsg -> wParam == VK_F6 )
				||
				( pMsg -> wParam == VK_F7 )
				||
				( pMsg -> wParam == VK_F8 )
				||
				( pMsg -> wParam == VK_F9 )
				||
				( pMsg -> wParam == VK_F10 )
				||
				( pMsg -> wParam == VK_F11 )
				||
				( pMsg -> wParam == VK_F12 )
				||
				( pMsg -> wParam == VK_ESCAPE )
			)
		  )
		{
			Log.Add(_CALL_, _T("Function key ignored"));
			return true;
		}

		//ban openning new page
		if( ( GetAsyncKeyState(VK_CONTROL) & 0x8000 ) && ('N' == pMsg->wParam) )
		{
			//Ctrl-N
			Log.Add(_CALL_, _T("Ctrl-N ignored"));
			return true;
		}
		//ban printing
		if( ( GetAsyncKeyState(VK_CONTROL) & 0x8000 ) && ('P' == pMsg->wParam) )
		{
			//Ctrl-P
			Log.Add(_CALL_, _T("Ctrl-P ignored"));
			return true;
		}
		//ban adding to favorites
		if( ( GetAsyncKeyState(VK_CONTROL) & 0x8000 ) && ('D' == pMsg->wParam) )
		{
			//Ctrl-D
			Log.Add(_CALL_, _T("Ctrl-D ignored"));
			return true;
		}
		//ban navigating backward
		if( ( GetAsyncKeyState(VK_MENU) & 0x8000 ) && (VK_LEFT == pMsg->wParam) )
		{
			//Atl-Left
			Log.Add(_CALL_, _T("Atl-Left ignored"));
			return true;
		}
		//ban navigating forward
		if( ( GetAsyncKeyState(VK_MENU) & 0x8000 ) && (VK_RIGHT == pMsg->wParam) )
		{
			//Atl-Right
			Log.Add(_CALL_, _T("Atl-Right ignored"));
			return true;
		}
	}

	if(pMsg->message==WM_TIMER)
	{
		Log.Add(_CALL_, _T("CDHtmlDialogEx::PreTranslateMessage WM_TIMER pMsg->wParam=0x%X"), pMsg->hwnd );	
	}

	Log.Add(_CALL_, _T("CDHtmlDialogEx::PreTranslateMessage pMsg->message=0x%X, pMsg->wParam=0x%X"), pMsg->message, pMsg->wParam);
CATCH_THROW(_T("CDHtmlDialogEx::PreTranslateMessage"))

	return CDHtmlDialog::PreTranslateMessage(pMsg);
}

HRESULT CDHtmlDialogEx::OnHtmlSelectStart(IHTMLElement* pElement)
{
	HRESULT RetVal = S_OK;	
TRY_CATCH

	HRESULT hr=S_FALSE;
	VARIANT_BOOL vbEdit=VARIANT_FALSE;
	CComBSTR bsTagName;

	hr = pElement->get_tagName(&bsTagName);
	if( SUCCEEDED(hr) )
	{
		//CString sTagName=bsTagName.m_str;
		CW2CT szTagName( bsTagName );
		CString sMsg;
		sMsg.Format( _T("OnHtmlSelectStart <%s>\r\n"),szTagName);
		OutputDebugString(sMsg);
	}

	hr = pElement->get_isTextEdit(&vbEdit);
	if( SUCCEEDED(hr) && (VARIANT_TRUE == vbEdit) )
	{
		RetVal =  S_OK;
	}
	else
	{
		::SendMessage(m_hWnd,WM_NCLBUTTONDOWN,HTCAPTION,0);
		//do not pass the event to the IE server/JavaScript
		RetVal =  S_FALSE;
	}
	
CATCH_THROW(_T("CDHtmlDialogEx::OnHtmlSelectStart"))
	return RetVal;
}

HRESULT CDHtmlDialogEx::OnHtmlDragStart(IHTMLElement* pElement)
{
	HRESULT RetVal = S_OK;	
TRY_CATCH

	HRESULT hr=S_FALSE;
	VARIANT_BOOL vbEdit=VARIANT_FALSE;
	CComBSTR bsTagName;
	
	hr = pElement->get_tagName(&bsTagName);
	if( SUCCEEDED(hr) )
	{
		//CString sTagName=bsTagName.m_str;
		CW2CT szTagName( bsTagName );
		CString sMsg;
		sMsg.Format(_T("OnHtmlDragStart <%s>\r\n"),szTagName);
		OutputDebugString(sMsg);
	}

	hr = pElement->get_isTextEdit(&vbEdit);
	if( SUCCEEDED(hr) && (VARIANT_TRUE == vbEdit) )
	{
		RetVal = S_OK;
	}
	else
	{
		::SendMessage(m_hWnd,WM_NCLBUTTONDOWN,HTCAPTION,0);
		//do not pass the event to the IE server/JavaScript
		RetVal = S_FALSE;
	}

CATCH_THROW(_T("CDHtmlDialogEx::OnHtmlDragStart"))
	return RetVal;
}

HRESULT CDHtmlDialogEx::OnHtmlContextMenu(IHTMLElement* pElement)
{
	HRESULT RetVal = S_OK;	

TRY_CATCH
	
	HRESULT hr=S_FALSE;
	VARIANT_BOOL vbEdit=VARIANT_FALSE;
	CComBSTR bsTagName;
	
	hr = pElement->get_tagName(&bsTagName);
	if( SUCCEEDED(hr) )
	{
		//CString sTagName=bsTagName.m_str;
		CW2CT szTagName( bsTagName );
		CString sMsg;
		sMsg.Format(_T("OnHtmlContextMenu <%s>\r\n"),szTagName);
		OutputDebugString(sMsg);
	}

	hr = pElement->get_isTextEdit(&vbEdit);
	if( SUCCEEDED(hr) && (VARIANT_TRUE == vbEdit) )
	{
		//pass the event to the IE server/JavaScript
		RetVal = S_OK;
	}
	else
	{
		//do not pass the event to the IE server/JavaScript
		RetVal = S_FALSE;
	}

CATCH_THROW(_T("CDHtmlDialogEx::OnHtmlContextMen"))
	return RetVal;
}

bool CDHtmlDialogEx::MakeWindowTransparent( int nPercent )
{
TRY_CATCH

	if (nPercent < 0)
	{
		nPercent = 0;
	}

	if (nPercent > 100)
	{
		nPercent = 100;
	}

	if(m_hWnd==0)
	{
		Log.Add(_MESSAGE_, _T("CDHtmlDialogEx::MakeWindowTransparent() parameter error"));
		return false;
	}

	m_layeredWnd.SetTransparentPercentage(m_hWnd, 100-nPercent);

CATCH_THROW(_T("CDHtmlDialogEx::MakeWindowTransparent"))
	return true;
}

LRESULT CDHtmlDialogEx::OnNcHitTest(CPoint point) 
{
	//Log.Add(_MESSAGE_, _T("CDHtmlDialogEx::OnNcHitTest" ) );
	return CDHtmlDialog::OnNcHitTest(point);	
}

void CDHtmlDialogEx::EnableF5(BOOL bEnable)
{
	m_bEnableF5 = bEnable;
}

void CDHtmlDialogEx::EnableCtrlF4(BOOL bEnable)
{
	m_bEnableCtrlF4 = bEnable;
}
void CDHtmlDialogEx::EnableWorkbenchMode(BOOL bEnable)
{
TRY_CATCH
	m_bEnableWorkbenchMode = bEnable;
CATCH_THROW(_T("CDHtmlDialogEx::EnableWorkbenchMode"))
}
