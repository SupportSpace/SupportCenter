// SessionDlg.h : Declaration of the CSessionDlg

#pragma once

#include "resource.h"       // main symbols

#include <atlhost.h>
#include "boost/shared_ptr.hpp"
#include "PropertiesDlg.h"
#include <boost/function.hpp>
#include "CSkinnedElement.h"
#include "CSkinnedComboBox.h"
// CSessionDlg

class CSessionDlg : 
	public CAxDialogImpl<CSessionDlg>,
	public CSkinnedElement
{
	
public:
	
	CSessionDlg():
			m_propertiesBtn(static_cast<CSkinnedElement*>(this),1)
	{
		
	}

	~CSessionDlg()
	{
	}

	enum { IDD = IDD_SESSIONDLG };
	enum { ANIMATE_TIME=0 };
BEGIN_MSG_MAP(CSessionDlg)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	//COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
	//COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
	COMMAND_HANDLER(IDC_START, BN_CLICKED, OnBnClickedStart)
	COMMAND_HANDLER(IDC_PROPERTIES, BN_CLICKED, OnBnClickedProperties)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	COMMAND_HANDLER(IDC_STOP, BN_CLICKED, OnBnClickedStop)
	//MESSAGE_HANDLER(WM_ERASEBKGND,OnErasebkgnd)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	MESSAGE_HANDLER(WM_DRAWITEM,OnDrawItem)
	MESSAGE_HANDLER_HWND(WM_PAINT,OnPaint) ///asd
	MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
	CHAIN_MSG_MAP(CSkinnedElement)
	CHAIN_MSG_MAP(CAxDialogImpl<CSessionDlg>)
ALT_MSG_MAP(1)
	CHAIN_MSG_MAP(CSkinnedElement)
END_MSG_MAP()

	virtual LRESULT OnPaint(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		PAINTSTRUCT paint;
		HDC hdc=BeginPaint(&paint);
		OnEraseBkgnd(hWnd, uMsg, (WPARAM)hdc, lParam, bHandled);
		EndPaint(&paint);
		bHandled = TRUE;
		return TRUE;
	}
	
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	boost::shared_ptr<CPropertiesDlg> propertiesDlg;
public:
	LRESULT OnBnClickedStart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
public:
	LRESULT OnBnClickedProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
public:
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	boost::function<void (void)> m_startClick;
	boost::function<void (void)> m_stopClick;
public:
	LRESULT OnBnClickedStop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnErasebkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	CImage skinImage;
	CContainedWindow m_propertiesBtn;
public:
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDrawItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	CSkinnedComboBox m_comboBox;
public:
	LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
};


