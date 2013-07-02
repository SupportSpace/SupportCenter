/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CRCViewerUIMediator.cpp
///
///  CRCViewerUIMediator, mediator between CCoRCViewer and his UI
///
///  @author "Archer Software" Kirill Solovyov @date 18.06.2006
///
////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "CRCViewerUIMediator.h"
#include "AidLib/CException/CException.h"

/// TODO: gather this in one place
#define rfbEncodingRaw 0
#define rfbEncodingCopyRect 1
#define rfbEncodingRRE 2
#define rfbEncodingCoRRE 4
#define rfbEncodingHextile 5
#define rfbEncodingZlib    6
#define rfbEncodingTight   7
#define rfbEncodingZlibHex 8
#define rfbEncodingUltra	9
#define rfbEncodingZRLE 16
#define rfbEncodingCache					0xFFFF0000
#define rfbEncodingCacheEnable				0xFFFF0001
#define rfbEncodingXOR_Zlib					0xFFFF0002
#define rfbEncodingXORMonoColor_Zlib		0xFFFF0003
#define rfbEncodingXORMultiColor_Zlib		0xFFFF0004
#define rfbEncodingSolidColor				0xFFFF0005
#define rfbEncodingXOREnable				0xFFFF0006
#define rfbEncodingCacheZip					0xFFFF0007
#define rfbEncodingSolMonoZip				0xFFFF0008
#define rfbEncodingUltraZip					0xFFFF0009

CRCViewerUIMediator::CRCViewerUIMediator(CCoRCViewer* owner)
	:	CSkinnedElement(NULL, boost::shared_ptr<CSkinsImageList>(new CSkinsImageList())),
		m_viewer(owner),
		m_status(false, EDSS_STATES_COUNT),
		m_sessionDlg(this),
		m_statusBarDlg(this),
		m_firstTimeCreation(true),
		m_displayMode(SCALE_MODE)
{
TRY_CATCH
	memset(m_uiStatusIcons,0,sizeof(m_uiStatusIcons));
CATCH_LOG()
}

CRCViewerUIMediator::~CRCViewerUIMediator(void)
{
TRY_CATCH
CATCH_LOG()
}

void CRCViewerUIMediator::CreateUI(HWND parentWnd)
{
TRY_CATCH
	//TODO recreaction on workbench by closing
	if(!m_firstTimeCreation)
		throw MCException("UI was created already. Only one time allow creation.");
	m_firstTimeCreation=false;
	if(m_parentWnd.IsWindow())
		throw MCException("UI has been created already");
	if(!::IsWindow(parentWnd))
		throw MCException("Parent window incorrect");
	m_parentWnd.Attach(parentWnd);
	//+create fonts
	m_logFont.reset(new LOGFONT);
	m_logFontBold.reset(new LOGFONT);
	HFONT fontParent=m_parentWnd.GetFont();
	ATLASSERT(::GetObjectType(fontParent) == OBJ_FONT);
	GetObject(fontParent, sizeof(LOGFONT),m_logFont.get());
	memcpy(m_logFontBold.get(),m_logFont.get(),sizeof(LOGFONT));
	m_logFontBold->lfWeight = FW_BOLD;
	m_logFont->lfHeight = -11;
	m_logFontBold->lfHeight = -11;
	//-create fonts
	m_sessionDlg.m_logFont=m_logFont;
	m_sessionDlg.m_logFontBold=m_logFontBold;
	m_sessionDlg.Create(parentWnd);
	// Attaching events
	m_sessionDlg.m_showPropertiesBtnClick=boost::bind(&CRCViewerUIMediator::OnPropertiesBtnClick,this);
	m_sessionDlg.m_startBtnClick=boost::bind(&CRCViewerUIMediator::OnSessionStartBtnClick,this,_1);
	m_sessionDlg.m_permissionCmbSelChange=boost::bind(&CRCViewerUIMediator::OnSessionPermissionCmbSelChange,this,_1);
	m_sessionDlg.m_propertiesDlg.m_applyBtnClick=boost::bind(&CRCViewerUIMediator::OnPropertiesApplyBtnClick,this,_1,_2,_3);
	m_sessionDlg.m_propertiesDlg.m_comboSelChanged=boost::bind(&CRCViewerUIMediator::OnPropertiesComboboxSelChanged,this);
	m_sessionDlg.m_propertiesDlg.m_cadBtnClick=boost::bind(&CRCViewerUIMediator::OnCADBtnClick,this);
	CWindow axPanel=CWindow(parentWnd).GetDlgItem(IDC_RCVIEWERAXPANEL);
	if(!axPanel.m_hWnd)
		throw MCException_Win("RCViewerAXPanel obtaining failed");
	//TODO transfer to m_sessionDlg
	m_sessionDlg.m_propertiesDlg.m_logFont=m_logFont;
	m_sessionDlg.m_propertiesDlg.m_logFontBold=m_logFontBold;
	m_sessionDlg.m_propertiesDlg.Create(axPanel);
	m_sessionDlg.m_customerLbl.Text=m_peerId;
	//m_sessionDlg.m_customerLbl.RedrawWindow();

	m_statusBarDlg.m_logFont=m_logFont;
	m_statusBarDlg.m_logFontBold=m_logFontBold;
	m_statusBarDlg.Create(parentWnd);
	//+ status icons loading 
	/// resource identifer of statues icon
	const int uiStatusIconsId[EDSS_STATES_COUNT]={	IDR_UIS_PERMISSION_REQUEST_SEND,
													IDR_UIS_PERMISSION_RECIEVED,
													IDR_UIS_PERMISSION_DENIED,
													IDR_UIS_INSTALLATION_INPROGRESS,
													IDR_UIS_INSTALLATION_SUCCESSFUL,
													IDR_UIS_INSTALLATION_FAILED,
													IDR_UIS_SESSION_CONNECTING,
													IDR_UIS_SESSION_FAILED,
													IDR_UIS_SESSION_OFF,
													IDR_UIS_SESSION_ON};

	if (NULL != m_skinsImageList.get())
		m_skinsImageList->ImageFromRes(m_uiStatusIcons,EDSS_STATES_COUNT,uiStatusIconsId);
	else
		Log.Add(_WARNING_,_T("NULL images list while creating UI"));

	SetUIStatus(SDSViewerState(false, EDSS_INIFIAL) , _T("Desktop sharing - Off"));
	//- status icons loading
	
	/// Creating scrollbars
	GetVertScrollBar();
	GetHorSrcrollBar();
	/// Handling apply btn
	m_sessionDlg.m_propertiesDlg.m_applyBtn.EnableWindow(FALSE);

CATCH_THROW()
}
void CRCViewerUIMediator::DestroyUI()
{
TRY_CATCH
	if(!m_parentWnd.IsWindow())
		throw MCException("UI has been not created yet");
	m_sessionDlg.m_startBtnClick=NULL;
	m_sessionDlg.m_permissionCmbSelChange=NULL;
	m_sessionDlg.m_propertiesDlg.m_applyBtnClick=NULL;
	m_vscrollBar.DestroyWindow();
	m_hscrollBar.DestroyWindow();
	m_sessionDlg.m_propertiesDlg.DestroyWindow();
	m_sessionDlg.DestroyWindow();
	m_statusBarDlg.DestroyWindow();
	m_parentWnd.Detach();
CATCH_THROW()
}

LRESULT CRCViewerUIMediator::Size(WPARAM wParam, LPARAM lParam)
{
	//The low-order word of lParam specifies the new width of the client area. 
	//The high-order word of lParam specifies the new height of the client area. 
TRY_CATCH
	//TODO recreaction on workbench by closing
	if(!m_parentWnd.IsWindow())
		throw MCException("UI has been not created yet");
	CWindow parent;
	parent=m_sessionDlg.GetParent();
	if(!parent.m_hWnd)
		throw MCException_Win("RCViewerSessionDlg parent obtaining failed");

	// CRCViewerSessionDlg
	RECT sessionDlgRect;
	if(!m_sessionDlg.GetWindowRect(&sessionDlgRect))
		throw	MCException_Win("RCViewerSessionDlg window rect obtaining failed");
	if(!parent.ScreenToClient(&sessionDlgRect))
		throw MCException_Win("RCViewerSessionDlg parent ScreenToClient conversion failed");
	if(!m_sessionDlg.SetWindowPos(NULL,0,0,LOWORD(lParam),sessionDlgRect.bottom-sessionDlgRect.top,SWP_NOZORDER|SWP_NOACTIVATE))
		throw MCException_Win("RCViewerSessionDlg resizing failed");
	
	// CRCViewerStatusBarDlg
	RECT statusBarDlgRect;
	if(!m_statusBarDlg.GetWindowRect(&statusBarDlgRect))
		throw	MCException_Win("RCViewerStatusBarDlg window rect obtaining failed");
	if(!parent.ScreenToClient(&statusBarDlgRect))
		throw MCException_Win("RCViewerStatusBarDlg parent ScreenToClient conversion failed");
	if(!m_statusBarDlg.SetWindowPos(NULL,0,HIWORD(lParam)-(statusBarDlgRect.bottom-statusBarDlgRect.top),
																LOWORD(lParam),statusBarDlgRect.bottom-statusBarDlgRect.top,
																SWP_NOZORDER|SWP_NOACTIVATE))
		throw MCException_Win("RCViewerStatusBarDlg resizing failed");

	
	// RCViewerAXPanel
	CWindow axPanel=parent.GetDlgItem(IDC_RCVIEWERAXPANEL);
	if(!axPanel.m_hWnd)
		throw MCException_Win("RCViewerAXPanel obtaining failed");
	if(!axPanel.SetWindowPos(	NULL,0,sessionDlgRect.bottom-sessionDlgRect.top,
														LOWORD(lParam),HIWORD(lParam)-(sessionDlgRect.bottom-sessionDlgRect.top)-(statusBarDlgRect.bottom-statusBarDlgRect.top),
														SWP_NOZORDER|SWP_NOACTIVATE))
		throw MCException_Win("CRCViewerAXPanel resizing failed");

	// CRCViewerPropertiesDlg
	RECT propertiesDlgRect;
	RECT axPanelRect;
	if(!m_sessionDlg.m_propertiesDlg.GetWindowRect(&propertiesDlgRect))
		throw	MCException_Win("CRCViewerPropertiesDlg window rect obtaining failed");
	if(!axPanel.GetClientRect(&axPanelRect))
		throw	MCException_Win("AXPanel client rect obtaining failed");
	//if(!m_sessionDlg.m_propertiesDlg.SetWindowPos(NULL,LOWORD(lParam)/2-(propertiesDlgRect.right-propertiesDlgRect.left)/2,0,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE))
	int dx = GetSystemMetrics(SM_CXVSCROLL) / 2.0;
	if(!m_sessionDlg.m_propertiesDlg.SetWindowPos(NULL,dx,0,axPanelRect.right-2*dx,propertiesDlgRect.bottom-propertiesDlgRect.top,SWP_NOZORDER|SWP_NOACTIVATE))
		throw MCException_Win("RCViewerPropertiesDlg resizing failed");

	/// Cutting white colors with help of window regions
	FormShape(m_sessionDlg.m_propertiesDlg, LOWER_LEFT);

	/// Aligning scrollbars
	RECT hscrollRect(axPanelRect);
	hscrollRect.top = hscrollRect.bottom - GetSystemMetrics(SM_CYHSCROLL);
	hscrollRect.right -= m_vscrollBar.IsWindowVisible()?GetSystemMetrics(SM_CXVSCROLL):0;
	m_hscrollBar.SetWindowPos(HWND_TOP/*axPanel*/, &hscrollRect, 0);

	RECT vscrollRect(axPanelRect);
	vscrollRect.left = vscrollRect.right - GetSystemMetrics(SM_CXVSCROLL);
	vscrollRect.bottom -= m_hscrollBar.IsWindowVisible()?GetSystemMetrics(SM_CYHSCROLL):0;
	m_vscrollBar.SetWindowPos(HWND_TOP/*axPanel*/, &vscrollRect, 0);

CATCH_LOG()
	return 0;
}

void CRCViewerUIMediator::OnSessionStartBtnClick(int permission)
{
TRY_CATCH
	if(!m_viewer)
		MCException("CCoRCViewer owner of CRCViewerUIMediator object isn't set");
	if(m_status.m_on)
	{
		m_viewer->HandleUIMediatorCommand(UIE_SESSION_STOP, 0);
	}
	else
	{
		m_viewer->m_hideWallpaper = m_sessionDlg.m_hideWallpaper;
		m_viewer->HandleUIMediatorCommand(UIE_SESSION_START, permission);
	}
CATCH_LOG()
}

void CRCViewerUIMediator::SetExtendedUIStatus(const EDSStateEx& status, const tstring& message)
{
TRY_CATCH
	SetUIStatus(SDSViewerState(m_status.m_on, status), message);
CATCH_THROW()
}

void CRCViewerUIMediator::SetUIStatus(const SDSViewerState& status, const tstring& message)
{
TRY_CATCH
	//Log.Add(_MESSAGE_,_T("status=%d %s"),status,message.c_str());
	//m_statusBarDlg.m_statusMessage.SetWindowText(message.c_str());
	if(m_status != status)
	{
		m_status = status;
		if (m_status.m_on)
		{
			m_sessionDlg.SetHideWallpaperEnabled(false);
			m_sessionDlg.m_startBtn.SetWindowText(_T("Stop"));
			m_sessionDlg.m_startBtn.Hint=_T("Press to stop session");
			m_sessionDlg.m_startBtn.EnableWindow();
			m_statusBarDlg.m_statusIcon.m_currentImage= m_uiStatusIcons[EDSS_STATES_COUNT-1];
			m_statusBarDlg.m_statusMessage.FontColor1= m_statusBarDlg.m_statusIcon.m_currentImage->GetPixel(m_statusBarDlg.m_statusIcon.m_currentImage->GetWidth()/2,1);
			switch(m_status.m_extendedState)
			{
				case EDSS_PERMISSION_REQUEST_SENT:
					m_sessionDlg.m_propertiesDlg.m_displayCmb.EnableWindow();
					m_sessionDlg.m_propertiesDlg.m_cadBtn.EnableWindow();
					m_sessionDlg.m_propertiesDlg.m_colorsCmb.EnableWindow();
					m_sessionDlg.m_propertiesDlg.m_compressionCmb.EnableWindow();
					m_sessionDlg.m_permissionCmb.EnableWindow(FALSE);
					break;
				case EDSS_INIFIAL:
				case EDSS_PERMISSION_RECEIVED:
				case EDSS_PERMISSION_DENIED:
					m_sessionDlg.m_propertiesDlg.m_displayCmb.EnableWindow();
					m_sessionDlg.m_propertiesDlg.m_cadBtn.EnableWindow();
					m_sessionDlg.m_propertiesDlg.m_colorsCmb.EnableWindow();
					m_sessionDlg.m_propertiesDlg.m_compressionCmb.EnableWindow();
					m_sessionDlg.m_permissionCmb.EnableWindow();					
					break;
				default:
					Log.Add(_WARNING_,_T("Wrong status code, while session is on = %d. CRCViewerUIMediator::SetUIStatus"),status);
			}

		} else
		{
			m_sessionDlg.SetHideWallpaperEnabled(true);
			m_sessionDlg.m_startBtn.SetWindowText(_T("Send"));
			if (m_status.m_extendedState >= EDSS_PERMISSION_REQUEST_SENT && m_status.m_extendedState < EDSS_STATES_COUNT)
			{
				m_statusBarDlg.m_statusIcon.m_currentImage=m_uiStatusIcons[m_status.m_extendedState - 1];
				m_statusBarDlg.m_statusMessage.FontColor1= m_statusBarDlg.m_statusIcon.m_currentImage->GetPixel(m_statusBarDlg.m_statusIcon.m_currentImage->GetWidth()/2,1);
			}
			else
			{
				m_statusBarDlg.m_statusMessage.FontColor1=0;
				m_statusBarDlg.m_statusIcon.m_currentImage=NULL;//unknown status code
			}

			switch(m_status.m_extendedState)
			{
				case EDSS_PERMISSION_REQUEST_SENT:
				case EDSS_PERMISSION_RECEIVED:
				case EDSS_INSTALLATION_PROGRESS:
				case EDSS_CONNECTING:
				case EDSS_INSTALLATION_SUCCESSFUL:
					m_sessionDlg.m_startBtn.Hint=_T("");
					m_sessionDlg.m_startBtn.EnableWindow(FALSE);
					m_sessionDlg.m_propertiesDlg.m_displayCmb.EnableWindow(FALSE);
					m_sessionDlg.m_propertiesDlg.m_cadBtn.EnableWindow(FALSE);
					m_sessionDlg.m_propertiesDlg.m_colorsCmb.EnableWindow();
					m_sessionDlg.m_propertiesDlg.m_compressionCmb.EnableWindow();
					m_sessionDlg.m_permissionCmb.EnableWindow(FALSE);
					break;
				case EDSS_PERMISSION_DENIED:
				case EDSS_INSTALLATION_FAILED:
				case EDSS_SESSION_FAILED:
				case EDSS_INIFIAL:
					m_sessionDlg.m_startBtn.Hint=_T("Request Customer's permission");
					m_sessionDlg.m_startBtn.EnableWindow();
					m_sessionDlg.m_propertiesDlg.m_displayCmb.EnableWindow();
					m_sessionDlg.m_propertiesDlg.m_cadBtn.EnableWindow(FALSE);
					m_sessionDlg.m_propertiesDlg.m_colorsCmb.EnableWindow();
					m_sessionDlg.m_propertiesDlg.m_compressionCmb.EnableWindow();
					m_sessionDlg.m_permissionCmb.EnableWindow();
					break;
				default:
					Log.Add(_WARNING_,_T("Unknown status code = %d. CRCViewerUIMediator::SetUIStatus"),status);
			}
		}
		
		//m_statusBarDlg.m_statusIcon.RedrawWindow();
		m_statusBarDlg.RedrawWindow();
		
	}
	m_statusBarDlg.m_statusMessage.Text =	m_status.m_on 
											&& 
											message.find(_T("Desktop Sharing - On")) == tstring::npos?Format(_T("%s %s"),_T("Desktop Sharing - On."),message.c_str()):message;
	m_statusBarDlg.m_statusMessage.RedrawWindow();
CATCH_LOG()
}

HWND CRCViewerUIMediator::GetViewerHostWnd(void)
{
TRY_CATCH
	HWND hWnd;
	if(!(hWnd=m_parentWnd.GetDlgItem(IDC_RCVIEWERAXPANEL)))
		throw MCException_Win("Viewer ActiveX panel obtaining failed");
	return hWnd;
CATCH_THROW()
}

void CRCViewerUIMediator::OnPropertiesBtnClick()
{
TRY_CATCH

	if(m_sessionDlg.m_propertiesDlg.IsWindowVisible())
	{

		bool underVista = false;
		OSVERSIONINFOEX osInf;
		osInf.dwOSVersionInfoSize=sizeof(osInf);
		if(GetVersionEx((OSVERSIONINFO*)&osInf))
			underVista = osInf.dwMajorVersion > 5;

		if(!::AnimateWindow(m_sessionDlg.m_propertiesDlg, ANIMATE_TIME, AW_HIDE|(underVista?AW_BLEND:AW_SLIDE|AW_VER_NEGATIVE)))
			throw MCException_Win("Properties dialog hiding failed");
		m_sessionDlg.m_propertiesBtn.SetImages(IDR_DOWN_REGULAR_74_22, IDR_BTN_DISABLED_72_22, IDR_DOWN_PRESSED_74_22, IDR_DOWN_MOUSE_OVER_74_22);
	}
	else
	{
		if(!::AnimateWindow(m_sessionDlg.m_propertiesDlg, ANIMATE_TIME, AW_VER_POSITIVE|AW_SLIDE))
			throw MCException_Win("Properties dialog showing failed");
		m_sessionDlg.m_propertiesDlg.SetWindowPos(HWND_TOP,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);

		m_sessionDlg.m_propertiesBtn.SetImages(IDR_UP_REGULAR_74_22, IDR_BTN_DISABLED_72_22, IDR_UP_PRESSED_74_22, IDR_UP_MOUSE_OVER_74_22);
	}

CATCH_THROW()
}

void CRCViewerUIMediator::OnPropertiesComboboxSelChanged()
{
TRY_CATCH

	/// Handling apply btn
	m_sessionDlg.m_propertiesDlg.m_applyBtn.EnableWindow(TRUE);

CATCH_THROW()
}

void CRCViewerUIMediator::OnPropertiesApplyBtnClick(int displayIndex,int colorsIndex,int compressionIndex)
{
TRY_CATCH
	if(!m_viewer)
		MCException("CCoRCViewer owner of CRCViewerUIMediator object isn't set");
	m_viewer->SetDisplayMode(m_displayMode = displayIndex);
	bool colorsCompressionChanged = false;
	if(colorsIndex>0)//manual set
	{
		colorsCompressionChanged |= m_viewer->m_opts.m_colorsCount != colorsIndex-1;
		m_viewer->m_opts.m_colorsCount=colorsIndex-1;
		colorsCompressionChanged |= m_viewer->m_opts.m_autoColors;
		m_viewer->m_opts.m_autoColors=false;
	}
	else
	{
		colorsCompressionChanged |= !m_viewer->m_opts.m_autoColors;
		m_viewer->m_opts.m_autoColors=true;
	}
	if (compressionIndex>0)//manual set
	{
		int encoding = rfbEncodingZlibHex;
		switch(compressionIndex)
		{
			case 1:
				encoding = rfbEncodingTight;
				break;
			case 2:
				encoding = rfbEncodingTight;
				break;
			case 3:
				encoding = rfbEncodingZlibHex;
				break;
			default:
				encoding = rfbEncodingZlibHex;
				break;
		}
		colorsCompressionChanged |= m_viewer->m_opts.m_PreferredEncoding != encoding;
		m_viewer->m_opts.m_PreferredEncoding = encoding;
		colorsCompressionChanged |= m_viewer->m_opts.m_autoColors;
		if (m_viewer->m_opts.m_autoColors)
			m_viewer->m_opts.m_colorsCount = 0;
		m_viewer->m_opts.m_autoColors=false;
	}
	if (colorsCompressionChanged)
		m_viewer->ApplyOptions();

	/// Handling apply btn
	m_sessionDlg.m_propertiesDlg.m_applyBtn.EnableWindow(FALSE);

CATCH_THROW()
}

void CRCViewerUIMediator::OnViewerInit(tstring& peerId)
{
TRY_CATCH
	m_peerId=peerId;
	if(IsWindow(m_sessionDlg.m_customerLbl.m_hWnd))
		m_sessionDlg.m_customerLbl.Text=m_peerId;
CATCH_LOG()
}

void CRCViewerUIMediator::OnSessionPermissionCmbSelChange(int permission)
{
TRY_CATCH
	if(!m_viewer)
		MCException("CCoRCViewer owner of CRCViewerUIMediator object isn't set");
	if(m_status.m_on)
	{
		m_viewer->HandleUIMediatorCommand(UIE_PERMISSION_REQUEST,permission);
	}
CATCH_LOG()
}

HWND CRCViewerUIMediator::GetVertScrollBar()
{
TRY_CATCH
	if (!m_vscrollBar.IsWindow())
	{
		m_vscrollBar.m_hWnd = NULL;
		m_vscrollBar.Create(GetViewerHostWnd(), NULL, NULL, SBS_VERT | WS_CHILD);
	}
	return m_vscrollBar;
CATCH_LOG()
	return NULL;
}

HWND CRCViewerUIMediator::GetHorSrcrollBar()
{
TRY_CATCH
	if (!m_hscrollBar.IsWindow())
	{
		m_hscrollBar.m_hWnd = NULL;
		m_hscrollBar.Create(GetViewerHostWnd(), NULL, NULL, WS_CHILD);
	}
	return m_hscrollBar;
CATCH_LOG()
	return NULL;
}

void CRCViewerUIMediator::OnSessionStarted()
{
TRY_CATCH
	//+crutch
	SetUIStatus(SDSViewerState(true,EDSS_INIFIAL),_T("Desktop Sharing - On"));//UIS_SESSION_OFF
	//-crutch
	if (TRUE == IsWindowVisible(m_sessionDlg.m_propertiesDlg))
	{
		m_sessionDlg.m_propertiesDlg.RedrawWindow();
	}
	/// Setting view mode STL-741
	m_viewer->SetDisplayMode(m_displayMode);

CATCH_LOG()
}

void CRCViewerUIMediator::OnSessionStopped(ESessionStopReason stopReason)
{
TRY_CATCH
	switch(stopReason)
	{
		case REMOTE_STOP:
			SetUIStatus(SDSViewerState(false,EDSS_SESSION_FAILED), BRT_SERVICE_STOPPED_BY_CUSTOMER);
			break;
		default:
			SetUIStatus(SDSViewerState(false,EDSS_INIFIAL),_T("Desktop Sharing - Off"));
			break;
	}
	//-crutch
	if (m_hscrollBar.IsWindow())
		m_hscrollBar.ShowWindow(SW_HIDE);
	if (m_vscrollBar.IsWindow())
		m_vscrollBar.ShowWindow(SW_HIDE);

	m_displayMode = m_sessionDlg.m_propertiesDlg.m_displayCmb.GetCurSel();

CATCH_LOG()
}

void CRCViewerUIMediator::OnDisplayModeChanged(EDisplayMode mode)
{
TRY_CATCH
	m_sessionDlg.m_propertiesDlg.m_displayCmb.SetCurSel(mode);
CATCH_LOG()
}

void CRCViewerUIMediator::OnCADBtnClick()
{
TRY_CATCH
	if(!m_viewer)
		MCException("CCoRCViewer owner of CRCViewerUIMediator object isn't set");
	m_viewer->SendCtrlAltDel();
CATCH_THROW()
}