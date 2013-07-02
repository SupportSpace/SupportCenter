//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CWidgetToolBar.cpp
///
///  Implements CWidgetToolBar class
///  
///  
///  @author Alexander Novak @date 28.11.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CWidgetToolBar.h"
#include "CCommandManager.h"
#include "resource.h"
//========================================================================================================

#define WTB_MAX_MENU_ITEM_LENGTH		50

// CWidgetToolBar [BEGIN] ////////////////////////////////////////////////////////////////////////////////

LRESULT CWidgetToolBar::OnButtonClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
TRY_CATCH

	DispatchCommand(EWidgetCommand(wID));

	return TRUE;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetToolBar::OnButtonDropDown(int wParam, LPNMHDR notifyHeader, BOOL& bHandled)
{
TRY_CATCH

	LPNMTOOLBAR lptb = (LPNMTOOLBAR)notifyHeader;
	RECT rc;
	GetRect(lptb->iItem,&rc);

	MapWindowPoints(HWND_DESKTOP,(LPPOINT)&rc,2);

	TPMPARAMS tpm;
	tpm.cbSize		= sizeof(TPMPARAMS);
	tpm.rcExclude	= rc;

	TrackPopupMenuEx(m_filterMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_VERTICAL,rc.left,rc.bottom,m_hWnd,&tpm);

	return TBDDRET_DEFAULT;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetToolBar::OnCustomDraw(int wParam, LPNMHDR notifyHeader, BOOL& bHandled)
{
TRY_CATCH

	LPNMCUSTOMDRAW customDraw = (LPNMCUSTOMDRAW)notifyHeader;

	switch ( customDraw->dwDrawStage )
	{
		case CDDS_PREPAINT:
			return CDRF_NOTIFYITEMDRAW;

		case CDDS_ITEMPREPAINT:
		{
			int resId = IDR_TB_DELETE_NORMAL_29x29 + static_cast<int>(customDraw->lItemlParam) * 4;

			if ( customDraw->uItemState & CDIS_DEFAULT )
				resId += IDR_TB_DELETE_NORMAL_29x29 - IDR_TB_DELETE_NORMAL_29x29;
			
			else if ( customDraw->uItemState & CDIS_DISABLED )
				resId += IDR_TB_DELETE_DISABLE_29x29 - IDR_TB_DELETE_NORMAL_29x29;
			
			else if ( customDraw->uItemState & CDIS_SELECTED )
				resId += IDR_TB_DELETE_PRESSED_29x29 - IDR_TB_DELETE_NORMAL_29x29;
			
			else if ( customDraw->uItemState & CDIS_HOT )
				resId += IDR_TB_DELETE_MOUSEOVER_29x29 - IDR_TB_DELETE_NORMAL_29x29;
				
			CImage* img;
			m_skinsImageList->ImageFromRes(&img,1,&resId);
			if ( img )
				img->Draw(customDraw->hdc,customDraw->rc);

			return CDRF_SKIPDEFAULT;
		}
	}
	return CDRF_DODEFAULT;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetToolBar::OnGetToolTip(int wParam, LPNMHDR notifyHeader, BOOL& bHandled)
{
TRY_CATCH

	LPNMTTDISPINFO dispInfo = (LPNMTTDISPINFO)notifyHeader;
	dispInfo->hinst		= NULL;
	dispInfo->lpszText	= const_cast<TCHAR*>(m_toolTips[EWidgetCommand(dispInfo->hdr.idFrom)].c_str());

	return FALSE;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetToolBar::OnMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	LPMEASUREITEMSTRUCT measureItem = (LPMEASUREITEMSTRUCT)lParam;

    measureItem->itemWidth	= m_maxItemWidth;
    measureItem->itemHeight	= m_maxItemHeight;

	return TRUE;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetToolBar::OnDrawMenuItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT)lParam;
	
	HGDIOBJ oldFont = SelectObject(lpDrawItem->hDC,m_hFont);

	if ((lpDrawItem->itemState & ODS_SELECTED) && (lpDrawItem->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
	{
		HGDIOBJ oldBrush = SelectObject(lpDrawItem->hDC,m_bkBrush);
		HGDIOBJ oldPen = SelectObject(lpDrawItem->hDC,m_highlightPen);
		
		Rectangle(lpDrawItem->hDC,lpDrawItem->rcItem.left,lpDrawItem->rcItem.top,lpDrawItem->rcItem.right,lpDrawItem->rcItem.bottom);
		
		SelectObject(lpDrawItem->hDC,oldBrush);
		SelectObject(lpDrawItem->hDC,oldPen);
	}
	else
		FillRect(lpDrawItem->hDC, &lpDrawItem->rcItem, m_bkBrush);

	int prevBkMode = SetBkMode(lpDrawItem->hDC,TRANSPARENT);
	COLORREF prevTextColor = SetTextColor(lpDrawItem->hDC,FontColor1);

	RECT rc = lpDrawItem->rcItem;
	rc.left		+= m_marginSpace;
	rc.right	= rc.left + m_checkMarkWidth;
	rc.top		+= (rc.bottom - rc.top)/2 - m_checkMarkHeignt/2;
	rc.bottom	= rc.top + m_checkMarkHeignt;
	
	bool checked = false;
	switch ( lpDrawItem->itemID )
	{
		case ID__ATR_HIDDEN:
			checked = !(m_disabledAttributes & FILE_ATTRIBUTE_HIDDEN);
			break;
		case ID__ATR_SYSTEM:
			checked = !(m_disabledAttributes & FILE_ATTRIBUTE_SYSTEM);
			break;
		case ID__ATR_READONLY:
			checked = !(m_disabledAttributes & FILE_ATTRIBUTE_READONLY);
			break;
	}
	int resId = checked ? IDR_CHB_CHECKED_REGULAR : IDR_CHB_UNCHECKED_REGULAR;
	CImage* img;
	m_skinsImageList->ImageFromRes(&img,1,&resId);
	img->Draw(lpDrawItem->hDC,rc);

	rc = lpDrawItem->rcItem;
	rc.left		+= m_checkMarkWidth + m_marginSpace*2;
	rc.right	-= m_marginSpace;

	DrawText(	lpDrawItem->hDC,
				m_filterMenuText[lpDrawItem->itemID].c_str(),
				(int)m_filterMenuText[lpDrawItem->itemID].size(),
				&rc,
				DT_TOP|DT_SINGLELINE|DT_LEFT|DT_VCENTER);

	SetTextColor(lpDrawItem->hDC,prevTextColor);
	SetBkMode(lpDrawItem->hDC,prevBkMode);
	
	SelectObject(lpDrawItem->hDC,oldFont);

	return TRUE;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetToolBar::OnMenuCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	// Just reverse attributes
	switch ( LOWORD(wParam) )
	{
		case ID__ATR_HIDDEN:
			m_disabledAttributes ^= FILE_ATTRIBUTE_HIDDEN;
			break;
		case ID__ATR_SYSTEM:
			m_disabledAttributes ^= FILE_ATTRIBUTE_SYSTEM;
			break;
		case ID__ATR_READONLY:
			m_disabledAttributes ^= FILE_ATTRIBUTE_READONLY;
			break;
		default:
			return 0;
	}
	DispatchCommand(cmd_FilterChanged);

	return 0;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CWidgetToolBar::CWidgetToolBar(HWND hParentWnd, boost::shared_ptr<CAbstractCommandManager> commandManager)
	:	CSkinnedElement(commandManager.get()),
		CCommandProxy(commandManager),
		m_disabledAttributes(0)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CWidgetToolBar::CWidgetToolBar"));

	Create(	hParentWnd,
			0,
			NULL,
			WS_CHILD|WS_VISIBLE|CCS_NOPARENTALIGN|CCS_NODIVIDER|TBSTYLE_FLAT|TBSTYLE_TOOLTIPS);

	SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS);

	SetButtonStructSize();
	
	m_buttonInfo[0].iBitmap		= 0;
	m_buttonInfo[0].idCommand	= cmd_DeleteFile;
	m_buttonInfo[0].fsState		= TBSTATE_ENABLED;
	m_buttonInfo[0].fsStyle		= BTNS_BUTTON;
	m_buttonInfo[0].dwData		= 0;
	m_buttonInfo[0].iString		= 0;

	m_buttonInfo[1].iBitmap		= 0;
	m_buttonInfo[1].idCommand	= cmd_RenameFile;
	m_buttonInfo[1].fsState		= TBSTATE_ENABLED;
	m_buttonInfo[1].fsStyle		= BTNS_BUTTON;
	m_buttonInfo[1].dwData		= 1;
	m_buttonInfo[1].iString		= 0;

	m_buttonInfo[2].iBitmap		= 0;
	m_buttonInfo[2].idCommand	= cmd_MoveFile;
	m_buttonInfo[2].fsState		= TBSTATE_ENABLED;
	m_buttonInfo[2].fsStyle		= BTNS_BUTTON;
	m_buttonInfo[2].dwData		= 2;
	m_buttonInfo[2].iString		= 0;

	m_buttonInfo[3].iBitmap		= 0;
	m_buttonInfo[3].idCommand	= cmd_CopyFile;
	m_buttonInfo[3].fsState		= TBSTATE_ENABLED;
	m_buttonInfo[3].fsStyle		= BTNS_BUTTON;
	m_buttonInfo[3].dwData		= 3;
	m_buttonInfo[3].iString		= 0;

	m_buttonInfo[4].iBitmap		= 0;
	m_buttonInfo[4].idCommand	= cmd_CreateDirectory;
	m_buttonInfo[4].fsState		= TBSTATE_ENABLED;
	m_buttonInfo[4].fsStyle		= BTNS_BUTTON;
	m_buttonInfo[4].dwData		= 4;
	m_buttonInfo[4].iString		= 0;

	//m_buttonInfo[5].iBitmap		= 0;
	//m_buttonInfo[5].idCommand	= cmd_FilterChanged;
	//m_buttonInfo[5].fsState		= TBSTATE_ENABLED;
	//m_buttonInfo[5].fsStyle		= BTNS_WHOLEDROPDOWN;
	//m_buttonInfo[5].dwData		= 5;
	//m_buttonInfo[5].iString		= 0;

	m_toolTips[cmd_DeleteFile]		= _T("Delete selected file or folder");
	m_toolTips[cmd_RenameFile]		= _T("Rename selected file or folder");
	m_toolTips[cmd_MoveFile]		= _T("Move selected file or folder");
	m_toolTips[cmd_CopyFile]		= _T("Copy selected file or folder");
	m_toolTips[cmd_CreateDirectory]	= _T("Create new folder");
//	m_toolTips[cmd_FilterChanged]	= _T("Select attributes filter");

	AddButtons(m_buttonCount,m_buttonInfo);
	SetButtonSize(29,29);
/*	
	// Load menu for the file filter operations
	HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(GWL_HINSTANCE);
	m_mainMenu		= LoadMenu(hInstance,MAKEINTRESOURCE(IDR_MENU)); 
	m_filterMenu	= GetSubMenu(m_mainMenu,0);

	// Get strings for the menu items and set an ownerdraw style for them
	MENUITEMINFO mii;
	TCHAR txtBuffer[WTB_MAX_MENU_ITEM_LENGTH];
	mii.cbSize	= sizeof(MENUITEMINFO);
	for ( int i = GetMenuItemCount(m_filterMenu)-1; i >= 0; i-- )
	{
		mii.fMask		= MIIM_ID | MIIM_STRING;
		mii.dwTypeData	= txtBuffer;
		mii.cch			= sizeof(txtBuffer)/sizeof(txtBuffer[0]);
		GetMenuItemInfo(m_filterMenu,i,TRUE,&mii);
		m_filterMenuText[mii.wID] = txtBuffer;

		mii.fMask	= MIIM_TYPE;
		mii.fType	= MFT_OWNERDRAW;
		SetMenuItemInfo(m_filterMenu,i,TRUE,&mii);
	}
	LOGFONT sysFont;
	::GetObject(GetStockObject(SYSTEM_FONT),sizeof(sysFont),&sysFont);
	Font = sysFont;
	
	OnSkinChanged();
	
	int resId = IDR_TB_ATTRIBUTES_MOUSEOVER_36x29;
	CImage* img;
	m_skinsImageList->ImageFromRes(&img,1,&resId);
	COLORREF pixelColor = img->GetPixel(0,0);
	m_bkBrush = CreateSolidBrush(pixelColor);
	
	pixelColor = img->GetPixel(0,img->GetHeight()-1);
	m_highlightPen = CreatePen(PS_SOLID,1,pixelColor);
*/	
	Log.Add(_MESSAGE_,_T("END CWidgetToolBar::CWidgetToolBar"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CWidgetToolBar::~CWidgetToolBar()
{
TRY_CATCH
/*
	DestroyMenu(m_mainMenu);
	DeleteObject(m_bkBrush);
	DeleteObject(m_highlightPen);
*/
	DestroyWindow();

CATCH_LOG()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetToolBar::ResizeWidget(LPRECT rect)
{
TRY_CATCH

	MoveWindow(rect,TRUE);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetToolBar::LockWidget(EWidgetCommand command)
{
TRY_CATCH

	if ( command == cmd_NullCommand )
		for ( int i = 0; i < m_buttonCount; i++ )
			SetState(m_buttonInfo[i].idCommand,0);
	else
		SetState(command,0);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetToolBar::UnlockWidget(EWidgetCommand command)
{
TRY_CATCH

	if ( command == cmd_NullCommand )
		for ( int i = 0; i < m_buttonCount; i++ )
			SetState(m_buttonInfo[i].idCommand,TBSTATE_ENABLED);
	else
		SetState(command,TBSTATE_ENABLED);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetToolBar::SetDisabledAttributes(const unsigned int attributes)
{
TRY_CATCH

	m_disabledAttributes = attributes;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

unsigned int CWidgetToolBar::GetDisabledAttributes()
{
TRY_CATCH

	return m_disabledAttributes;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetToolBar::OnSkinChanged()
{
TRY_CATCH

/*
	//Creating font
	if ( m_hFont )
		::DeleteObject(m_hFont);
	m_hFont = CreateFontIndirect(&Font);
	if (m_hFont == NULL)
		Log.Add(_WARNING_,_T("CWidgetToolBar::OnSkinChanged: Failed to CreateFontIndirect"));

	// Calc item width and height
	m_checkMarkWidth = GetSystemMetrics(SM_CXMENUCHECK);
	m_checkMarkHeignt = GetSystemMetrics(SM_CYMENUCHECK);

	HDC hdc = GetDC();

	m_marginSpace = 3;
	m_maxItemWidth = m_maxItemHeight = 0;
	HGDIOBJ oldFont = SelectObject(hdc,m_hFont);
	for ( std::map<int,tstring>::iterator iTxt = m_filterMenuText.begin(); iTxt != m_filterMenuText.end(); iTxt++ )
	{
		RECT rc = {0,0,0,0};
		DrawText(hdc,iTxt->second.c_str(),(int)iTxt->second.size(),&rc,DT_CALCRECT);

		if ( m_maxItemWidth < rc.right - rc.left )
			m_maxItemWidth = rc.right - rc.left;
		if ( m_maxItemHeight < rc.bottom - rc.top )
			m_maxItemHeight = rc.bottom - rc.top;
	}
	SelectObject(hdc,oldFont);

	ReleaseDC(hdc);
	
	m_maxItemWidth += m_checkMarkWidth + m_marginSpace*3;
	m_maxItemHeight = max(m_checkMarkHeignt,m_maxItemHeight) + m_marginSpace;
*/

CATCH_THROW()
}
// CWidgetToolBar [END] //////////////////////////////////////////////////////////////////////////////////
