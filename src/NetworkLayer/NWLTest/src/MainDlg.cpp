// MainDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NetworkLayerTest.h"
#include "MainDlg.h"
#include ".\maindlg.h"

// CMainDlg dialog

IMPLEMENT_DYNAMIC(CMainDlg, CDialog)
CMainDlg::CMainDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMainDlg::IDD, pParent)
{
	Log.RegisterLog(new CListLog(&m_logList));
	Log.RegisterLog(new cFileLog());
}

CMainDlg::~CMainDlg()
{
}

void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TABCONTROL, m_tabCtrl);
	DDX_Control(pDX, IDC_LOG_LIST, m_logList);
}

BOOL CMainDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_tabCtrl.InsertItem(0,"CDirectNetworkStream");
	m_tabCtrl.InsertItem(1,"CNATTraversingUDPNetworkStream");
	m_tabCtrl.InsertItem(2,"CRelayedNetworkStream");
	m_tabCtrl.InsertItem(3,"CStreamFactoryD");
	m_tabCtrl.InsertItem(4,"Logging");

	m_directStreamDlg.Create(m_directStreamDlg.IDD, &m_tabCtrl);
	m_directStreamDlg.SetParent(&m_tabCtrl);
	m_directStreamDlg.ShowWindow(SW_SHOW);

	m_NATTraversalDlg.Create(m_NATTraversalDlg.IDD, &m_tabCtrl);
	m_NATTraversalDlg.SetParent(&m_tabCtrl);
	m_NATTraversalDlg.ShowWindow(SW_HIDE);

	m_streamFactoryDlg.Create(m_streamFactoryDlg.IDD, &m_tabCtrl);
	m_streamFactoryDlg.SetParent(&m_tabCtrl);
	m_streamFactoryDlg.ShowWindow(SW_HIDE);

	m_RelayedStreamDlg.Create(m_RelayedStreamDlg.IDD, &m_tabCtrl);
	m_RelayedStreamDlg.SetParent(&m_tabCtrl);
	m_RelayedStreamDlg.ShowWindow(SW_HIDE);


	m_logList.ShowWindow(SW_HIDE);
	Log.Add(_MESSAGE_,"NetworkLayerTest started");
	return TRUE;
}


BEGIN_MESSAGE_MAP(CMainDlg, CDialog)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TABCONTROL, OnTcnSelchangeTabcontrol)
	ON_COMMAND(ID_COMMANDS_CLEARLOG, OnCommandsClearlog)
	ON_COMMAND(ID_COMMANDS_SAVELOGTOFILE, OnCommandsSavelogtofile)
END_MESSAGE_MAP()


// CMainDlg message handlers

void CMainDlg::OnTcnSelchangeTabcontrol(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	switch(m_tabCtrl.GetCurFocus())
	{
		case 0: //Direct stream;
			m_RelayedStreamDlg.ShowWindow(SW_HIDE);
			m_directStreamDlg.ShowWindow(SW_SHOW);
			m_streamFactoryDlg.ShowWindow(SW_HIDE);
			m_NATTraversalDlg.ShowWindow(SW_HIDE);
			m_logList.ShowWindow(SW_HIDE);
			break;
		case 1: //NAT traversal stream;
			m_RelayedStreamDlg.ShowWindow(SW_HIDE);
			m_directStreamDlg.ShowWindow(SW_HIDE);
			m_NATTraversalDlg.ShowWindow(SW_SHOW);
			m_streamFactoryDlg.ShowWindow(SW_HIDE);
			m_logList.ShowWindow(SW_HIDE);
			break;
		case 2: //Relayed stream
			m_RelayedStreamDlg.ShowWindow(SW_SHOW);
			m_directStreamDlg.ShowWindow(SW_HIDE);
			m_NATTraversalDlg.ShowWindow(SW_HIDE);
			m_streamFactoryDlg.ShowWindow(SW_HIDE);
			m_logList.ShowWindow(SW_HIDE);
			break;
		case 3: //Stream factory;
			m_RelayedStreamDlg.ShowWindow(SW_HIDE);
			m_directStreamDlg.ShowWindow(SW_HIDE);
			m_NATTraversalDlg.ShowWindow(SW_HIDE);
			m_streamFactoryDlg.ShowWindow(SW_SHOW);
			m_logList.ShowWindow(SW_HIDE);
			break;
		default:
			m_RelayedStreamDlg.ShowWindow(SW_HIDE);
			m_directStreamDlg.ShowWindow(SW_HIDE);
			m_NATTraversalDlg.ShowWindow(SW_HIDE);
			m_streamFactoryDlg.ShowWindow(SW_HIDE);
			m_logList.ShowWindow(SW_SHOW);
	}
	*pResult = 0;
}

void CMainDlg::OnCommandsClearlog()
{
TRY_CATCH
	m_logList.ResetContent();
CATCH_THROW("CMainDlg::OnCommandsClearlog")
}

void CMainDlg::OnCommandsSavelogtofile()
{
TRY_CATCH
	CFileDialog fileDialog(	FALSE /*save*/,
							"log",
							"NetworkLayerTest");
	fileDialog.DoModal();
	tstring fileName = static_cast<LPCSTR>(fileDialog.GetPathName());
	HANDLE hFile = CreateFile(	fileName.c_str(),
								GENERIC_WRITE,
								FILE_SHARE_READ,
								NULL,
								CREATE_ALWAYS,
								0,
								NULL );
	if (hFile == INVALID_HANDLE_VALUE)
		throw MCStreamException_Win("failed to CreateFile",GetLastError());
	CString str;
	DWORD written;
	for(int i=0; 
		i<m_logList.GetCount();
		++i)
	{
		m_logList.GetText(i, str);
		str = "\r\n" + str;
		if (!WriteFile(hFile,(LPCSTR)str,str.GetLength()+1,&written,FALSE))
			throw MCStreamException_Win("failed to WriteFile",GetLastError());
	}
	CloseHandle(hFile);
CATCH_LOG("CMainDlg::OnCommandsSavelogtofile")
}