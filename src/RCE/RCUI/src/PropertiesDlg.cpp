// PropertiesDlg.cpp : Implementation of CPropertiesDlg

#include "stdafx.h"
#include "PropertiesDlg.h"
#include <NWL/Streaming/CStreamException.h>
#include "CSkinnedElement.h"
// CPropertiesDlg
LRESULT CPropertiesDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CAxDialogImpl<CPropertiesDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);
	RECT dlgRect;
	if(!GetWindowRect(&dlgRect))
		throw MCException_Win("Dialog rect obtaining failed");
	HRGN hRgn;
	//if(!(hRgn=CreateEllipticRgn(0,0,dlgRect.right-dlgRect.left,dlgRect.bottom-dlgRect.top)))
	if(!(hRgn=CreateRoundRectRgn(0,0,dlgRect.right-dlgRect.left,dlgRect.bottom-dlgRect.top,10,10)))
		throw MCException_Win("Region creation failed");
	if(!SetWindowRgn(hRgn))
		throw MCException_Win("Dialog region set failed");
	//skinImage.LoadFromResource(_AtlBaseModule.m_hInst,IDB_PROPERTIESBITMAP);
	//skinImage.Load(_T("C:\\Documents and Settings\\Administrator\\My Documents\\My Pictures\\properties.PNG"));
	CComPtr<IStream> skinStream;
	skinStream.Attach(LoadStream(_AtlBaseModule.m_hInst,RT_SKINS,MAKEINTRESOURCE(IDR_SKINS1)));
	skinImage.Load(skinStream);
	
	//skinImage.LoadFromResource(_AtlBaseModule.m_hInst,IDR_SKINS1);
	bHandled = TRUE;
	return 1;  // Let the system set the focus
}

LRESULT CPropertiesDlg::OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	GetDlgItem(IDCANCEL).BringWindowToTop();
	RedrawWindow();
	return 0;
}

LRESULT CPropertiesDlg::OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default
	//HDC hdc=GetDC();
	//RECT dlgRect;
	//GetWindowRect(&dlgRect);
	//ScreenToClient(&dlgRect);
	//skinImage.Draw(hdc,dlgRect,dlgRect);
	//ReleaseDC(hdc);
	//::Beep(1000,50);
	HDC hdc;
	PAINTSTRUCT paint;
	if(wParam)
	{
		::Beep(440,50);
	}
	else
	{
		hdc=BeginPaint(&paint);
		//if(paint.fErase)
		//	skinImage.Draw(hdc,paint.rcPaint,paint.rcPaint);
		EndPaint(&paint);
		return 0;
	}
	return 1;
}

LRESULT CPropertiesDlg::OnErasebkgnd(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	HDC hdc=reinterpret_cast<HDC>(wParam);
	RECT dlgRect;
	GetWindowRect(&dlgRect);
	ScreenToClient(&dlgRect);
	skinImage.Draw(hdc,dlgRect);
	return 1L;
}
//IStream* LoadStream(HINSTANCE hInstance, LPCTSTR lpType, LPCTSTR lpNameResource)
//{
//	HRESULT res=0;
//	HRSRC resource = FindResource(hInstance, lpNameResource, lpType);
//	if (!resource)
//		return NULL;
//	DWORD imageSize=SizeofResource(hInstance, resource);
//	if(!imageSize)
//		return NULL;
//	const void* resourceData=LockResource(LoadResource(hInstance, resource));
//	if(!resourceData)
//		return NULL;
//	HGLOBAL hBuffer=GlobalAlloc(GMEM_MOVEABLE, imageSize);
//	if(!hBuffer)
//		res=GetLastError();
//	else 
//	{
//		void* buffer=::GlobalLock(hBuffer);
//		if(!buffer)
//			res=GetLastError();
//		else
//		{
//			CopyMemory(buffer, resourceData, imageSize);
//			IStream* stream = NULL;
//			if ((res=CreateStreamOnHGlobal(hBuffer, TRUE, &stream))==S_OK)
//				return stream;
//			GlobalUnlock(buffer);
//		}
//	}
//	GlobalFree(hBuffer);
//	SetLastError(res);
//	return NULL;
//}
LRESULT CPropertiesDlg::OnLButtonDblClk(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default

	return 0;
}

LRESULT CPropertiesDlg::OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	::MessageBox(NULL,_T("CPropertiesDlg::OnLButtonDown"),NULL,0);

	return 0;
}
