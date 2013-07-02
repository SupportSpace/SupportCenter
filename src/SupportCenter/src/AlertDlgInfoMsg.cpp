#include "StdAfx.h"
#include "AlertDlgInfoMsg.h"

#include <AidLib/Logging/cLog.h> 
#include "AidLib/CException/CException.h"

//#define DHTML_INTERFACE_MSG_NEW_REQUEST			_T("stargate_new_request")
#define DHTML_INTERFACE_MSG_ALERT_INFO_MSG		_T("shuttle_info_msg_notification")
#define ALERT_INFO_MSG_HTML_FILENAME				_T("%s\\alert.html")

#define ALERT_INFO_MSG_DIALOG_WIDTH			213//219
#define ALERT_INFO_MSG_DIALOG_HEIGHT			150//130
#define ALERT_INFO_MSG_DIALOG_OFFSET			25//offset between alerts 

#define CLOSE_ALERT_TIMER					0x150	
#define CLOSE_ALERT_TIMER_WAIT_TIMEOUT		45000	//one minute to give 15 seconds server side to send close

//
//	define pointer to Callback function of the class CAlertDlgInfoMsg
//
typedef HRESULT (CAlertDlgInfoMsg::*pfnCallbackAlertDlgInfoMsg)(IHTMLElement *pElement);
//
//	define pair used for mapping string and callbacks
//
typedef std::pair<CString, pfnCallbackAlertDlgInfoMsg> DHTML_CUSTOM_EVENT_ALERT_DLG;
//
//	init pairs map of calbacks
//
static const DHTML_CUSTOM_EVENT_ALERT_DLG callBacksPairsAlertDlgInfoMsg[] =
{
	DHTML_CUSTOM_EVENT_ALERT_DLG(_T("onclick"),CAlertDlgInfoMsg::OnAlertPickUp),
	DHTML_CUSTOM_EVENT_ALERT_DLG(_T("ReportIssue"),CAlertDlgInfoMsg::OnReportIssue),
	DHTML_CUSTOM_EVENT_ALERT_DLG(_T("ReadMore"),CAlertDlgInfoMsg::OnReadMore),
	DHTML_CUSTOM_EVENT_ALERT_DLG(_T("closeAlert"),CAlertDlgInfoMsg::OnAlertClose),
	DHTML_CUSTOM_EVENT_ALERT_DLG(_T("EnterAlert"),CAlertDlgInfoMsg::OnAlertEnter),
	DHTML_CUSTOM_EVENT_ALERT_DLG(_T("LeaveAlert"),CAlertDlgInfoMsg::OnAlertLeave),
};
//
//	define map of callbacks
//
const static std::map<CString, pfnCallbackAlertDlgInfoMsg> mapCallbacks(callBacksPairsAlertDlgInfoMsg, callBacksPairsAlertDlgInfoMsg + sizeof(callBacksPairsAlertDlgInfoMsg)/sizeof(callBacksPairsAlertDlgInfoMsg[0]));
typedef std::map<CString, pfnCallbackAlertDlgInfoMsg>::const_iterator MapCallbacksAlertDlgIterator;

BEGIN_MESSAGE_MAP(CAlertDlgInfoMsg, CAlertDlg)
	ON_WM_GETMINMAXINFO()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CAlertDlgInfoMsg::~CAlertDlgInfoMsg(void)
{
}

CAlertDlgInfoMsg::CAlertDlgInfoMsg(
		CWnd*		pNotifDIalog,
		CTransparentWindow*	pTransWindow,
		HWND		hWndParent,
		CWnd*		pParent,
		DhtmlGuiLocation	eGUIlocation,
		CString		sGUIlocationFilePath,
		long		lFreePosition,
		long		lCallUID,
		eCallType	callType):CAlertDlg(
 			  pNotifDIalog,
			  pTransWindow,
			  hWndParent,
			  pParent,
			  eGUIlocation,
			  sGUIlocationFilePath,
			  IDR_HTML_ALERT,
			  ALERT_INFO_MSG_HTML_FILENAME)
{
	m_lFreePosition = lFreePosition;
	m_eCallType = callType;
	m_lCallUID = lCallUID;
}

HRESULT CAlertDlgInfoMsg::OnCbkEventsHandlerClickedEvent(IHTMLElement *pElement)
{
TRY_CATCH

	VARIANT AttributeValue1;
	pElement->getAttribute( L"sEvent", 0, &AttributeValue1);
	CString sEvent = (CString)AttributeValue1;

	MapCallbacksAlertDlgIterator it = mapCallbacks.find(sEvent);

	if(it != mapCallbacks.end())
	{
		 Log.Add(_MESSAGE_,_T("Event found - eventID: '%s' "), sEvent );
		 (this->*((*it).second))(pElement);
	}
	else
	{
		 Log.Add(_MESSAGE_,_T("Warning - this eventID: '%s' not in the event map of CAlertDlgInfoMsg"), sEvent );
	}

CATCH_LOG(_T("CAlertDlgInfoMsg ::OnCbkEventsHandlerClickedEvent"))

	return S_OK;
}

HRESULT CAlertDlgInfoMsg::OnAlertPickUp(IHTMLElement *pElement)
{
TRY_CATCH
	
	if(m_pNotifDIalog != NULL)
		m_pNotifDIalog->PostMessage(WM_ALERTDLG_INFO_MSG_HELP, (WPARAM)m_lMid, (LPARAM)m_lCallUID);//TODO -user selction

CATCH_LOG(_T("CAlertDlgInfoMsg ::OnAlertPickUp"))

	return S_OK;
}

HRESULT CAlertDlgInfoMsg::OnAlertClose( IHTMLElement *pElement )
{
TRY_CATCH

	if(m_pNotifDIalog != NULL)
		m_pNotifDIalog->PostMessage(WM_ALERTDLG_INFO_MSG_CLOSE, 0, (LPARAM)m_lCallUID);//todo callid here

CATCH_LOG(_T("CAlertDlgInfoMsg::OnAlertClose"))

	return S_OK;
}

HRESULT CAlertDlgInfoMsg::OnReportIssue(IHTMLElement *pElement)
{
TRY_CATCH

	if(m_pNotifDIalog != NULL)
		m_pNotifDIalog->PostMessage(WM_ALERTDLG_INFO_MSG_CLOSE, 0, (LPARAM)m_lCallUID);//todo callid here

CATCH_LOG(_T("CAlertDlgInfoMsg::OnReportIssue"))
	return S_OK;
}

HRESULT CAlertDlgInfoMsg::OnReadMore(IHTMLElement *pElement)
{
TRY_CATCH

	if(m_pNotifDIalog != NULL)
		m_pNotifDIalog->PostMessage(WM_ALERTDLG_INFO_MSG_HELP, 0, (LPARAM)m_lCallUID);//todo callid here

CATCH_LOG(_T("CAlertDlgInfoMsg::OnReadMore"))
	return S_OK;
}

LRESULT CAlertDlgInfoMsg::OnAlertFadeCompleted( WPARAM wParam, LPARAM lParam )
{
TRY_CATCH

	if(m_pNotifDIalog != NULL)
		m_pNotifDIalog->PostMessage(WM_ALERTDLG_INFO_MSG_CLOSE, 0, (LPARAM)m_lCallUID);

CATCH_LOG(_T("CAlertDlgInfoMsg::OnAlertFadeCompleted"))
	return S_OK;
}

HRESULT CAlertDlgInfoMsg::CalcluateAlertPosition(RECT* pRc,WINDOWPOS* pPosc)
{
TRY_CATCH

	CRect	m_screen_rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &m_screen_rect, 0);

	int	m_nWidth =  ALERT_INFO_MSG_DIALOG_WIDTH;
	int	m_nHeight = ALERT_INFO_MSG_DIALOG_HEIGHT; 

	//int x = m_screen_rect.right - m_nWidth - m_pTransWindow->VERTICAL_BORDER_WIDTH;
	//int y = m_screen_rect.bottom - m_nHeight - m_pTransWindow->HORIZONTAL_BORDER_WIDTH;

	int x = m_screen_rect.right -  m_nWidth - m_pTransWindow->VERTICAL_BORDER_WIDTH;
	int y = m_screen_rect.bottom - (m_nHeight + ALERT_DIALOG_OFFSET)*m_lFreePosition;

	int cx = m_nWidth;
	int cy = m_nHeight;

	SetWindowPos( &CWnd::wndTopMost, 
		x,
		y, 
		cx, 
		cy, //For Dialog Border needed TODO
		SWP_NOZORDER | SWP_SHOWWINDOW);

	RECT rc;

	rc.left = x;
	rc.top = y;
	rc.right = x + cx;
	rc.bottom= y + cy;

	WINDOWPOS posc;

	posc.hwnd = m_hWndParent;
	posc.hwndInsertAfter = this->m_hWnd;
	posc.x = x;
	posc.y = y;
	posc.cx = cx;
	posc.cy = cy;
	posc.flags = SWP_SHOWWINDOW;

	*pRc = rc;
	*pPosc = posc;

CATCH_LOG(_T("CAlertDlgInfoMsg::CalcluateAlertPosition"))
	return S_OK;
}

HRESULT CAlertDlgInfoMsg::OnPageLoaded()
{
TRY_CATCH

	m_cHTMLInterface.INTERFACE_UpdateData(
		DHTML_INTERFACE_MSG_ALERT_INFO_MSG, m_sCalls);//todo

	SetTimer(CLOSE_ALERT_TIMER, CLOSE_ALERT_TIMER_WAIT_TIMEOUT, NULL );

CATCH_LOG(_T("CAlertDlgInfoMsg::OnPageLoaded"))
	return S_OK;
}

HRESULT CAlertDlgInfoMsg::UpdateAlertText(CString sNewAlertText)
{
TRY_CATCH

	m_cHTMLInterface.INTERFACE_UpdateData(
		DHTML_INTERFACE_MSG_ALERT_INFO_MSG, _T(""), sNewAlertText);

CATCH_LOG(_T("CAlertDlgInfoMsg::OnPageLoaded"))
	return S_OK;
}

void CAlertDlgInfoMsg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	Log.Add(_WARNING_,_T("OnGetMinMaxInfo"));
}

void CAlertDlgInfoMsg::OnTimer(UINT nIDEvent)
{
TRY_CATCH

	switch(nIDEvent)
	{
	//	alert may be closed by server side after 45 seconds
	//	client will close alert after 60 seconds if server for some reason did not send close alert
	case CLOSE_ALERT_TIMER: 
		Log.Add(_MESSAGE_,_T("CAlertDlgNewCall::OnTimer CLOSE_ALERT_TIMER %d"),(LPARAM)m_lCallUID);
		//DestroyWindow();//todo anatoly - close also parent window
		KillTimer(CLOSE_ALERT_TIMER);
		if( m_pNotifDIalog != NULL )
			m_pNotifDIalog->PostMessage(WM_ALERTDLG_INFO_MSG_CLOSE, 0, (LPARAM)m_lCallUID); 
		return;
	default:
		break;
	}
		
	CAlertDlg::OnTimer(nIDEvent);

CATCH_LOG(_T("CAlertDlgInfoMsg::OnTimer"))
}
