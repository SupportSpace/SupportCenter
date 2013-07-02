//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CWidgetHistoryListView.cpp
///
///  Implements CWidgetHistoryListView class
///  
///  
///  @author Alexander Novak @date 28.11.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CWidgetHistoryListView.h"
#include "CCommandManager.h"
#include "resource.h"

// CWidgetHistoryListView [BEGIN] ////////////////////////////////////////////////////////////////////////

LRESULT CWidgetHistoryListView::OnCustomDrawHeader(int wParam, LPNMHDR notifyHeader, BOOL& bHandled)
{
TRY_CATCH

	LPNMCUSTOMDRAW customDraw = (LPNMCUSTOMDRAW)notifyHeader;
	
	switch ( customDraw->dwDrawStage )
	{
		case CDDS_PREPAINT:
			return CDRF_NOTIFYPOSTPAINT;

		case CDDS_POSTPAINT:
		{
			if ( customDraw->rc.right <= customDraw->rc.left || customDraw->rc.bottom <= customDraw->rc.top ) 
				return CDRF_SKIPDEFAULT;
				
			RECT rcHeader;
			::GetClientRect(customDraw->hdr.hwndFrom,&rcHeader);
						
			if ( m_fillImage )
			{
				m_fillImage->Draw(customDraw->hdc,customDraw->rc);

				if ( m_topImage && customDraw->rc.top < rcHeader.top + m_topImage->GetHeight() )
				{
					customDraw->rc.top = rcHeader.top;
	
					RECT rcTop = customDraw->rc;
					rcTop.bottom = rcTop.top + m_topImage->GetHeight();

					m_topImage->Draw(customDraw->hdc,rcTop);
				}
			}
			else
			{
				customDraw->rc.right++;
				customDraw->rc.bottom++;
				
				FillRect(customDraw->hdc,&customDraw->rc,static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
			}
			int oldBkMode = SetBkMode(customDraw->hdc,TRANSPARENT);
			::SetTextColor(customDraw->hdc,FontColor1);
			HGDIOBJ oldPen = SelectObject(customDraw->hdc,CreatePen(PS_SOLID,1,FontColor1));

			int itemCount = Header_GetItemCount(customDraw->hdr.hwndFrom);
			for ( int i = 0; i < itemCount; i++ )
			{
				RECT rcItem;
				Header_GetItemRect(customDraw->hdr.hwndFrom,i,&rcItem);
				
				TCHAR textBuffer[101];
				HDITEM itemInfo;
				itemInfo.mask		= HDI_FORMAT|HDI_TEXT;
				itemInfo.pszText	= textBuffer;
				itemInfo.cchTextMax	= sizeof(textBuffer)/sizeof(textBuffer[0]);
				Header_GetItem(customDraw->hdr.hwndFrom,i,&itemInfo);
				
				itemInfo.fmt &= HDF_JUSTIFYMASK;
				UINT textFormat = DT_VCENTER|DT_SINGLELINE;
				switch ( itemInfo.fmt )
				{
					case HDF_CENTER:
						textFormat |= DT_CENTER;
						break;
					case HDF_RIGHT:
						textFormat |= DT_RIGHT;
						break;
					default:
						textFormat |= DT_LEFT;
				}
				rcItem.left		+= m_textMargin;
				rcItem.right	-= m_textMargin;

				DrawText(customDraw->hdc,itemInfo.pszText,static_cast<int>(_tcslen(itemInfo.pszText)),&rcItem,textFormat);
				
				rcItem.right	+= m_textMargin;

				rcItem.top		+= m_textMargin/2;
				rcItem.bottom	-= m_textMargin/2;

				MoveToEx(customDraw->hdc,rcItem.right,rcItem.top,NULL);
				LineTo(customDraw->hdc,rcItem.right,rcItem.bottom);
			}
			DeleteObject(SelectObject(customDraw->hdc,oldPen));
			SetBkMode(customDraw->hdc,oldBkMode);

			return CDRF_SKIPDEFAULT;
		}
	}
	return CDRF_DODEFAULT;
	
CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetHistoryListView::OnCustomDrawList(int wParam, LPNMHDR notifyHeader, BOOL& bHandled)
{
TRY_CATCH

	LPNMLVCUSTOMDRAW customDraw = (LPNMLVCUSTOMDRAW)notifyHeader;
	
	switch ( customDraw->nmcd.dwDrawStage )
	{
		case CDDS_PREPAINT:
			
			if ( customDraw->nmcd.rc.right <= customDraw->nmcd.rc.left || customDraw->nmcd.rc.bottom <= customDraw->nmcd.rc.top ) 
				return CDRF_SKIPDEFAULT;

			RECT rcListView;
			GetClientRect(&rcListView);
									
			if ( m_fillImage )
			{
				m_fillImage->Draw(customDraw->nmcd.hdc,customDraw->nmcd.rc);

				if ( m_bottomImage && customDraw->nmcd.rc.bottom > rcListView.bottom - m_bottomImage->GetHeight() )
				{
					customDraw->nmcd.rc.bottom = rcListView.bottom;
	
					RECT rcBottom = customDraw->nmcd.rc;
					rcBottom.top = rcBottom.bottom - m_bottomImage->GetHeight();

					m_bottomImage->Draw(customDraw->nmcd.hdc,rcBottom);
				}
			}
			else
			{
				customDraw->nmcd.rc.right++;
				customDraw->nmcd.rc.bottom++;
				
				FillRect(customDraw->nmcd.hdc,&customDraw->nmcd.rc,static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
			}
			return CDRF_NOTIFYITEMDRAW;

		case CDDS_ITEMPREPAINT:
		{
			int itemIndex = (int)customDraw->nmcd.dwItemSpec;
			RECT rcItem;
			GetSubItemRect(itemIndex,0,LVIR_BOUNDS,&rcItem);
			
			int oldBkMode = SetBkMode(customDraw->nmcd.hdc,TRANSPARENT);
			::SetTextColor(customDraw->nmcd.hdc,FontColor1);
			
			SHistoryInfo& updateInfo = m_updateInfo[itemIndex];

			CImage* directionImage = ( updateInfo.m_receiveDirection ) ? m_receiveImage : m_sendImage;

			rcItem.left		= (GetColumnWidth(0) - directionImage->GetWidth())/2;
			rcItem.right	= rcItem.left + directionImage->GetWidth();
			rcItem.bottom	= rcItem.top + directionImage->GetHeight();
			
			BitBlt(	customDraw->nmcd.hdc,
					rcItem.left,
					rcItem.top,
					directionImage->GetWidth(),
					directionImage->GetHeight(),
					directionImage->GetDC(),
					0,
					0,
					SRCCOPY);
			directionImage->ReleaseDC();
			
			GetSubItemRect(itemIndex,1,LVIR_BOUNDS,&rcItem);

			rcItem.left += m_textMargin;
			DrawIconEx(	customDraw->nmcd.hdc,
						rcItem.left,
						rcItem.top,
						updateInfo.m_typeIcon.get(),
						0,
						0,
						0,
						NULL,
						DI_NORMAL);
			rcItem.left += m_iconMargin;

			DrawText(	customDraw->nmcd.hdc,
						updateInfo.m_fileName.c_str(),
						static_cast<int>(updateInfo.m_fileName.size()),
						&rcItem,
						DT_TOP|DT_SINGLELINE|DT_LEFT);

			GetSubItemRect(itemIndex,2,LVIR_BOUNDS,&rcItem);

			rcItem.left += m_textMargin;
			DrawText(	customDraw->nmcd.hdc,
						updateInfo.m_fileSize.c_str(),
						static_cast<int>(updateInfo.m_fileSize.size()),
						&rcItem,
						DT_TOP|DT_SINGLELINE|DT_LEFT/*DT_RIGHT*/);

			GetSubItemRect(itemIndex,3,LVIR_BOUNDS,&rcItem);

			rcItem.left += m_textMargin;
			DrawText(	customDraw->nmcd.hdc,
						updateInfo.m_fileDate.c_str(),
						static_cast<int>(updateInfo.m_fileDate.size()),
						&rcItem,
						DT_TOP|DT_SINGLELINE|DT_LEFT/*DT_RIGHT*/);

			SetBkMode(customDraw->nmcd.hdc,oldBkMode);
		
			return CDRF_SKIPDEFAULT;
		}
	}
	return CDRF_DODEFAULT;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetHistoryListView::OnUpdateControl(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	bHandled = FALSE;
	
	RECT rcUpdate, rcHeader;
	GetClientRect(&rcUpdate);
	::GetClientRect(GetHeader(),&rcHeader);
	rcUpdate.top += rcHeader.bottom - rcHeader.top;
	
	InvalidateRect(&rcUpdate,FALSE);
	
	return FALSE;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetHistoryListView::OnMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	LPMEASUREITEMSTRUCT measureItem = (LPMEASUREITEMSTRUCT)lParam;

	measureItem->CtlType	= ODT_LISTVIEW;
    measureItem->CtlID		= 0;
    measureItem->itemID		= 0;
    measureItem->itemWidth	= 0;
    measureItem->itemHeight	= m_iconMargin;
    measureItem->itemData	= 0;

	return TRUE;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CWidgetHistoryListView::CWidgetHistoryListView(	HWND hParentWnd,
												boost::shared_ptr<CAbstractCommandManager> commandManager,
												const int fillImageId,
												const int topImageId,
												const int bottomImageId)
	:	CSkinnedElement(commandManager.get()),
		CCommandProxy(commandManager),
		m_fillImage(NULL),
		m_topImage(NULL),
		m_bottomImage(NULL)
{
TRY_CATCH

	Create(	hParentWnd,
			0,
			NULL,
			WS_CHILD|WS_VISIBLE|LVS_REPORT|LVS_NOSORTHEADER|LVS_SINGLESEL|LVS_OWNERDRAWFIXED);
	
	m_textMargin = GetSystemMetrics(SM_CXEDGE) * 3; // Hardcoded magic number inside Header control
	m_iconMargin = GetSystemMetrics(SM_CXSMICON) + GetSystemMetrics(SM_CXEDGE);
			
	if ( fillImageId != -1 ) 
		m_skinsImageList->ImageFromRes(&m_fillImage,1,&fillImageId);
	if ( topImageId != -1 ) 
		m_skinsImageList->ImageFromRes(&m_topImage,1,&topImageId);
	if ( bottomImageId != -1 ) 
		m_skinsImageList->ImageFromRes(&m_bottomImage,1,&bottomImageId);
		
	int resId = IDR_HLV_SENDDIRECTION_16x16;
	m_skinsImageList->ImageFromRes(&m_sendImage,1,&resId);
	
	resId = IDR_HLV_RECEIVEDIRECTION_16x16;
	m_skinsImageList->ImageFromRes(&m_receiveImage,1,&resId);

	InsertColumn(0,_T("Direction"),LVCFMT_CENTER,60);
	InsertColumn(1,_T("Name"),LVCFMT_LEFT,195);
	InsertColumn(2,_T("Size"),LVCFMT_LEFT,70);
	InsertColumn(3,_T("Date"),LVCFMT_LEFT,120);
	
	SetExtendedListViewStyle(LVS_EX_BORDERSELECT);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CWidgetHistoryListView::~CWidgetHistoryListView()
{
TRY_CATCH

	DestroyWindow();

CATCH_LOG()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetHistoryListView::ResizeWidget(LPRECT rect)
{
TRY_CATCH

	MoveWindow(rect,TRUE);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetHistoryListView::AddTransferInfo(const CFileData& fileData, bool receiveDirection)
{
TRY_CATCH

	SHistoryInfo itemData;
	itemData.m_receiveDirection	= receiveDirection;
	itemData.m_fileName			= fileData.GetFileName();

	SHFILEINFO shfi;
	if ( SHGetFileInfo(	fileData.GetFileName(),	
						fileData.GetAttributes(),
						&shfi,
						sizeof(SHFILEINFO),
						SHGFI_ICON|SHGFI_SMALLICON|SHGFI_USEFILEATTRIBUTES) )
		itemData.m_typeIcon.reset(shfi.hIcon,DeleteObject);

	TCHAR frmtStr[25];
	frmtStr[0] = _T('\0');
	if ( !(fileData.GetAttributes() & FILE_ATTRIBUTE_DIRECTORY) )
	{
		ULARGE_INTEGER fileSizeKB = fileData.GetFileSize();
		fileSizeKB.QuadPart /= 1024;
		_stprintf_s(frmtStr,sizeof(frmtStr)/sizeof(frmtStr[0]),_T("%I64u KB"),fileSizeKB.QuadPart);

		itemData.m_fileSize = frmtStr;
	}

	frmtStr[0] = _T('\0');

	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);
	_stprintf_s(frmtStr,
				sizeof(frmtStr)/sizeof(frmtStr[0]),
				_T("%d/%.2d/%d %d:%.2d:%.2d"),
				sysTime.wDay,
				sysTime.wMonth,
				sysTime.wYear,
				sysTime.wHour,
				sysTime.wMinute,
				sysTime.wSecond);
	itemData.m_fileDate = frmtStr;

	std::vector<SHistoryInfo>::iterator insertedItem = m_updateInfo.insert(m_updateInfo.begin(),itemData);

	if ( InsertItem(0,_T("_____")) != -1 )
	{
		SetItem(0,1,LVIF_TEXT,itemData.m_fileName.c_str(),0,0,0,NULL);
		SetItem(0,2,LVIF_TEXT,itemData.m_fileSize.c_str(),0,0,0,NULL);
		SetItem(0,3,LVIF_TEXT,itemData.m_fileDate.c_str(),0,0,0,NULL);
	}
	else
		m_updateInfo.erase(insertedItem);

CATCH_THROW()
}
// CWidgetHistoryListView [END] //////////////////////////////////////////////////////////////////////////
