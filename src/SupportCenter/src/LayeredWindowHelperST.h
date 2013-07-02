//
//	Class:		CLayeredWindowHelperST
//
#ifndef _LAYEREDWINDOWHELPERST_H_
#define _LAYEREDWINDOWHELPERST_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WS_EX_LAYERED
#define WS_EX_LAYERED           0x00080000
#define LWA_COLORKEY            0x00000001
#define LWA_ALPHA               0x00000002
#endif

class CLayeredWindowHelperST  
{
public:
	CLayeredWindowHelperST();
	virtual ~CLayeredWindowHelperST();

	LONG AddLayeredStyle(HWND hWnd);
	LONG RemoveLayeredStyle(HWND hWnd);
	BOOL SetLayeredWindowAttributes(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);
	BOOL SetTransparentPercentage(HWND hWnd, BYTE byPercentage);

	static short GetVersionI()		{return 11;}
	static LPCTSTR GetVersionC()	{return (LPCTSTR)_T("1.1");}

private:
	typedef BOOL (WINAPI* lpfnSetLayeredWindowAttributes)(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);

	HMODULE		m_hDll;
};

#endif
