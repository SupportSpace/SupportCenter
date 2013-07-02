#include "StdAfx.h"
#include "AlertDlgNewCall.h"

#include <AidLib/Logging/cLog.h> 
#include "AidLib/CException/CException.h"

#define DHTML_INTERFACE_ALERT_CALL		_T("shuttle_alert_call")
#define DHTML_INTERFACE_ALERT_CONSULT	_T("shuttle_alert_consult")

#define ALERT_NEW_CALL_HTML_FILENAME	_T("%s\\alert.html")

#define CLOSE_ALERT_TIMER					0x150	
#define CLOSE_ALERT_TIMER_WAIT_TIMEOUT		45000	//one minute to give 15 seconds server side to send close

//
//	define pointer to Callback function of the class CAlertDlgNewCall
//
typedef HRESULT (CAlertDlgNewCall::*pfnCallbackAlertDlgNewCall)(IHTMLElement *pElement);
//
//	define pair used for mapping string and callbacks
//
typedef std::pair<CString, pfnCallbackAlertDlgNewCall> DHTML_CUSTOM_EVENT_ALERT_DLG;
//
//	init pairs map of calbacks
//
static const DHTML_CUSTOM_EVENT_ALERT_DLG callBacksPairsAlertDlgNewCall[] =
{
	DHTML_CUSTOM_EVENT_ALERT_DLG(_T("pickup"),CAlertDlgNewCall::OnAlertPickUp),
	DHTML_CUSTOM_EVENT_ALERT_DLG(_T("startConsultAlert"),CAlertDlgNewCall::OnAlertPickUp),
	DHTML_CUSTOM_EVENT_ALERT_DLG(_T("expandAlert"),CAlertDlgNewCall::OnAlertExpand),
	DHTML_CUSTOM_EVENT_ALERT_DLG(_T("pageLoaded"),CAlertDlgNewCall::OnCallBackPageLoaded),
	DHTML_CUSTOM_EVENT_ALERT_DLG(_T("closeAlert"),CAlertDlgNewCall::OnAlertClose),
	DHTML_CUSTOM_EVENT_ALERT_DLG(_T("EnterAlert"),CAlertDlg::OnAlertEnter),
	DHTML_CUSTOM_EVENT_ALERT_DLG(_T("LeaveAlert"),CAlertDlg::OnAlertLeave),
};
//
//	define map of callbacks
//
const static std::map<CString, pfnCallbackAlertDlgNewCall> mapCallbacks(callBacksPairsAlertDlgNewCall, callBacksPairsAlertDlgNewCall + sizeof(callBacksPairsAlertDlgNewCall)/sizeof(callBacksPairsAlertDlgNewCall[0]));
typedef std::map<CString, pfnCallbackAlertDlgNewCall>::const_iterator MapCallbacksAlertDlgIterator;


BEGIN_MESSAGE_MAP(CAlertDlgNewCall, CAlertDlg)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CAlertDlgNewCall::~CAlertDlgNewCall(void)
{
}

CAlertDlgNewCall::CAlertDlgNewCall(
		CWnd*		pNotifDIalog,
		CTransparentWindow*	pTransWindow,
		HWND		hWndParent,
		CWnd*		pParent,
		DhtmlGuiLocation	eGUIlocation,
		CString		sGUIlocationFilePath,
		long		lFreePosition,
		long		lCallUID,
		CString		sCalls,
		eCallType	callType):CAlertDlg(
 			  pNotifDIalog,
			  pTransWindow,
			  hWndParent,
			  pParent,
			  eGUIlocation,
			  sGUIlocationFilePath,
			  IDR_HTML_ALERT,
			  ALERT_NEW_CALL_HTML_FILENAME)
{
	m_lFreePosition = lFreePosition;
	m_lCallUID = lCallUID;
	m_sCalls = sCalls;
	m_eCallType = callType;
}

HRESULT CAlertDlgNewCall::OnCbkEventsHandlerClickedEvent(IHTMLElement *pElement)
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
		 Log.Add(_MESSAGE_,_T("Warning - this eventID: '%s' not in the event map of CAlertDlgNewCall"), sEvent );
	}

CATCH_LOG(_T("CAlertDlgNewCall ::OnCbkEventsHandlerClickedEvent"))

	return S_OK;
}

HRESULT CAlertDlgNewCall::OnAlertPickUp(IHTMLElement *pElement)
{
TRY_CATCH

	long		lCallUID = 0;
	long		iWorkflowID = 0;
	VARIANT		tmpCallUID;
	CString     sDisplayName;
	
	pElement->getAttribute( L"iCallUID", 0, &tmpCallUID);
	lCallUID	= tmpCallUID.lVal;
	Log.Add(_MESSAGE_, _T("CAlertDlgNewCall::OnAlertPickUp Attribute iCallUID %d"), lCallUID);

	pElement->getAttribute(L"sDisplayUserName", 0, &tmpCallUID);
	sDisplayName = (CString)tmpCallUID;

	if(m_eCallType==ConsultCall)
	{
		pElement->getAttribute( L"iWorkflowID", 0, &tmpCallUID);
		iWorkflowID	= tmpCallUID.lVal;
		Log.Add(_MESSAGE_, _T("CAlertDlgNewCall::OnAlertPickUp Attribute iWorkflowID %d"), iWorkflowID);
	}
	
	if(m_pNotifDIalog != NULL)
	{
		CCall*	pCall = new CCall();
		if(pCall!=NULL)
		{
			pCall->setCallType(m_eCallType);
			pCall->setUid(m_lCallUID);		
			pCall->setWorkflowID(iWorkflowID);	
			pCall->setCustomerDislpayName(sDisplayName);	
			m_pNotifDIalog->PostMessage(WM_ALERTDLG_NEW_CALL_PICKUP, 0, (LPARAM)pCall);
		}
	}

CATCH_LOG(_T("CAlertDlgNewCall ::OnAlertPickUp"))

	return S_OK;
}

HRESULT CAlertDlgNewCall::OnAlertExpand( IHTMLElement *pElement )
{
TRY_CATCH

	long		lCallUID;
	VARIANT		tmpCallUID;
	pElement->getAttribute( L"iCallUID", 0, &tmpCallUID);
	lCallUID	= tmpCallUID.lVal;
	Log.Add(_MESSAGE_, _T("CAlertDlgNewCall::OnAlertExpand Attribute iCallUID %d"), lCallUID);

	if(m_pNotifDIalog != NULL)
		m_pNotifDIalog->PostMessage(WM_ALERTDLG_NEW_CALL_EXPAND, 0, (LPARAM)m_lCallUID);

CATCH_LOG(_T("CAlertDlgNewCall::OnAlertExpand"))

	return S_OK;
}

HRESULT CAlertDlgNewCall::OnAlertClose(IHTMLElement *pElement)
{
TRY_CATCH

	long		lCallUID;
	VARIANT		tmpCallUID;
	pElement->getAttribute( L"iCallUID", 0, &tmpCallUID);
	lCallUID	= tmpCallUID.lVal;
	Log.Add(_MESSAGE_, _T("CAlertDlg::OnAlertClose Attribute iCallUID %d"), lCallUID);

	if(m_pNotifDIalog != NULL)
		m_pNotifDIalog->PostMessage(WM_ALERTDLG_NEW_CALL_CLOSE, 0, (LPARAM)m_lCallUID);

CATCH_LOG(_T("CAlertDlg::OnAlertClose"))

	return S_OK;
}

LRESULT CAlertDlgNewCall ::OnAlertFadeCompleted(WPARAM wParam, LPARAM lParam)
{
TRY_CATCH

	if(m_pNotifDIalog != NULL)
		m_pNotifDIalog->PostMessage(WM_ALERTDLG_NEW_CALL_CLOSE, 0, (LPARAM)m_lCallUID);

CATCH_LOG(_T("CAlertDlg::OnAlertFadeCompleted"))
	return S_OK;
}

HRESULT CAlertDlgNewCall::CalcluateAlertPosition(RECT* pRc,WINDOWPOS* pPosc)
{
TRY_CATCH

	CRect	m_screen_rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &m_screen_rect, 0);

	int	m_nWidth =  ALERT_NEW_DIALOG_WIDTH;
	int	m_nHeight = ALERT_NEW_DIALOG_HEIGHT;

	//int x = m_screen_rect.right - m_nWidth - m_pTransWindow->VERTICAL_BORDER_WIDTH;
	//int y = m_screen_rect.bottom - m_nHeight - m_pTransWindow->HORIZONTAL_BORDER_WIDTH;

	int x = m_screen_rect.right -  m_nWidth - m_pTransWindow->VERTICAL_BORDER_WIDTH;
	int y = m_screen_rect.bottom - (m_nHeight + ALERT_DIALOG_OFFSET)*m_lFreePosition;

	int cx = m_nWidth;
	int cy = m_nHeight;
/*
	SetWindowPos( &CWnd::wndTopMost, 
		x,
		y, 
		cx, 
		cy, //For Dialog Border needed TODO
		SWP_NOZORDER | SWP_SHOWWINDOW);//SWP_SHOWWINDOW
*/
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

CATCH_LOG(_T("CAlertDlg::CalcluateAlertPosition"))
	return S_OK;
}

HRESULT CAlertDlgNewCall::OnCallBackPageLoaded(IHTMLElement *pElement)
{
TRY_CATCH

	if(this->m_spHtmlDoc== NULL)
	{
		Log.Add(_ERROR_,_T("CAlertDlgNewCall::OnCallBackPageLoaded and this->m_spHtmlDoc== NULL!!!"));

		if(this->m_pBrowserApp==NULL)
		{
			Log.Add(_ERROR_,_T("CAlertDlgNewCall::OnCallBackPageLoaded and this>m_pBrowserApp==NULL!!!"));
			//this>m_pBrowserApp->get_Document();
		}
		
		// TODO - can not reproduce problem here on my PC. Try IE6, try Lynda's PC
		// tryied 30000 alerts on my pc - no problem detected 
		AfxMessageBox("Internal error: alert not displayed. Please, close application and open again.");
	}
	else
	{
		Log.Add(_MESSAGE_,_T("CAlertDlgNewCall::OnCallBackPageLoaded and this->m_spHtmlDoc!=NULL"));	
	}

CATCH_LOG(_T("CAlertDlgNewCall::OnCallBackPageLoaded"))
	return S_OK;
}

HRESULT CAlertDlgNewCall::OnPageLoaded()
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("CAlertDlgNewCall::OnPageLoaded()"));

	CString sCallId;

	sCallId.FormatMessage(_T("%1!d!"), m_lCallUID); 

	switch(m_eCallType)//todo - OOD it is better avoid switch to make class CAlertDlgNewCall, CAlertDlgDierctCall,etc
	{
	case CustomerDirectCall:
		m_cHTMLInterface.INTERFACE_UpdateData(DHTML_INTERFACE_ALERT_CALL, m_sCalls, sCallId, _T(""));
		break;
	case ConsultCall:
		m_cHTMLInterface.INTERFACE_UpdateData(DHTML_INTERFACE_ALERT_CONSULT, m_sCalls, sCallId, _T(""));
		break;
	default:
		break;
	}

	SetTimer(CLOSE_ALERT_TIMER, CLOSE_ALERT_TIMER_WAIT_TIMEOUT, NULL );
		
CATCH_LOG(_T("CAlertDlg::OnPageLoaded"))
	return S_OK;
}

void CAlertDlgNewCall::OnTimer(UINT nIDEvent)
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
			m_pNotifDIalog->PostMessage(WM_ALERTDLG_CLOSE, 0, (LPARAM)m_lCallUID); 
		return;
	default:
		break;
	}
		
	CAlertDlg::OnTimer(nIDEvent);

CATCH_LOG(_T("CAlertDlgNewCall::OnTimer"))
}

BOOL	CAlertDlgNewCall::IsFreePositionAvailble(int iFreePosition)
{
TRY_CATCH
	//
	//	check that there is a place for next alert in new position on the screen
	//	Seee requirement here http://srv-dev/jira/browse/STL-239
	//
	CRect	m_screen_rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &m_screen_rect, 0);
	
	if( (iFreePosition)*(ALERT_NEW_DIALOG_HEIGHT + ALERT_NEW_DIALOG_OFFSET) > m_screen_rect.bottom )
	{
		Log.Add(_WARNING_, _T("Due screen resolution %dx%d. Alert number %d can not be displayed"),m_screen_rect.right, m_screen_rect.bottom, iFreePosition);	
		return FALSE; 
	}
	else
	{
		return TRUE;
	}

CATCH_LOG(_T("CAlertDlgNewCall::IsPositionAvailble"))
	return FALSE;
}