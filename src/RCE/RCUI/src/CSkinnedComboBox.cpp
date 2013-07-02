/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSkinnedComboBox.cpp
///
///  CSkinnedComboBox, skinned combobox
///
///  @author "Archer Software" Kirill Solovyov @date 10.07.2006
///
////////////////////////////////////////////////////////////////////////
#include "CSkinnedComboBox.h"
//#include <AidLib/Logging/cLog.h>
#include "wtl/atlgdi.h"
#include <boost/shared_array.hpp>
#include <boost\type_traits\remove_pointer.hpp>

CSkinnedComboBox::CSkinnedComboBox(	CSkinnedElement *parent,
									const int regularImageId,
									const int disabledImageId,
									const int pressedImageId,
									const int mouseOverImageId )
	:	CSkinnedElement(parent)
{
TRY_CATCH
	if (NULL != m_skinsImageList.get())
	{
		m_skinsImageList->ImageFromRes(&m_regularImage,1,&regularImageId);
		m_skinsImageList->ImageFromRes(&m_mouseOverImage,1,&mouseOverImageId);
		m_skinsImageList->ImageFromRes(&m_pressedImage,1,&pressedImageId);
		m_skinsImageList->ImageFromRes(&m_disabledImage,1,&disabledImageId);
		m_currentImage = m_regularImage;
	} 
	else
		Log.Add(_WARNING_,_T("NULL images list while loading control"));
CATCH_LOG()
}

CSkinnedComboBox::~CSkinnedComboBox(void)
{
TRY_CATCH
CATCH_LOG()
}

void CSkinnedComboBox::SizeToImage()
{
TRY_CATCH
	if(NULL == m_currentImage)
		MCException("The m_currentImage wasn't set");
	if(!SetWindowPos(NULL,0,0,m_currentImage->GetWidth(),m_currentImage->GetHeight(),SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE))
		throw MCException_Win("Sizing failed");
	if(SetItemHeight(-1/* height of selection field*/,m_currentImage->GetHeight()-2*GetSystemMetrics(SM_CYFIXEDFRAME))==CB_ERR)
		throw MCException("SetItemHeight() failed");
CATCH_THROW()
}


LRESULT CSkinnedComboBox::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
TRY_CATCH
	//The low-order word of lParam specifies the new width of the client area. 
	//The high-order word of lParam specifies the new height of the client area. 
CATCH_LOG()
	return 0;
}

BOOL CSkinnedComboBox::Attach(HWND hWnd)
{
TRY_CATCH
	ATLASSERT(::IsWindow(hWnd));
	//BOOL bRet = CWindowImplEx<CSkinnedComboBox, CComboBox>::SubclassWindow(hWnd);
	BOOL bRet = SubclassWindow(hWnd);
	COMBOBOXINFO cmbInfo;
	cmbInfo.cbSize=sizeof(cmbInfo);
	GetComboBoxInfo(&cmbInfo);
	m_list.EdgeColor1=EdgeColor1;
	m_list.SubclassWindow(cmbInfo.hwndList);
	OnSkinChanged();
	return bRet;
CATCH_THROW()
}

LRESULT CSkinnedComboBox::OnEraseBkgnd(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH
	return CSkinnedElement::OnEraseBkgnd(hWnd, uMsg, wParam, lParam, bHandled);
CATCH_THROW()
}

LRESULT CSkinnedComboBox::OnPaint(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	LRESULT res = OnEraseBkgnd(hWnd, uMsg, wParam, lParam, bHandled);

	// Drawing original ComboBox item
	RECT rc;
	GetClientRect(&rc);
	InflateRect(&rc,-GetSystemMetrics(SM_CXEDGE),-GetSystemMetrics(SM_CYEDGE));
	rc.right-=GetSystemMetrics(SM_CXVSCROLL);
	InvalidateRect(&rc, FALSE);
	DefWindowProc(WM_PAINT,wParam,lParam);
	return res;
CATCH_THROW()
}

LRESULT CSkinnedComboBox::OnPrint(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH
	RECT rc;
	GetClientRect(&rc);
	CMemoryDC memDC(reinterpret_cast<HDC>(wParam), rc);
	// Drawing original ComboBox item
	DefWindowProc(WM_PRINT,reinterpret_cast<WPARAM>(memDC.m_hDC),lParam);
	InflateRect(&rc,-GetSystemMetrics(SM_CXEDGE),-GetSystemMetrics(SM_CYEDGE));
	rc.right-=GetSystemMetrics(SM_CXVSCROLL);
	memDC.ExcludeClipRect(rc.left,rc.top,rc.right,rc.bottom);
	return OnEraseBkgnd(hWnd, uMsg,reinterpret_cast<WPARAM>(memDC.m_hDC), lParam, bHandled);
CATCH_THROW()
}

LRESULT CSkinnedComboBox::OnNCPaint(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH
	return OnPaint(hWnd, uMsg, wParam, lParam, bHandled);
CATCH_THROW()
}

void CSkinnedComboBox::OnSkinChanged()
{
TRY_CATCH
	if (!IsWindowEnabled())
		m_currentImage = m_disabledImage;
	else
		m_currentImage = m_regularImage;
	if (NULL != m_currentImage)
		SizeToImage();
	RedrawWindow();
CATCH_THROW()
}

//CImage *asd;

void CSkinnedComboBox::OnLMouseDown()
{
TRY_CATCH
	if (!IsWindowEnabled() || m_currentImage == m_pressedImage)
		return;
	m_currentImage = m_pressedImage;
	RedrawWindow();
CATCH_THROW()
}

void CSkinnedComboBox::OnLMouseUp()
{
TRY_CATCH
	if (!IsWindowEnabled() || m_currentImage == m_regularImage)
		return;
	m_currentImage = m_regularImage;
	RedrawWindow();
CATCH_THROW()
}

void CSkinnedComboBox::OnMouseMove(int fwKeys)
{
TRY_CATCH
	if (MK_LBUTTON == (fwKeys & MK_LBUTTON))
	{
		OnLMouseDown();
	}
	else
	{
		OnLMouseUp();
	}
CATCH_THROW()
}

LRESULT CSkinnedComboBox::OnDrawItem(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH
	DRAWITEMSTRUCT *dis =reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
	if(hWnd==m_hWnd&&dis->CtlType==ODT_COMBOBOX)
	{
		int textLength=CComboBox(dis->hwndItem).GetLBTextLen(dis->itemID);
		boost::shared_array<TCHAR> text(new TCHAR[textLength+1]);
		text[textLength]=_T('\0');
		CComboBox(dis->hwndItem).GetLBText(dis->itemID,text.get());
		CDCHandle dc(dis->hDC);
		dc.SetTextColor((dis->itemState==ODS_DISABLED)?FontColor2:FontColor1);
		//BkColor1: odd item indexes and edit; BkColor2: even item indexes
		dc.SetBkColor((!(dis->itemState&ODS_SELECTED)||(dis->itemState&ODS_COMBOBOXEDIT))?BkColor1:BkColor2);
		if(dis->itemState&ODS_COMBOBOXEDIT)
		{
			// redraw edit border (draq by system earlier)
			RECT rect(dis->rcItem);
			InflateRect(&rect,GetSystemMetrics(SM_CXBORDER),GetSystemMetrics(SM_CYBORDER));
			HRGN oldRgn=NULL;
			bool bSelectClipRgn=false;
			oldRgn=CreateRectRgn(0,0,0,0);
			boost::shared_ptr<boost::remove_pointer<HRGN>::type> __oldRgn(oldRgn,::DeleteObject);
			if(GetClipRgn(dis->hDC,oldRgn)==1)
			{
				HRGN rgn=CreateRectRgn(rect.left,rect.top,rect.right,rect.bottom);
				bSelectClipRgn=SelectClipRgn(dis->hDC,rgn)!=ERROR;
				DeleteObject(rgn);
			}
			dc.ExtTextOut(dis->rcItem.left,dis->rcItem.top,ETO_CLIPPED|ETO_OPAQUE,&rect,text.get(),textLength,NULL);
			if(bSelectClipRgn)
				SelectClipRgn(dis->hDC,oldRgn);
		}
		else
		{
			dc.ExtTextOut(dis->rcItem.left+GetSystemMetrics(SM_CXEDGE),dis->rcItem.top+GetSystemMetrics(SM_CYEDGE)/2,
                                     ETO_CLIPPED|ETO_OPAQUE,&dis->rcItem,text.get(),textLength,NULL);
			//draw splitline
			if(dis->itemID!=0&&EdgeColor1!=-1)
			{
				CPen pen;
				pen.CreatePen(PS_SOLID,GetSystemMetrics(SM_CXBORDER),EdgeColor1);
				HPEN oldPen=dc.SelectPen(pen);
				dc.MoveTo(dis->rcItem.left,dis->rcItem.top);
				dc.LineTo(dis->rcItem.right,dis->rcItem.top);
				dc.SelectPen(oldPen);
			}
		}
		//if(dis->itemState&ODS_FOCUS)
		//{
		//	if(!(dis->itemState&ODS_COMBOBOXEDIT))
		//		InflateRect(&dis->rcItem,-GetSystemMetrics(SM_CXEDGE)/2,-GetSystemMetrics(SM_CYEDGE)/2);
		//	dc.DrawFocusRect(&dis->rcItem);
		//}
		//Log.Add(_MESSAGE_,_T("CSkinnedComboBox::OnDrawItem t=%x cid=%x iid=%x a=%x s=%x hw=%x dc=%x r=%d d=%d"),dis->CtlType,dis->CtlID,dis->itemID
		//	,dis->itemAction,dis->itemState,dis->hwndItem,dis->hDC,dis->rcItem.right,dis->itemData);
		return TRUE;
	}
CATCH_LOG()
	bHandled=FALSE;
	return FALSE;
}

LRESULT CSkinnedComboBox::OnMeasureItem(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH
	MEASUREITEMSTRUCT *msrItem=reinterpret_cast<MEASUREITEMSTRUCT*>(lParam);
	//set height of edit field
	if(hWnd==m_hWnd&&msrItem->CtlType==ODT_COMBOBOX&&msrItem->itemID==-1/*by edit field*/&&m_currentImage)
	{
		msrItem->itemHeight=m_currentImage->GetHeight()-2*GetSystemMetrics(SM_CYFIXEDFRAME);
		return TRUE;
	}
CATCH_LOG()
	bHandled=FALSE;
	return 0;
}
void CSkinnedComboBox::SetLogFont(LOGFONT newVal)
{
TRY_CATCH
	CSkinnedElement::SetLogFont(newVal);
	CFontHandle hFont;
	hFont.CreateFontIndirect(&newVal);
	SetFont(hFont,FALSE);
	m_hFont=hFont;
CATCH_THROW()
}
void CSkinnedComboBox::SetEdgeColor1(COLORREF newVal)
{
	m_list.EdgeColor1=newVal;
	CSkinnedElement::SetEdgeColor1(newVal);
}
