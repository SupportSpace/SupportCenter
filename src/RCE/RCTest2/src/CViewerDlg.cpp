// CViewerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RCTest2.h"
#include "CViewerDlg.h"
#include "cviewerdlg.h"
#include <RCEngine/Streaming/CInFileStreamTimeStamped.h>
#include <RCEngine/Streaming/COutStreamGZipped.h>


// CViewerDlg dialog

IMPLEMENT_DYNAMIC(CViewerDlg, CDialog)
CViewerDlg::CViewerDlg(CWnd* pParent, boost::shared_ptr<CAbstractStream> stream)
	: CDialog(CViewerDlg::IDD, pParent), CRCViewer(stream, NULL)
{
TRY_CATCH
	Create(IDD);
	///m_hDesctop = m_hWnd;
	SetDesctopHandle( m_hWnd );
	ShowWindow(SW_SHOW);
CATCH_THROW("CViewerDlg::CViewerDlg")
}

CViewerDlg::~CViewerDlg()
{
}

void CViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CViewerDlg, CDialog)
	ON_COMMAND(IDCANCEL, OnCancel)
	ON_COMMAND(IDOK, OnOK)
	ON_COMMAND(ID_COMMANDS_START, OnCommandsStart)
	ON_COMMAND(ID_COMMANDS_STOP, OnCommandsStop)
	ON_COMMAND(ID_OPTIONS_VIEWONLY, OnOptionsViewonly)
	ON_COMMAND(ID_OPTIONS_VISUALPOINTER, OnOptionsVisualpointer)
	ON_COMMAND(ID_OPTIONS_SCALEMODE, OnOptionsScalemode)
	ON_COMMAND(ID_OPTIONS_FULLSCREENMODE, OnOptionsFullscreenmode)
	ON_COMMAND(ID_OPTIONS_SCROLLMODE, OnOptionsScrollmode)
	ON_COMMAND(ID_REPLAY_START, OnReplayStart)
	ON_COMMAND(ID_REPLAY_STOP, OnReplayStop)
	ON_COMMAND(ID_REPLAY_PAUSE, OnReplayPause)
	ON_COMMAND(ID_FASTFORWARD_X1, OnFastforwardX1)
	ON_COMMAND(ID_FASTFORWARD_X2, OnFastforwardX2)
	ON_COMMAND(ID_FASTFORWARD_X4, OnFastforwardX4)
	ON_COMMAND(ID_FASTFORWARD_X8, OnFastforwardX8)
	ON_COMMAND(ID_FASTFORWARD_X20, OnFastforwardX20)
	ON_COMMAND(ID_FASTFORWARD_FASTEST, OnFastforwardFastest)
	ON_COMMAND(ID_ZOOM_STRETCHING, OnZoomStretching)
	ON_COMMAND(ID_OPTIONS_SETSHADOWSTREAM, OnOptionsSetshadowstream)
	ON_COMMAND(ID_COMMANDS_SENDCTRL, &CViewerDlg::OnCommandsSendctrl)
	ON_COMMAND(ID_COMMANDS_SENDGARBAGEINTOSTREAM, &CViewerDlg::OnCommandsSendgarbageintostream)
END_MESSAGE_MAP()


// CViewerDlg message handlers
BOOL CViewerDlg::OnInitDialog()
{
	BOOL result;
TRY_CATCH
	result = CDialog::OnInitDialog();
	/// Setting hWnd to CRCViewer
CATCH_LOG("CViewerDlg::OnInitDialog")
	return result;
}

void CViewerDlg::NotifySessionStarted()
{
TRY_CATCH
	ShowWindow(SW_SHOW);
	Log.Add(_MESSAGE_,"Viewer session started");
CATCH_THROW("CViewerDlg::NotifySessionStarted")
}

void CViewerDlg::NotifySessionStopped(ESessionStopReason ReasonCode)
{
TRY_CATCH
	PostMessage(WM_COMMAND,IDCANCEL,0);
	tstring reason;
	switch(ReasonCode)
	{
		case LOCAL_STOP:
			reason = "Local stop";
			break;
		case REMOTE_STOP:
			reason = "Remote stop";
			break;
		case STREAM_ERROR:
			reason = "Stream error";
			break;
		default:
			reason = "unknown";
			break;
	}
	Log.Add(_MESSAGE_,"Viewer session stopped. Reason(%s)",reason.c_str());
CATCH_THROW("CViewerDlg::NotifySessionStopped")
}

void CViewerDlg::OnCancel()
{
TRY_CATCH
	UnblockFileStream();
	CDialog::OnCancel();
	delete this;
CATCH_LOG("CViewerDlg::OnCancel")
}

void CViewerDlg::OnOK()
{
TRY_CATCH
	UnblockFileStream();
	CDialog::OnOK();
	delete this;
CATCH_LOG("CViewerDlg::OnOK")
}

void CViewerDlg::OnCommandsStart()
{
TRY_CATCH
	Start();
CATCH_LOG("CViewerDlg::OnCommandsStart")
}

void CViewerDlg::OnCommandsStop()
{
TRY_CATCH
	UnblockFileStream();
	Stop();
CATCH_LOG("CViewerDlg::OnCommandsStop")
}

void CViewerDlg::UnblockFileStream()
{
TRY_CATCH
	CInFileStreamTimeStamped *fileStream = dynamic_cast<CInFileStreamTimeStamped*>(m_stream->GetMainStream().get());
	if (fileStream)
	{
		try
		{
			fileStream->Start();
		}catch(...)
		{
		}
	}
CATCH_LOG("CViewerDlg::UnblockFileStream")
}

void CViewerDlg::OnOptionsViewonly()
{
TRY_CATCH
	///TODO set session mode viewonly
	SetSessionMode(VIEW_ONLY,true);
CATCH_LOG("CViewerDlg::OnOptionsViewonly")
}

void CViewerDlg::OnOptionsVisualpointer()
{
TRY_CATCH
	///TODO set session mode visual pointer
	SetSessionMode(VISUAL_POINTER,true);
CATCH_LOG("CViewerDlg::OnOptionsVisualpointer")
}

void CViewerDlg::OnOptionsScalemode()
{
TRY_CATCH
	SetDisplayMode(SCALE_MODE);
CATCH_LOG("CViewerDlg::OnOptionsScalemode")
}

void CViewerDlg::OnOptionsFullscreenmode()
{
TRY_CATCH
	SetDisplayMode(FULLSCREEN_MODE);
CATCH_LOG("CViewerDlg::OnOptionsFullscreenmode")
}

void CViewerDlg::OnOptionsScrollmode()
{
TRY_CATCH
	SetDisplayMode(SCROLL_MODE);
CATCH_LOG("CViewerDlg::OnOptionsScrollmode")
}

void CViewerDlg::OnReplayStart()
{
TRY_CATCH
	CInFileStreamTimeStamped* stream = dynamic_cast<CInFileStreamTimeStamped*>(m_stream->GetMainStream().get());
	if (!stream) throw MCException("This isn't replay session");
	stream->Start();
CATCH_LOG("CViewerDlg::OnReplayStart")
}

void CViewerDlg::OnReplayStop()
{
TRY_CATCH
	CInFileStreamTimeStamped* stream = dynamic_cast<CInFileStreamTimeStamped*>(m_stream->GetMainStream().get());
	if (!stream) throw MCException("This isn't replay session");
	stream->Stop();
CATCH_LOG("CViewerDlg::OnReplayStop")
}

void CViewerDlg::OnReplayPause()
{
TRY_CATCH
	CInFileStreamTimeStamped* stream = dynamic_cast<CInFileStreamTimeStamped*>(m_stream->GetMainStream().get());
	if (!stream) throw MCException("This isn't replay session");
	stream->Pause();
CATCH_LOG("CViewerDlg::OnReplayPause")
}

void CViewerDlg::OnFastforwardX1()
{
TRY_CATCH
	CInFileStreamTimeStamped* stream = dynamic_cast<CInFileStreamTimeStamped*>(m_stream->GetMainStream().get());
	if (!stream) throw MCException("This isn't replay session");
	stream->SetDelayFactor(1);
CATCH_LOG("CViewerDlg::OnFastforwardX1")
}

void CViewerDlg::OnFastforwardX2()
{
TRY_CATCH
	CInFileStreamTimeStamped* stream = dynamic_cast<CInFileStreamTimeStamped*>(m_stream->GetMainStream().get());
	if (!stream) throw MCException("This isn't replay session");
	stream->SetDelayFactor(1.0 / 2.0);
CATCH_LOG("CViewerDlg::OnFastforwardX2")
}

void CViewerDlg::OnFastforwardX4()
{
TRY_CATCH
	CInFileStreamTimeStamped* stream = dynamic_cast<CInFileStreamTimeStamped*>(m_stream->GetMainStream().get());
	if (!stream) throw MCException("This isn't replay session");
	stream->SetDelayFactor(1.0 / 4.0);
CATCH_LOG("CViewerDlg::OnFastforwardX4")
}

void CViewerDlg::OnFastforwardX8()
{
TRY_CATCH
	CInFileStreamTimeStamped* stream = dynamic_cast<CInFileStreamTimeStamped*>(m_stream->GetMainStream().get());
	if (!stream) throw MCException("This isn't replay session");
	stream->SetDelayFactor(1.0 / 8.0);
CATCH_LOG("CViewerDlg::OnFastforwardX8")
}

void CViewerDlg::OnFastforwardX20()
{
TRY_CATCH
	CInFileStreamTimeStamped* stream = dynamic_cast<CInFileStreamTimeStamped*>(m_stream->GetMainStream().get());
	if (!stream) throw MCException("This isn't replay session");
	stream->SetDelayFactor(static_cast<float>(1.0 / 20.0));
CATCH_LOG("CViewerDlg::OnFastforwardX20")
}

void CViewerDlg::OnFastforwardFastest()
{
TRY_CATCH
	CInFileStreamTimeStamped* stream = dynamic_cast<CInFileStreamTimeStamped*>(m_stream->GetMainStream().get());
	if (!stream) throw MCException("This isn't replay session");
	stream->SetDelayFactor(0);
CATCH_LOG("CViewerDlg::OnFastforwardFastest")
}

void CViewerDlg::SetRemoteDesktopSize(const int width, const int height)
{
TRY_CATCH
	m_cx = width;
	m_cy = height;
CATCH_LOG("CViewerDlg::SetRemoteDesktopSize")
}

void CViewerDlg::OnZoomStretching()
{
TRY_CATCH
	RECT cRect,wRect;
	GetClientRect(&cRect);
	GetWindowRect(&wRect);
	SetDisplayMode(SCROLL_MODE);	
	SetWindowPos(	0,0,0,
					m_cx + (wRect.right - wRect.left) - (cRect.right - cRect.left), 
					m_cy + (wRect.bottom - wRect.top)-(cRect.bottom - cRect.top),
					SWP_NOMOVE | SWP_NOOWNERZORDER );
CATCH_LOG("CViewerDlg::OnZoomStretching")
}

BOOL CViewerDlg::PreTranslateMessage(MSG* pMsg)
{
TRY_CATCH

	if(WM_KEYDOWN == pMsg->message)
    {
		switch(pMsg->wParam)
		{
			case VK_TAB:
			case VK_LEFT:
			case VK_UP:
			case VK_RIGHT:
			case VK_DOWN:
			case VK_RETURN:
			case VK_ESCAPE:
	            return FALSE;
		}
    }
    return CDialog::PreTranslateMessage(pMsg);
CATCH_LOG("CViewerDlg::PreTranslateMessage")
	return TRUE;
}
void CViewerDlg::OnOptionsSetshadowstream()
{
TRY_CATCH

	CFileDialog fileDialog(	FALSE /*save*/,
							"rce",
							"session");
	fileDialog.DoModal();
	tstring fileName = static_cast<LPCSTR>(fileDialog.GetPathName());
	COutStreamGZipped *fileStream = new COutStreamGZipped(fileName);
	SetShadowStream(boost::shared_ptr<CAbstractStream>(fileStream));

CATCH_LOG("CViewerDlg::OnOptionsSetshadowstream")
}

void CViewerDlg::OnCommandsSendctrl()
{
TRY_CATCH
	SendCtrlAltDel();
CATCH_LOG()
}

void CViewerDlg::OnCommandsSendgarbageintostream()
{
TRY_CATCH

	if (m_stream.get() != NULL)
	{
		srand(GetTickCount());
		char buf[MAX_PATH];
		for(int i=0; i<sizeof(buf); ++i)
		{
			buf[i] = rand() % 255;
		}
		m_stream->Send(buf, sizeof(buf));
	}
CATCH_LOG()
}
