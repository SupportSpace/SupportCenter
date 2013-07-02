#pragma once
#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

class CAboutDlgInfo
{
public:
	CAboutDlgInfo(void);
	~CAboutDlgInfo(void);
	
	CString	 GetSupportMessngerVersion()
	{
		return m_csVersion;
	};

private:
	void	 ReadSupportMessngerVersion();
	CString  m_csVersion;
};
