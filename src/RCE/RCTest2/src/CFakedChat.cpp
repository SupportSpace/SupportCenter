// CFakedChat.cpp : implementation file
//

#include "stdafx.h"
#include "RCTest2.h"
#include "CFakedChat.h"
#include "InputShared.h"
#include <boost/scoped_ptr.hpp>

// CFakedChat dialog

unsigned int CFakedChat::m_mouseMsg;
unsigned int CFakedChat::m_keyboardMsg;

IMPLEMENT_DYNAMIC(CFakedChat, CDialog)

CFakedChat::CFakedChat(CWnd* pParent /*=NULL*/)
	: CDialog(CFakedChat::IDD, pParent)
	, m_messageForSend(_T(""))
	, m_chatBoardText(_T(""))
	, m_controls(8)
	, m_leftButton(false)
	, m_keyboardLoaded(false)
{
	m_mouseMsg = RegisterWindowMessage(_T("CFakedChat::m_mouseMsg"));
	m_keyboardMsg = RegisterWindowMessage(_T("CFakedChat::m_keyboardMsg"));
	memset(m_keyboard, 0, sizeof(m_keyboard));
}

CFakedChat::~CFakedChat()
{
}

void CFakedChat::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_CHAT_SENDLINE, m_messageForSend);
	DDX_Text(pDX, IDC_CHAT_CHATBOARD, m_chatBoardText);
	DDX_Control(pDX, IDC_CHAT_SENDLINE, m_sendBox);
}


BEGIN_MESSAGE_MAP(CFakedChat, CDialog)
	ON_BN_CLICKED(IDC_CHAT_SENDBUTTON, &CFakedChat::OnBnClickedChatSendbutton)
	ON_BN_CLICKED(IDC_CHAT_SMILEGLAD, &CFakedChat::OnBnClickedChatSmileglad)
	ON_BN_CLICKED(IDC_CHAT_SMILESAD, &CFakedChat::OnBnClickedChatSmilesad)
	ON_BN_CLICKED(IDC_CHAT_SMILENEUTRAL, &CFakedChat::OnBnClickedChatSmileneutral)
	ON_BN_CLICKED(IDC_CHAT_SMILEGLASSES, &CFakedChat::OnBnClickedChatSmileglasses)
	ON_BN_CLICKED(IDC_CHAT_SMILELOL, &CFakedChat::OnBnClickedChatSmilelol)
	ON_BN_CLICKED(IDC_CHAT_SMILECRAZY, &CFakedChat::OnBnClickedChatSmilecrazy)
	ON_WM_CLOSE()
	ON_MESSAGE(WM_CHAT_MOUSE, &CFakedChat::OnMouseMsg)
	ON_MESSAGE(WM_CHAT_KBRD, &CFakedChat::OnKeyboardMsg)
	ON_BN_CLICKED(IDC_CHAT_CLOSE, &CFakedChat::OnBnClickedChatClose)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CFakedChat message handlers

void CFakedChat::OnBnClickedChatSendbutton()
{
	UpdateData();
	m_chatBoardText		+= _T(">> ");
	m_chatBoardText		+= m_messageForSend;
	m_chatBoardText		+= _T("\r\n");
	m_messageForSend	= _T("");
	UpdateData(FALSE);
}

void CFakedChat::OnBnClickedChatSmileglad()
{
	UpdateData();
	m_messageForSend += _T(" :-) ");
	UpdateData(FALSE);
}

void CFakedChat::OnBnClickedChatSmilesad()
{
	UpdateData();
	m_messageForSend += _T(" :-( ");
	UpdateData(FALSE);
}

void CFakedChat::OnBnClickedChatSmileneutral()
{
	UpdateData();
	m_messageForSend += _T(" :-| ");
	UpdateData(FALSE);
}

void CFakedChat::OnBnClickedChatSmileglasses()
{
	UpdateData();
	m_messageForSend += _T(" B-) ");
	UpdateData(FALSE);
}

void CFakedChat::OnBnClickedChatSmilelol()
{
	UpdateData();
	m_messageForSend += _T(" :-D ");
	UpdateData(FALSE);
}

void CFakedChat::OnBnClickedChatSmilecrazy()
{
	UpdateData();
	m_messageForSend += _T(" %-7 ");
	UpdateData(FALSE);
}

void CFakedChat::OnOK()
{
}

BOOL CFakedChat::Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
{
	BOOL result = CDialog::Create(lpszTemplateName, pParentWnd);
	
	::SetWindowLong(m_hWnd,GWL_EXSTYLE,GetWindowLong(m_hWnd,GWL_EXSTYLE)|WS_EX_LAYERED|WS_EX_TRANSPARENT|WS_EX_TOPMOST);
	::SetLayeredWindowAttributes(m_hWnd,RGB(255,0,0),200,LWA_ALPHA|LWA_COLORKEY);
	RECT Rect;
	::GetWindowRect(m_hWnd, &Rect);
	::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, Rect.right - Rect.left, Rect.bottom - Rect.top, SWP_NOREPOSITION | SWP_NOSIZE);
	
	PrepareMap();
	return result;
}

void CFakedChat::OnClose()
{
	::PostMessage(GetParent()->m_hWnd,WM_CHAT_IS_CLOSE,(WPARAM)m_hWnd,0);
	
	CDialog::OnClose();
}

LRESULT CFakedChat::OnMouseMsg(WPARAM wparam, LPARAM lparam)
{
//	OutputDebugString("MOUSE_MESSAGE\n");
	boost::scoped_ptr<MOUSE_INPUT_DATA> data(reinterpret_cast<PMOUSE_INPUT_DATA>(wparam));
//	char buffer[MAX_PATH];
//	memset(buffer, 0, MAX_PATH);
//	POINT point;
//	point.x = data->LastX;
//	point.y = data->LastY;
//	ScreenToClient(&point);
//	sprintf(buffer, "Coords (%d, %d)\n", point.x, point.y);
//	OutputDebugString(buffer);


	if(MOUSE_LEFT_BUTTON_DOWN == (data->ButtonFlags & MOUSE_LEFT_BUTTON_DOWN))
	{
		if(!m_leftButton)
		{
			POINT point;
			point.x = data->LastX;
			point.y = data->LastY;
			ScreenToClient(&point);
			m_leftButton = true;
			int ctrl = GetControlByPoint(point);
			switch(ctrl)
			{
			case IDC_CHAT_SENDBUTTON:
				OnBnClickedChatSendbutton();
				break;
			case IDC_CHAT_SMILEGLAD:
				OnBnClickedChatSmileglad();
				break;
			case IDC_CHAT_SMILESAD:
				OnBnClickedChatSmilesad();
				break;
			case IDC_CHAT_SMILENEUTRAL:
				OnBnClickedChatSmileneutral();
				break;
			case IDC_CHAT_SMILEGLASSES:
				OnBnClickedChatSmileglasses();
				break;
			case IDC_CHAT_SMILELOL:
				OnBnClickedChatSmilelol();
				break;
			case IDC_CHAT_SMILECRAZY:
				OnBnClickedChatSmilecrazy();
				break;
			case IDC_CHAT_CLOSE:
				OnBnClickedChatClose();
				break;
			}
		}
	}

	if(MOUSE_LEFT_BUTTON_UP == (data->ButtonFlags & MOUSE_LEFT_BUTTON_UP))
	{
		m_leftButton = false;
	}
	return 0;
}

LRESULT CFakedChat::OnKeyboardMsg(WPARAM wparam, LPARAM lparam)
{
	boost::scoped_ptr<KEYBOARD_INPUT_DATA> data(reinterpret_cast<PKEYBOARD_INPUT_DATA>(wparam));
/*
	char buf[MAX_PATH];
	memset(buf, 0, MAX_PATH);
	sprintf(buf, "KBRD -> Flags(%d), Code(%d)\n", data->Flags, data->MakeCode);
	OutputDebugString(buf);
*/
	if(!m_keyboardLoaded)
	{
		GetKeyboardState(m_keyboard);
		m_keyboardLoaded = true;
	}


	if(KEY_MAKE == (data->Flags & 1))
	{
		UINT vKey = MapVirtualKey(data->MakeCode, 1);
		ULONG scan = data->MakeCode;
		scan = scan << 16;
		LPARAM param = scan + 1;
	
		if((KEY_E0 == (data->Flags & KEY_E0)) || (KEY_E1 == (data->Flags & KEY_E1)))
			param |= 0x01000000;

		WORD code = 0;
		int res = ToAscii(vKey, data->MakeCode, m_keyboard, &code, 0);
		if(res)
		{
/*
			memset(buf, 0, MAX_PATH);
			sprintf(buf, "WM_CHAR (%d) sent\n", code);
			OutputDebugString(buf);
*/
			::PostMessage(m_sendBox.m_hWnd, WM_CHAR, code, param);
		}
		else
		{
//			OutputDebugString("WM_KEYDOWN sent\n");
			::PostMessage(m_sendBox.m_hWnd, WM_KEYDOWN, vKey, param);
		}

		if((VK_CAPITAL == vKey) || (VK_NUMLOCK == vKey) || (VK_SCROLL == vKey))
		{
			if(1 & m_keyboard[vKey])
				m_keyboard[vKey] = 0x80;
			else
				m_keyboard[vKey] = 0x81;
		}
		else
			m_keyboard[vKey] = 0x80;

		if(VK_RETURN == vKey)
			OnBnClickedChatSendbutton();
	}

	if(KEY_BREAK == (data->Flags & KEY_BREAK))
	{
		UINT vKey = MapVirtualKey(data->MakeCode, 1);
		ULONG scan = data->MakeCode;
		scan = scan << 16;
		LPARAM param = 0xC0000001 | scan;
	
		if((KEY_E0 == (data->Flags & KEY_E0)) || (KEY_E1 == (data->Flags & KEY_E1)))
			param |= 0x01000000;
	
		WORD code = 0;
		int res = ToAscii(vKey, data->MakeCode, m_keyboard, &code, 0);
		if(!res)
		{
//			OutputDebugString("WM_KEYUP sent\n");
			::PostMessage(m_sendBox.m_hWnd, WM_KEYUP, vKey, param);
		}
		if((VK_CAPITAL == vKey) || (VK_NUMLOCK == vKey) || (VK_SCROLL == vKey))
		{
			if(1 & m_keyboard[vKey])
				m_keyboard[vKey] = 1;
			else
				m_keyboard[vKey] = 0;
		}
		else
			m_keyboard[vKey] = 0;
	}

	return 0;
}

void CFakedChat::OnBnClickedChatClose()
{
	PostMessage(WM_CLOSE);
}

void CFakedChat::PrepareMap()
{
	std::vector<int> tmp(8);
	tmp[0] = IDC_CHAT_SENDBUTTON;
	tmp[1] = IDC_CHAT_SMILEGLAD;
	tmp[2] = IDC_CHAT_SMILESAD;
	tmp[3] = IDC_CHAT_SMILENEUTRAL;
	tmp[4] = IDC_CHAT_SMILEGLASSES;
	tmp[5] = IDC_CHAT_SMILELOL;
	tmp[6] = IDC_CHAT_SMILECRAZY;
	tmp[7] = IDC_CHAT_CLOSE;

	for(int i = 0; i < 8; ++i)
	{
		HWND item = ::GetDlgItem(m_hWnd, tmp[i]);
		RECT rect;
		::GetWindowRect(item, &rect);
		POINT left, right;
		left.x = rect.left;
		left.y = rect.top;
		right.x = rect.right;
		right.y = rect.bottom;
		ScreenToClient(&left);
		ScreenToClient(&right);
		rect.left = left.x;
		rect.top = left.y;
		rect.right = right.x;
		rect.bottom = right.y;
		
		m_controls[i] = CtrlPair(rect, tmp[i]);
	}

}

int CFakedChat::GetControlByPoint(POINT point)
{
	for(int i = 0; i < 8; ++i)
	{
		if(TRUE == PtInRect(&m_controls[i].first, point))
			return m_controls[i].second;
	}
	return 0;
}

void CFakedChat::OnShowWindow(BOOL bShow, UINT)
{
}
