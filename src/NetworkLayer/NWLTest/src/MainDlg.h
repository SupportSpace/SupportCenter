#pragma once
#include "afxcmn.h"
#include "NetworkLayerTestDlg.h"
#include "StreamFactory.h"
#include "afxwin.h"
#include <AidLib/Logging/cLog.h>
#include "CNATTraversalDialog.h"
#include "CRelayedStreamDlg.h"
#include <AidLib/CThread/CThreadLS.h>

// CMainDlg dialog

class CMainDlg : public CDialog
{
	/// Logging into ListBox
	class CListLog : public cLog, public CThread
	{
	private:

		/// listbox for logging reference
		CListBox* m_listBox;
	public:
		/// Initializes logger
		/// @param listBox listbox for logging
		CListLog(CListBox* listBox)
			: m_listBox(listBox), CThread()
		{
			Start();
		}

		~CListLog()
		{
			PostThreadMessage(GetTid(),WM_QUIT,0,0);
		}

		/// Clears log
		void Clear()
		{
			m_listBox->ResetContent();
		}
	protected:
		virtual void AddList(const cLog::cEventDesc &EventDesc, const TCHAR *Item, ...) throw( )
		{
		try
		{
			va_list vl;
			tstring str;

			SYSTEMTIME SysTime;
			GetLocalTime(&SysTime);
			DWORD DateFlags;
			tstring DateFormat;
			DateFlags=DATE_LONGDATE;
			DateFormat = "dd.MM.yyyy";
			TCHAR Buf[MAX_PATH];
			tstring TimeStr;
			if (!GetDateFormat(LOCALE_USER_DEFAULT,0,&SysTime,DateFormat.c_str(),Buf,MAX_PATH))
				TimeStr = _T("Invalid date");
			else
				TimeStr = Buf;
			TimeStr+=_T(" ");
				if (!GetTimeFormat(LOCALE_USER_DEFAULT,0,&SysTime,NULL,Buf,MAX_PATH))
			TimeStr += _T("Invalid time");
				else
			TimeStr += Buf;
			TimeStr += "> ";


			for(va_start(vl, Item); Item; Item=va_arg(vl, PTCHAR))
			{
				str += Item;
			}
			va_end(vl);

			TimeStr += str;

			//Preventing simultaneous execution of AddString functions
			EnterCriticalSection(&AddStringCS);

			try
			{
				//Calling virtual function of adding string to destinate log('s)
				AddString(TimeStr.c_str(), EventDesc.getSeverity());
			}
			catch(...)
			{
				try
				{
					AddString(_T("cLog: Exception during AddString happened"),_EXCEPTION);
				}
				catch(...)
				{
				}
			}

			LeaveCriticalSection(&AddStringCS);
		}
		catch(...)
		{
		}
		}

		/// CLog AddString redefinition
		virtual void AddString(const TCHAR *LogString, const eSeverity Severity) throw()
		{
			TCHAR seps[]   = _T("\n");
			TCHAR *nextToken;
			std::auto_ptr<char> str = std::auto_ptr<char>(new char[strlen(LogString)+1]);
			strcpy_s(str.get(),strlen(LogString)+1,LogString);
			for (	PTCHAR token = _tcstok_s( str.get() , seps, &nextToken );
					token;
					token = _tcstok_s(NULL, seps, &nextToken) )
			{
				char* str = _strdup(token);
				PostThreadMessage(GetCurrentThreadId(),WM_USER,0,reinterpret_cast<LPARAM>(str));
				//m_listBox->AddString(token);			
			}
		}

		virtual void Execute(void*)
		{
			SET_THREAD_LS;

		TRY_CATCH
			for(MSG msg;
				GetMessage(&msg,NULL,0,0) &&
				!Terminated();)
			{
				switch(msg.message)
				{
					case WM_USER:
						if (IsWindow(m_listBox->m_hWnd))
							m_listBox->AddString(reinterpret_cast<LPCSTR>(msg.lParam));
						delete reinterpret_cast<char*>(msg.lParam);
						break;
					default:
						break;
				}
			}
		CATCH_LOG("CRCTest2Dlg::CListLog")
		}
	};

	DECLARE_DYNAMIC(CMainDlg)
public:
	CMainDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMainDlg();

// Dialog Data
	enum { IDD = IDD_MAIN_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	CTabCtrl m_tabCtrl;
	CNetworkLayerTestDlg m_directStreamDlg;
	CStreamFactoryDlg m_streamFactoryDlg;
	CNATTraversalDialog m_NATTraversalDlg;
	CRelayedStreamDlg m_RelayedStreamDlg;
public:
	afx_msg void OnTcnSelchangeTabcontrol(NMHDR *pNMHDR, LRESULT *pResult);
	CListBox m_logList;
	afx_msg void OnCommandsClearlog();
	afx_msg void OnCommandsSavelogtofile();
};
