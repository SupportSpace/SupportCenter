#pragma once

#include <vector>
#include "afxwin.h"

#define WM_CHAT_IS_CLOSE		(WM_USER + 10)
#define WM_CHAT_MOUSE		(WM_USER + 11)
#define WM_CHAT_KBRD		(WM_USER + 12)

typedef std::pair<RECT,int> CtrlPair;
typedef std::vector<CtrlPair> Controls;

// CFakedChat dialog

class CFakedChat : public CDialog
{
	DECLARE_DYNAMIC(CFakedChat)

public:
	CFakedChat(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFakedChat();

// Dialog Data
	enum { IDD = IDD_FAKED_CHAT };

protected:
	CString m_messageForSend;
	CString m_chatBoardText;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedChatSendbutton();
	afx_msg void OnBnClickedChatSmileglad();
	afx_msg void OnBnClickedChatSmilesad();
	afx_msg void OnBnClickedChatSmileneutral();
	afx_msg void OnBnClickedChatSmileglasses();
	afx_msg void OnBnClickedChatSmilelol();
	afx_msg void OnBnClickedChatSmilecrazy();
	afx_msg void OnClose();
	afx_msg LRESULT OnMouseMsg(WPARAM, LPARAM);
	afx_msg LRESULT OnKeyboardMsg(WPARAM, LPARAM);
	afx_msg void OnBnClickedChatClose();
	afx_msg void OnShowWindow(BOOL, UINT);

	virtual BOOL Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);
public:
	static unsigned int m_mouseMsg;
	static unsigned int m_keyboardMsg;
private:
	Controls m_controls;
	CEdit m_sendBox;
	bool m_leftButton;
	void PrepareMap();
	int GetControlByPoint(POINT point);
	bool m_keyboardLoaded;
	BYTE m_keyboard[256];

};
