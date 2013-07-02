#include "StdAfx.h"
#include "AlertDlgMissedCalls.h"

#include <AidLib/Logging/cLog.h> 
#include "AidLib/CException/CException.h"

//#define DHTML_INTERFACE_MSG_NEW_REQUEST			_T("stargate_new_request")
#define DHTML_INTERFACE_MSG_ALERT_MISSED_CALLS		_T("shuttle_alert_missed_calls")
#define ALERT_UPDATE_CALL_HTML_FILENAME				_T("%s\\alert.html")

#define ALERT_MISSED_CALLS_DIALOG_WIDTH				213//219
#define ALERT_MISSED_CALLS_DIALOG_HEIGHT			150//130
#define ALERT_MISSED_CALLS_DIALOG_OFFSET			25//offset between alerts 

//
//	define pointer to Callback function of the class CAlertDlgNewCall
//
typedef HRESULT (CAlertDlgMissedCalls::*pfnCallbackAlertDlgNewCall)(IHTMLElement *pElement);
//
//	define pair used for mapping string and callbacks
//
typedef std::pair<CString, pfnCallbackAlertDlgNewCall> DHTML_CUSTOM_EVENT_ALERT_DLG;
//
//	init pairs map of calbacks
//
static const DHTML_CUSTOM_EVENT_ALERT_DLG callBacksPairsAlertDlgNewCall[] =
{
	DHTML_CUSTOM_EVENT_ALERT_DLG(_T("onclick"),CAlertDlgMissedCalls::OnAlertPickUp),
	DHTML_CUSTOM_EVENT_ALERT_DLG(_T("pickup"),CAlertDlgMissedCalls::OnAlertPickUp),
	DHTML_CUSTOM_EVENT_ALERT_DLG(_T("closeAlert"),CAlertDlgMissedCalls::OnAlertClose),
	DHTML_CUSTOM_EVENT_ALERT_DLG(_T("EnterAlert"),CAlertDlgMissedCalls::OnAlertEnter),
	DHTML_CUSTOM_EVENT_ALERT_DLG(_T("LeaveAlert"),CAlertDlgMissedCalls::OnAlertLeave),
};
//
//	define map of callbacks
//
const static std::map<CString, pfnCallbackAlertDlgNewCall> mapCallbacks(callBacksPairsAlertDlgNewCall, callBacksPairsAlertDlgNewCall + sizeof(callBacksPairsAlertDlgNewCall)/sizeof(callBacksPairsAlertDlgNewCall[0]));
typedef std::map<CString, pfnCallbackAlertDlgNewCall>::const_iterator MapCallbacksAlertDlgIterator;

CAlertDlgMissedCalls::~CAlertDlgMissedCalls(void)
{
}

CAlertDlgMissedCalls::CAlertDlgMissedCalls(
		CWnd*		pNotifDIalog,
		CTransparentWindow*	pTransWindow,
		HWND		hWndParent,
		CWnd*		pParent,
		DhtmlGuiLocation	eGUIlocation,
		CString		sGUIlocationFilePath,
		long		lFreePosition):CAlertDlg(
 			  pNotifDIalog,
			  pTransWindow,
			  hWndParent,
			  pParent,
			  eGUIlocation,
			  sGUIlocationFilePath,
			  IDR_HTML_ALERT,
			  ALERT_UPDATE_CALL_HTML_FILENAME)
{
	m_lFreePosition = lFreePosition;
}

HRESULT CAlertDlgMissedCalls::OnCbkEventsHandlerClickedEvent(IHTMLElement *pElement)
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
		 Log.Add(_MESSAGE_,_T("Warning - this eventID: '%s' not in the event map of CAlertDlgMissedCalls"), sEvent );
	}

CATCH_LOG(_T("CAlertDlgMissedCalls ::OnCbkEventsHandlerClickedEvent"))

	return S_OK;
}

HRESULT CAlertDlgMissedCalls::OnAlertPickUp(IHTMLElement *pElement)
{
TRY_CATCH
	
	if(m_pNotifDIalog != NULL)
		m_pNotifDIalog->PostMessage(WM_ALERTDLG_MISSEDCALLS_PICKUP, 0, (LPARAM)0);//TODO -user selction

CATCH_LOG(_T("CAlertDlgMissedCalls ::OnAlertPickUp"))

	return S_OK;
}

HRESULT CAlertDlgMissedCalls::OnAlertClose( IHTMLElement *pElement )
{
TRY_CATCH

	if(m_pNotifDIalog != NULL)
		m_pNotifDIalog->PostMessage(WM_ALERTDLG_MISSEDCALLS_CLOSE, 0, (LPARAM)0);

CATCH_LOG(_T("CAlertDlgMissedCalls::OnAlertClose"))

	return S_OK;
}

LRESULT CAlertDlgMissedCalls ::OnAlertFadeCompleted( WPARAM wParam, LPARAM lParam )
{
TRY_CATCH

	if(m_pNotifDIalog != NULL)
		m_pNotifDIalog->PostMessage(WM_ALERTDLG_MISSEDCALLS_CLOSE, 0, (LPARAM)0);

CATCH_LOG(_T("CAlertDlgMissedCalls::OnAlertFadeCompleted"))
	return S_OK;
}

HRESULT CAlertDlgMissedCalls::CalcluateAlertPosition(RECT* pRc,WINDOWPOS* pPosc)
{
TRY_CATCH

	CRect	m_screen_rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &m_screen_rect, 0);

	int	m_nWidth =  ALERT_MISSED_CALLS_DIALOG_WIDTH;
	int	m_nHeight = ALERT_MISSED_CALLS_DIALOG_HEIGHT; 

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

CATCH_LOG(_T("CAlertDlgMissedCalls::CalcluateAlertPosition"))
	return S_OK;
}

HRESULT CAlertDlgMissedCalls::OnPageLoaded()
{
TRY_CATCH

	m_cHTMLInterface.INTERFACE_UpdateData(
		DHTML_INTERFACE_MSG_ALERT_MISSED_CALLS, _T(""), (CString)MAKEINTRESOURCE(IDS_MISSEDCALLS_ALERT_TEXT));

CATCH_LOG(_T("CAlertDlgMissedCalls::OnPageLoaded"))
	return S_OK;
}

HRESULT CAlertDlgMissedCalls::UpdateAlertText(CString sNewAlertText)
{
TRY_CATCH

	m_cHTMLInterface.INTERFACE_UpdateData(
		DHTML_INTERFACE_MSG_ALERT_MISSED_CALLS, _T(""), sNewAlertText);

CATCH_LOG(_T("CAlertDlgMissedCalls::OnPageLoaded"))
	return S_OK;
}
