// DHtmlDialogEx.h : header file
//
//===========================================================================
// SupportSpace ltd. @{SRCH}
//								CDHtmlDialogEx
//
//---------------------------------------------------------------------------
// DESCRIPTION: @{HDES}
// -----------
// CDHtmlDialogEx :	this class provides interface to the Internet Explorer ocx
//					it also disable some features of IE that main not appear in the 
//					Windows GUI application (like Refresh on F5 and much more )
//					This also will give useful methods like make Window Transparent 
//					and call JavaScript funciton of the loaded page
//---------------------------------------------------------------------------
// CHANGES LOG: @{HREV}
// -----------
// Revision: 01.00
// By      : Anatoly Gutnick
// Date    : 10/29/2006 05:21:10 PM
// Comments: First Issue
//===========================================================================
#pragma once

#include "TrasparentWindow.h"
#include "LayeredWindowHelperST.h"
#include "HTMLInterface.h"
#include "exdispid.h" //todo required to fix IE disable 

//define the macro for mapping one event from all HTML tags
#define DHTML_EVENT_TAG_ALL(dispid, memberFxn)\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("a"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("abbr"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("acronym"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("address"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("applet"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("area"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("b"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("base"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("basefont"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("bdo"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("big"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("blockquote"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("body"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("br"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("button"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("caption"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("center"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("cite"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("code"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("col"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("colgroup"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("dd"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("del"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("dir"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("div"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("dfn"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("dl"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("dt"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("em"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("fieldset"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("font"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("form"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("frame"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("frameset"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("h1"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("h2"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("h3"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("h4"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("head"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("hr"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("html"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("i"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("iframe"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("img"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("input"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("ins"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("isindex"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("kbd"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("label"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("legend"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("li"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("link"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("map"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("menu"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("meta"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("noframes"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("noscript"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("object"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("ol"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("optgroup"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("option"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("p"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("param"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("pre"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("q"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("s"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("samp"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("script"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("select"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("small"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("span"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("strike"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("strong"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("style"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("sub"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("sup"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("table"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("tbody"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("td"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("textarea"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("tfoot"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("th"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("thead"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("title"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("tr"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("tt"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("u"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("ul"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("var"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\
{ DHTMLEVENTMAPENTRY_TAG, dispid, _T("xmp"), (DHEVTFUNCCONTROL) (DHEVTFUNC) theClass::memberFxn },\

// CDHtmlDialogEx dialog
class CDHtmlDialogEx : public CDHtmlDialog
{
// Construction
public:
	CDHtmlDialogEx(UINT nIDTemplate, UINT nHtmlResID = 0, CWnd* pParent = NULL);	// standard constructor
	virtual ~CDHtmlDialogEx(){};	// standard destructor constructor

// Dialog Data
	enum { IDD = 0, IDH = 0 };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	void OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl);
	//override this functions to avoid issues with IE features in our application
	void OnBeforeNavigate2(LPDISPATCH pDisp, VARIANT FAR* URL, VARIANT FAR* Flags, VARIANT FAR* TargetFrameName, VARIANT FAR* PostData, VARIANT FAR* Headers, BOOL FAR* Cancel);
	//override this functions to avoid issues with IE features in our application
	void OnNavigateComplete2(LPDISPATCH pDisp, VARIANT FAR* URL);
	//override this functions to avoid issues with IE features in our application
	void OnNewWindow2(LPDISPATCH FAR* ppDisp, BOOL FAR* Cancel);
	// Ban some keyboard accelerators in browser control and some app level accelerators
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	HRESULT STDMETHODCALLTYPE ShowContextMenu(DWORD /*dwID*/, POINT *ppt, IUnknown* /*pcmdtReserved*/, IDispatch* /*pdispReserved*/);

	HRESULT OnHtmlSelectStart( IHTMLElement *pElement );
	HRESULT OnHtmlDragStart( IHTMLElement *pElement );
	HRESULT OnHtmlContextMenu( IHTMLElement *pElement );

	bool MakeWindowTransparent( int nPercent );

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg int  OnCreate(LPCREATESTRUCT lpRes);
	
	//	parent-border window may notify this window about moving 
	long	OnUserBorderMove( WPARAM wParam, LPARAM lParam );
	//	we have to notify parent-border window about moving
	void	OnMoving(UINT uint,LPRECT rect);

	LRESULT OnNcHitTest(CPoint point);
	
	BOOL	OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
	DECLARE_EVENTSINK_MAP() //todo needed
public:
	BOOL				IsPageLoaded()
	{
		if( m_webPage.GetHtmlDocument()!=NULL) return TRUE;
		else return FALSE;
	}

	void				EnableF5(BOOL bEnable);
	void				EnableCtrlF4(BOOL bEnable);
	void				EnableWorkbenchMode(BOOL bEnable);

	time_t					m_WorkbenchStartTime;

	CString					m_sDestUrl;
	CHTMLInterface			m_cHTMLInterface;
	CWebPage				m_webPage;
	CLayeredWindowHelperST	m_layeredWnd;

	CTransparentWindow*		m_pTransWindow;		//border window implementation
	HWND					m_hWndParent;		//border window implementation
	BOOL				    m_bEnableF5;
	BOOL					m_bEnableCtrlF4;
	BOOL					m_bEnableWorkbenchMode;
};