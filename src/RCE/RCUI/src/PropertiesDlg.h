// PropertiesDlg.h : Declaration of the CPropertiesDlg

#pragma once

#include "resource.h"       // main symbols

#include <atlhost.h>
#include "wtl/atlctrls.h"


// CPropertiesDlg

//#define RT_SKINS _T("SKINS")
// The function loads the specified resource from a module's executable file and create stream via CreateStreamOnHGlobal().The application must call Release() method to delete Stream object.
// @param hInstance Handle to the instance of the module whose executable file contains the bitmap to be loaded.
// @param lpType Specifies the resource type.
// @param lpNameResource Pointer to a null-terminated string that contains the name of the resource to be loaded. Alternatively, this parameter can consist of the resource identifier in the low-order word and zero in the high-order word. The MAKEINTRESOURCE macro can be used to create this value.
//IStream* LoadStream(HINSTANCE hInstance,LPCTSTR lpType,LPCTSTR lpNameResource);

class CPropertiesDlg : 
	public CAxDialogImpl<CPropertiesDlg>
{
public:
	CPropertiesDlg()
	{
	}

	~CPropertiesDlg()
	{
	}

	enum { IDD = IDD_PROPERTIESDLG };

BEGIN_MSG_MAP(CPropertiesDlg)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_HANDLER(IDOK, BN_CLICKED, OnBnClickedOk)
	MESSAGE_HANDLER(WM_PAINT, OnPaint)
	MESSAGE_HANDLER(WM_ERASEBKGND,OnErasebkgnd)
	MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClk)
	MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
	CHAIN_MSG_MAP(CAxDialogImpl<CPropertiesDlg>)
END_MSG_MAP()

// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnErasebkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
public:
	LRESULT OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	CImage skinImage;
	//CComPtr<IStream> skinStream;
public:
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
public:
	LRESULT OnLButtonDblClk(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
public:
	LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	
};


