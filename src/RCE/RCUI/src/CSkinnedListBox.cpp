/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSkinnedListBox.cpp
///
///  CSkinnedListBox, skinned list box
///
///  @author "Archer Software" Kirill Solovyov @date 10.07.2006
///
////////////////////////////////////////////////////////////////////////
#include "CSkinnedListBox.h"

CSkinnedListBox::CSkinnedListBox(void)
{
TRY_CATCH
CATCH_THROW()
}

CSkinnedListBox::~CSkinnedListBox(void)
{
TRY_CATCH
CATCH_LOG()
}

LRESULT CSkinnedListBox::OnEraseBkgnd(HWND hWnd, UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
TRY_CATCH
	bHandled=FALSE;
	return 0L;//unprocessed
CATCH_THROW()
}

LRESULT CSkinnedListBox::OnPaint(HWND hWnd, UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
TRY_CATCH
	bHandled=FALSE;
	return 1L;//unprocessed
CATCH_THROW()
}

void CSkinnedListBox::OnSkinChanged()
{
TRY_CATCH
CATCH_THROW()
}
LRESULT CSkinnedListBox::OnNCPaint(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH
	if(::IsWindow(hWnd))
	{
		HDC hdc;
		if(wParam==1)
			hdc=::GetDCEx(hWnd,0,DCX_WINDOW|DCX_PARENTCLIP);
		else
			hdc=::GetDCEx(hWnd,reinterpret_cast<HRGN>(wParam),DCX_WINDOW|DCX_INTERSECTRGN);
		CDC dc(hdc);
		RECT windowRect;
		CWindow window(hWnd);
		window.GetWindowRect(&windowRect);
		window.ScreenToClient(&windowRect);
		OffsetRect(&windowRect,GetSystemMetrics(SM_CXBORDER),GetSystemMetrics(SM_CYBORDER));
		//draw edgelines
		if(EdgeColor1!=-1)
		{
			CPen pen;
			pen.CreatePen(PS_SOLID,GetSystemMetrics(SM_CXBORDER),EdgeColor1);
			HPEN oldPen=dc.SelectPen(pen);
			dc.Rectangle(&windowRect);
			dc.SelectPen(oldPen);
			return 0;//processed
		}
	}
CATCH_LOG()
	bHandled=FALSE;
	return 1L;//unprocessed
}
BOOL CSkinnedListBox::Attach(HWND hWnd)
{
TRY_CATCH
	ATLASSERT(::IsWindow(hWnd));
	BOOL bRet = SubclassWindow(hWnd);
	OnSkinChanged();
	return bRet;
CATCH_THROW()
}