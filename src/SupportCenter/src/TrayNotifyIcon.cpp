// CTrayNotifyIcon.cpp
//
//===========================================================================
// SupportSpace ltd. @{SRCH}
//								CTrayNotifyIcon
//
//---------------------------------------------------------------------------
// DESCRIPTION: @{HDES}
// -----------
// CTrayNotifyIcon : implementation for a MFC class to encapsulate Shell_NotifyIcon
//---------------------------------------------------------------------------
// CHANGES LOG: @{HREV}
// -----------
// Revision: 01.00
// By      : 
// Date    : 10/29/2006 05:21:10 PM
// Comments: First Issue
//===========================================================================

/////////////////////////////////  Includes  //////////////////////////////////

#include "stdafx.h"
#include "TrayNotifyIcon.h"
#ifndef _INC_SHELLAPI
#pragma message("To avoid this message, please put ShellApi.h in your PCH (normally stdafx.h)")
#include <ShellApi.h>
#endif


/////////////////////////////////  Macros /////////////////////////////////////

#ifdef _AFX
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#endif

//Defines our own versions of various constants we use from ShellApi.h. This allows us to operate in a mode
//where we do not depend on value set for _WIN32_IE
#ifndef NIIF_USER
#define NIIF_USER 0x00000004
#endif

#ifndef NIF_STATE
#define NIF_STATE 0x00000008
#endif

#ifndef NIF_INFO
#define NIF_INFO 0x00000010
#endif

#ifndef NIS_HIDDEN
#define NIS_HIDDEN              0x00000001
#endif

#ifndef NOTIFYICON_VERSION
#define NOTIFYICON_VERSION 3
#endif

#ifndef NIM_SETVERSION
#define NIM_SETVERSION 0x00000004
#endif

#ifndef NIIF_NONE
#define NIIF_NONE 0x00000000
#endif

#ifndef NIIF_INFO
#define NIIF_INFO 0x00000001
#endif

#ifndef NIIF_WARNING
#define NIIF_WARNING 0x00000002
#endif

#ifndef NIIF_ERROR
#define NIIF_ERROR 0x00000003
#endif

#ifndef NIIF_USER
#define NIIF_USER 0x00000004
#endif

#ifndef NIIF_NOSOUND
#define NIIF_NOSOUND 0x00000010
#endif

#ifndef NIM_SETFOCUS
#define NIM_SETFOCUS 0x00000003
#endif

DWORD CTrayNotifyIcon::sm_dwShellVersion = 0;


///////////////////////////////// Implementation //////////////////////////////

const UINT wm_TaskbarCreated = RegisterWindowMessage(_T("TaskbarCreated"));

CTrayIconHooker::CTrayIconHooker() : m_pTrayIcon(NULL),
                                     m_phIcons(NULL),
                                     m_nNumIcons(0),
                                     m_nTimerID(0),
                                     m_nCurrentIconIndex(0)
{
}

CTrayIconHooker::~CTrayIconHooker()
{
  StopUsingAnimation();
}

#ifdef _AFX
BOOL CTrayIconHooker::Init(CTrayNotifyIcon* pTrayIcon, CWnd* pNotifyWnd)
#else
BOOL CTrayIconHooker::Init(CTrayNotifyIcon* pTrayIcon, CWindow* pNotifyWnd)
#endif
{
  //Validate our parameters
  ATLASSERT(pTrayIcon); //must have a valid tray notify instance
#ifdef _AFX
  ATLASSERT(pNotifyWnd && ::IsWindow(pNotifyWnd->GetSafeHwnd()));
#else
  ATLASSERT(pNotifyWnd && pNotifyWnd->IsWindow());
#endif

  //Hive away the input parameter
  m_pTrayIcon = pTrayIcon;

  //Hook the top level frame of the notify window in preference 
  //to the notify window itself. This will ensure that we get
  //the taskbar created message
#ifdef _AFX  
  CWnd* pTopLevelWnd = pNotifyWnd->GetTopLevelFrame();
  if (pTopLevelWnd)
    return SubclassWindow(pTopLevelWnd->operator HWND()); 
  else
    return SubclassWindow(pNotifyWnd->GetSafeHwnd());
#else
  CWindow TopLevelWnd = pNotifyWnd->GetTopLevelWindow();
  if (TopLevelWnd.IsWindow())
    return SubclassWindow(TopLevelWnd.operator HWND()); 
  else
    return SubclassWindow(pNotifyWnd->m_hWnd);
#endif
}

void CTrayIconHooker::StartUsingAnimation(HICON* phIcons, int nNumIcons, DWORD dwDelay)
{
  //Validate our parameters
  ATLASSERT(nNumIcons >= 2); //must be using at least 2 icons if you are using animation
  ATLASSERT(phIcons);        //array of icon handles must be valid
  ATLASSERT(dwDelay);        //must be non zero timer interval
  ATLASSERT(m_pTrayIcon);

  //Stop the animation if already started  
  StopUsingAnimation();

  //Hive away all the values locally
  ATLASSERT(m_phIcons == NULL);
  m_phIcons = new HICON[nNumIcons];
  for (int i=0; i<nNumIcons; i++)
    m_phIcons[i] = phIcons[i];
  m_nNumIcons = nNumIcons;

  //Start up the timer 
  m_nTimerID = SetTimer(m_pTrayIcon->m_NotifyIconData.uID, dwDelay);
}

void CTrayIconHooker::StopUsingAnimation()
{
  //Kill the timer
  if (m_nTimerID)
  {
    if (::IsWindow(m_hWnd))
      KillTimer(m_nTimerID);
    m_nTimerID = 0;
  }
 
  //Free up the memory
  if (m_phIcons)
  {
    delete [] m_phIcons;
    m_phIcons = NULL;
  }

  //Reset the other animation related variables
  m_nCurrentIconIndex = 0;
  m_nNumIcons = 0;
}

BOOL CTrayIconHooker::UsingAnimatedIcon() const
{
  return (m_nNumIcons != 0);
}

HICON CTrayIconHooker::GetCurrentIcon() const 
{ 
  ATLASSERT(UsingAnimatedIcon());
  ATLASSERT(m_phIcons);
  return m_phIcons[m_nCurrentIconIndex]; 
}

BOOL CTrayIconHooker::ProcessWindowMessage(HWND /*hWnd*/, UINT nMsg, WPARAM wParam, LPARAM /*lParam*/, LRESULT& lResult, DWORD /*dwMsgMapID*/)
{
  //Validate our parameters
  ATLASSERT(m_pTrayIcon);

  lResult = 0;
  BOOL bHandled = FALSE;

  if (nMsg == wm_TaskbarCreated)
  {
    //Refresh the tray icon if necessary
    m_pTrayIcon->Delete();
    m_pTrayIcon->Create();
  }
  else if ((nMsg == WM_TIMER) && (wParam == m_pTrayIcon->m_NotifyIconData.uID))
  {
    OnTimer(m_pTrayIcon->m_NotifyIconData.uID); 
    bHandled = TRUE; //Do not allow this message to go any further because we have fully handled the message
  }
  else if (nMsg == WM_DESTROY)
    m_pTrayIcon->Delete();

  return bHandled;
}


void CTrayIconHooker::OnTimer(UINT_PTR nIDEvent)
{
  if(nIDEvent != m_nTimerID) 
  {
	  CString sMsg;
	  sMsg.Format(_T("nIDEvent:%d != m_nTimerID:%d \r\n"),nIDEvent, m_nTimerID);
	  OutputDebugString(sMsg);
	  return;
  }
  //ATLASSERT(nIDEvent == m_nTimerID);

  //increment the icon index
  ++m_nCurrentIconIndex;
  m_nCurrentIconIndex = m_nCurrentIconIndex % m_nNumIcons;

  //update the tray icon
  m_pTrayIcon->m_NotifyIconData.uFlags = NIF_ICON;
  m_pTrayIcon->m_NotifyIconData.hIcon = m_phIcons[m_nCurrentIconIndex];
  Shell_NotifyIcon(NIM_MODIFY, reinterpret_cast<PNOTIFYICONDATA>(&m_pTrayIcon->m_NotifyIconData));
}


CTrayNotifyIcon::CTrayNotifyIcon() : m_bCreated(FALSE),
                                     m_bHidden(FALSE),
                                     m_pNotificationWnd(NULL),
                                     m_bDefaultMenuItemByPos(TRUE),
                                     m_nDefaultMenuItem(0),
                                     m_hDynamicIcon(NULL)
{
  memset(&m_NotifyIconData, 0, sizeof(m_NotifyIconData));
  m_NotifyIconData.cbSize = GetNOTIFYICONDATASizeForOS();
}

CTrayNotifyIcon::~CTrayNotifyIcon()
{
  //Delete the tray icon
  Delete();
  
  //Free up any dynamic icon we may have
  if (m_hDynamicIcon)
  {
    DestroyIcon(m_hDynamicIcon);
    m_hDynamicIcon = NULL;
  }
}

BOOL CTrayNotifyIcon::Delete()
{
  //What will be the return value from this function (assume the best)
  BOOL bSuccess = TRUE;

  if (m_bCreated)
  {
    m_NotifyIconData.uFlags = 0;
    bSuccess = Shell_NotifyIcon(NIM_DELETE, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
    m_bCreated = FALSE;
  }
  return bSuccess;
}

BOOL CTrayNotifyIcon::Create()
{
  m_NotifyIconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
  BOOL bSuccess = Shell_NotifyIcon(NIM_ADD, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
  if (bSuccess)
    m_bCreated = TRUE;
  return bSuccess;
}

BOOL CTrayNotifyIcon::Hide()
{
  //Validate our parameters
  ATLASSERT(GetShellVersion() >= 5); //Only supported on Shell v5 or later
  ATLASSERT(!m_bHidden); //Only makes sense to hide the icon if it is not already hidden

  m_NotifyIconData.uFlags = NIF_STATE;
  m_NotifyIconData.dwState = NIS_HIDDEN;
  m_NotifyIconData.dwStateMask = NIS_HIDDEN; 
  BOOL bSuccess = Shell_NotifyIcon(NIM_MODIFY, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
  if (bSuccess)
    m_bHidden = TRUE;
  return bSuccess;
}

BOOL CTrayNotifyIcon::Show()
{
  //Validate our parameters
  ATLASSERT(GetShellVersion() >= 5); //Only supported on Shell v5 or later
  ATLASSERT(m_bHidden); //Only makes sense to show the icon if it has been previously hidden

  ATLASSERT(m_bCreated);
  m_NotifyIconData.uFlags = NIF_STATE;
  m_NotifyIconData.dwState = 0;
  m_NotifyIconData.dwStateMask = NIS_HIDDEN;
  BOOL bSuccess = Shell_NotifyIcon(NIM_MODIFY, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
  if (bSuccess)
    m_bHidden = FALSE;
  return bSuccess;
}

void CTrayNotifyIcon::SetMenu(HMENU hMenu)
{
  //Validate our parameters
  ATLASSERT(hMenu);

  m_Menu.DestroyMenu();
  m_Menu.Attach(hMenu);

#ifdef _AFX
  CMenu* pSubMenu = m_Menu.GetSubMenu(0);
  ATLASSERT(pSubMenu); //Your menu resource has been designed incorrectly
    
  //Make the specified menu item the default (bold font)
  pSubMenu->SetDefaultItem(m_nDefaultMenuItem, m_bDefaultMenuItemByPos);
#else
  CMenuHandle subMenu = m_Menu.GetSubMenu(0);
  ATLASSERT(subMenu.IsMenu()); //Your menu resource has been designed incorrectly

  //Make the specified menu item the default (bold font)
  subMenu.SetMenuDefaultItem(m_nDefaultMenuItem, m_bDefaultMenuItemByPos);
#endif
}

CMenu& CTrayNotifyIcon::GetMenu()
{
  return m_Menu;
}

void CTrayNotifyIcon::SetDefaultMenuItem(UINT uItem, BOOL fByPos) 
{ 
  m_nDefaultMenuItem = uItem; 
  m_bDefaultMenuItemByPos = fByPos; 

  //Also update in the live menu if it is present
  if (m_Menu.operator HMENU())
  {
  #ifdef _AFX
    CMenu* pSubMenu = m_Menu.GetSubMenu(0);
    ATLASSERT(pSubMenu); //Your menu resource has been designed incorrectly

    pSubMenu->SetDefaultItem(m_nDefaultMenuItem, m_bDefaultMenuItemByPos);
  #else
    CMenuHandle subMenu = m_Menu.GetSubMenu(0);
    ATLASSERT(subMenu.IsMenu()); //Your menu resource has been designed incorrectly
    
    subMenu.SetMenuDefaultItem(m_nDefaultMenuItem, m_bDefaultMenuItemByPos);
  #endif
  }
}

#ifdef _AFX
BOOL CTrayNotifyIcon::Create(CWnd* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, HICON hIcon, UINT nNotifyMessage, UINT uMenuID)
#else
BOOL CTrayNotifyIcon::Create(CWindow* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, HICON hIcon, UINT nNotifyMessage, UINT uMenuID)
#endif
{
  //Validate our parameters
  ATLASSERT(pNotifyWnd && ::IsWindow(pNotifyWnd->operator HWND()));
#ifdef _DEBUG  
  if (GetShellVersion() >= 5) //If on Shell v5 or higher, then use the larger size tooltip
  {
    NOTIFYICONDATA_2 dummy;
    ATLASSERT(_tcslen(pszTooltipText) < sizeof(dummy.szTip)/sizeof(TCHAR));
    DBG_UNREFERENCED_LOCAL_VARIABLE(dummy);
  }
  else
  {
    NOTIFYICONDATA_1 dummy;
    ATLASSERT(_tcslen(pszTooltipText) < sizeof(dummy.szTip)/sizeof(TCHAR));
    DBG_UNREFERENCED_LOCAL_VARIABLE(dummy);
  }
#endif
  ATLASSERT(hIcon); 
  ATLASSERT(nNotifyMessage >= WM_USER); //Make sure we avoid conflict with other messages

  //Load up the menu resource which is to be used as the context menu
  if (!m_Menu.LoadMenu(uMenuID == 0 ? uID : uMenuID))
  {
    ATLASSERT(FALSE);
    return FALSE;
  }
#ifdef _AFX
  CMenu* pSubMenu = m_Menu.GetSubMenu(0);
  if (!pSubMenu) 
  {
    ATLASSERT(FALSE); //Your menu resource has been designed incorrectly
    return FALSE;
  }
  //Make the specified menu item the default (bold font)
  pSubMenu->SetDefaultItem(m_nDefaultMenuItem, m_bDefaultMenuItemByPos);
#else
  CMenuHandle subMenu = m_Menu.GetSubMenu(0);
  if (!subMenu.IsMenu())
  {
    ATLASSERT(FALSE); //Your menu resource has been designed incorrectly
    return FALSE;
  }
  subMenu.SetMenuDefaultItem(m_nDefaultMenuItem, m_bDefaultMenuItemByPos);
#endif

  //Install the hook
  if (!m_HookWnd.Init(this, pNotifyWnd))
    return FALSE;

  //Call the Shell_NotifyIcon function
  m_pNotificationWnd = pNotifyWnd;
  m_NotifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
  m_NotifyIconData.hWnd = pNotifyWnd->operator HWND();
  m_NotifyIconData.uID = uID;
  m_NotifyIconData.uCallbackMessage = nNotifyMessage;
  m_NotifyIconData.hIcon = hIcon;
#if (_MSC_VER >= 1400)
  _tcscpy_s(m_NotifyIconData.szTip, sizeof(m_NotifyIconData.szTip)/sizeof(TCHAR), pszTooltipText);
#else  
  _tcscpy(m_NotifyIconData.szTip, pszTooltipText);
#endif  
  m_bCreated = Shell_NotifyIcon(NIM_ADD, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));

  //Turn on Shell v5 style behaviour if supported
  if (GetShellVersion() >= 5)
    SetVersion(NOTIFYICON_VERSION);

  return m_bCreated;
}

BOOL CTrayNotifyIcon::SetVersion(UINT uVersion)
{
  //Validate our parameters
  ATLASSERT(GetShellVersion() >= 5); //Only supported on Shell v5 or later

  //Call the Shell_NotifyIcon function
  m_NotifyIconData.uVersion = uVersion;
  return Shell_NotifyIcon(NIM_SETVERSION, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
}

HICON CTrayNotifyIcon::BitmapToIcon(CBitmap* pBitmap)
{
  //Validate our parameters
  ATLASSERT(pBitmap);

  //Get the width and height of a small icon
  int w = GetSystemMetrics(SM_CXSMICON);
  int h = GetSystemMetrics(SM_CYSMICON);

  //Create a 0 mask
  int nMaskSize = h*(w/8);
  unsigned char* pMask = new unsigned char[nMaskSize];
  memset(pMask, 0, nMaskSize);

  //Create a mask bitmap
  CBitmap maskBitmap;
#ifdef _AFX
  BOOL bSuccess = maskBitmap.CreateBitmap(w, h, 1, 1, pMask);
#else
  maskBitmap.CreateBitmap(w, h, 1, 1, pMask);
  BOOL bSuccess = !maskBitmap.IsNull();
#endif

  //Free up the heap memory now that we have created the mask bitmap
  delete [] pMask;

  //Handle the error
  if (!bSuccess)
    return NULL;

  //Create an ICON base on the bitmap just created
  ICONINFO iconInfo;
  iconInfo.fIcon = TRUE;
  iconInfo.xHotspot = 0;
  iconInfo.yHotspot = 0;
  iconInfo.hbmMask = maskBitmap;
  iconInfo.hbmColor = *pBitmap; 
  return CreateIconIndirect(&iconInfo); 
}

#ifdef _AFX
BOOL CTrayNotifyIcon::Create(CWnd* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, CBitmap* pBitmap, UINT nNotifyMessage, UINT uMenuID)
#else
BOOL CTrayNotifyIcon::Create(CWindow* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, CBitmap* pBitmap, UINT nNotifyMessage, UINT uMenuID)
#endif
{
  //Convert the bitmap to an ICON
  if (m_hDynamicIcon)
    DestroyIcon(m_hDynamicIcon);
  m_hDynamicIcon = BitmapToIcon(pBitmap);

  //Pass the buck to the other function to do the work
  return Create(pNotifyWnd, uID, pszTooltipText, m_hDynamicIcon, nNotifyMessage, uMenuID);
}

#ifdef _AFX
BOOL CTrayNotifyIcon::Create(CWnd* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, HICON* phIcons, int nNumIcons, DWORD dwDelay, UINT nNotifyMessage, UINT uMenuID)
#else
BOOL CTrayNotifyIcon::Create(CWindow* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, HICON* phIcons, int nNumIcons, DWORD dwDelay, UINT nNotifyMessage, UINT uMenuID)
#endif
{
  //Validate our parameters
  ATLASSERT(phIcons);
  ATLASSERT(nNumIcons >= 2); //must be using at least 2 icons if you are using animation
  ATLASSERT(dwDelay);

  //let the normal Create function do its stuff
  BOOL bSuccess = Create(pNotifyWnd, uID, pszTooltipText, phIcons[0], nNotifyMessage, uMenuID);

  //tell the hook class to do the animation
  m_HookWnd.StartUsingAnimation(phIcons, nNumIcons, dwDelay);

  return bSuccess;
}

#ifdef _AFX
BOOL CTrayNotifyIcon::Create(CWnd* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, LPCTSTR pszBalloonText, LPCTSTR pszBalloonCaption, UINT nTimeout, BalloonStyle style, HICON hIcon, UINT nNotifyMessage, UINT uMenuID, BOOL bNoSound)
#else
BOOL CTrayNotifyIcon::Create(CWindow* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, LPCTSTR pszBalloonText, LPCTSTR pszBalloonCaption, UINT nTimeout, BalloonStyle style, HICON hIcon, UINT nNotifyMessage, UINT uMenuID, BOOL bNoSound)
#endif
{
  //Validate our parameters
  ATLASSERT(pNotifyWnd && ::IsWindow(pNotifyWnd->operator HWND()));
  ATLASSERT(GetShellVersion() >= 5); //Only supported on Shell v5 or later
#ifdef _DEBUG
  NOTIFYICONDATA_2 dummy;
  DBG_UNREFERENCED_LOCAL_VARIABLE(dummy);
  ATLASSERT(_tcslen(pszTooltipText) < sizeof(dummy.szTip)/sizeof(TCHAR));
  ATLASSERT(_tcslen(pszBalloonText) < sizeof(dummy.szInfo)/sizeof(TCHAR));
  ATLASSERT(_tcslen(pszBalloonCaption) < sizeof(dummy.szInfoTitle)/sizeof(TCHAR));
  ATLASSERT(hIcon); 
  ATLASSERT(nNotifyMessage >= WM_USER); //Make sure we avoid conflict with other messages
#endif

  //Load up the menu resource which is to be used as the context menu
  if (!m_Menu.LoadMenu(uMenuID == 0 ? uID : uMenuID))
  {
    ATLASSERT(FALSE);
    return FALSE;
  }
#ifdef _AFX
  CMenu* pSubMenu = m_Menu.GetSubMenu(0);
  if (!pSubMenu) 
  {
    ATLASSERT(FALSE); //Your menu resource has been designed incorrectly
    return FALSE;
  }
  //Make the specified menu item the default (bold font)
  pSubMenu->SetDefaultItem(m_nDefaultMenuItem, m_bDefaultMenuItemByPos);
#else
  CMenuHandle subMenu = m_Menu.GetSubMenu(0);
  if (!subMenu.IsMenu()) 
  {
    ATLASSERT(FALSE); //Your menu resource has been designed incorrectly
    return FALSE;
  }
  //Make the specified menu item the default (bold font)
  subMenu.SetMenuDefaultItem(m_nDefaultMenuItem, m_bDefaultMenuItemByPos);
#endif  

  //Install the hook
  if (!m_HookWnd.Init(this, pNotifyWnd))
    return FALSE;

  //Call the Shell_NotifyIcon function
  m_pNotificationWnd = pNotifyWnd;
  m_NotifyIconData.hWnd = pNotifyWnd->operator HWND();
  m_NotifyIconData.uID = uID;
  m_NotifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
  m_NotifyIconData.uCallbackMessage = nNotifyMessage;
  m_NotifyIconData.hIcon = hIcon;
#if (_MSC_VER >= 1400)
  _tcscpy_s(m_NotifyIconData.szTip, sizeof(m_NotifyIconData.szTip)/sizeof(TCHAR), pszTooltipText);
  _tcscpy_s(m_NotifyIconData.szInfo, sizeof(m_NotifyIconData.szInfo)/sizeof(TCHAR), pszBalloonText);
  _tcscpy_s(m_NotifyIconData.szInfoTitle, sizeof(m_NotifyIconData.szInfoTitle)/sizeof(TCHAR), pszBalloonCaption);
#else
  _tcscpy(m_NotifyIconData.szTip, pszTooltipText);
  _tcscpy(m_NotifyIconData.szInfo, pszBalloonText);
  _tcscpy(m_NotifyIconData.szInfoTitle, pszBalloonCaption);
#endif  
  m_NotifyIconData.uTimeout = nTimeout;
  switch (style)
  {
    case Warning:
    {
      m_NotifyIconData.dwInfoFlags = NIIF_WARNING;
      break;
    }
    case Error:
    {
      m_NotifyIconData.dwInfoFlags = NIIF_ERROR;
      break;
    }
    case Info:
    {
      m_NotifyIconData.dwInfoFlags = NIIF_INFO;
      break;
    }
    case None:
    {
      m_NotifyIconData.dwInfoFlags = NIIF_NONE;
      break;
    }
    case User:
    {
      ATLASSERT(hIcon != NULL); //You forget to provide a user icon
      m_NotifyIconData.dwInfoFlags = NIIF_USER;
      break;
    }
    default:
    {
      ATLASSERT(FALSE);
      break;
    }
  }
  if (bNoSound)
    m_NotifyIconData.dwInfoFlags |= NIIF_NOSOUND;

  m_bCreated = Shell_NotifyIcon(NIM_ADD, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));

  //Turn on Shell v5 tray icon behaviour
  SetVersion(NOTIFYICON_VERSION);

  return m_bCreated;
}

#ifdef _AFX
BOOL CTrayNotifyIcon::Create(CWnd* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, LPCTSTR pszBalloonText, LPCTSTR pszBalloonCaption, UINT nTimeout, BalloonStyle style, CBitmap* pBitmap, UINT nNotifyMessage, UINT uMenuID, BOOL bNoSound)
#else
BOOL CTrayNotifyIcon::Create(CWindow* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, LPCTSTR pszBalloonText, LPCTSTR pszBalloonCaption, UINT nTimeout, BalloonStyle style, CBitmap* pBitmap, UINT nNotifyMessage, UINT uMenuID, BOOL bNoSound)
#endif
{
  //Convert the bitmap to an ICON
  if (m_hDynamicIcon)
    DestroyIcon(m_hDynamicIcon);
  m_hDynamicIcon = BitmapToIcon(pBitmap);

  //Pass the buck to the other function to do the work
  return Create(pNotifyWnd, uID, pszTooltipText, pszBalloonText, pszBalloonCaption, nTimeout, style, m_hDynamicIcon, nNotifyMessage, uMenuID, bNoSound);
}

#ifdef _AFX
BOOL CTrayNotifyIcon::Create(CWnd* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, LPCTSTR pszBalloonText, LPCTSTR pszBalloonCaption, UINT nTimeout, BalloonStyle style, HICON* phIcons, int nNumIcons, DWORD dwDelay, UINT nNotifyMessage, UINT uMenuID, BOOL bNoSound)
#else
BOOL CTrayNotifyIcon::Create(CWindow* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, LPCTSTR pszBalloonText, LPCTSTR pszBalloonCaption, UINT nTimeout, BalloonStyle style, HICON* phIcons, int nNumIcons, DWORD dwDelay, UINT nNotifyMessage, UINT uMenuID, BOOL bNoSound)
#endif
{
  //Validate our parameters
  ATLASSERT(phIcons);
  ATLASSERT(nNumIcons >= 2); //must be using at least 2 icons if you are using animation
  ATLASSERT(dwDelay);

  //let the normal Create function do its stuff
  BOOL bSuccess = Create(pNotifyWnd, uID, pszTooltipText, pszBalloonText, pszBalloonCaption, nTimeout, style, phIcons[0], nNotifyMessage, uMenuID, bNoSound);

  //tell the hook class to do the animation
  m_HookWnd.StartUsingAnimation(phIcons, nNumIcons, dwDelay);

  return bSuccess;
}

BOOL CTrayNotifyIcon::SetBalloonDetails(LPCTSTR pszBalloonText, LPCTSTR pszBalloonCaption, BalloonStyle style, UINT nTimeout, HICON hUserIcon, BOOL bNoSound)
{
  if (!m_bCreated)
    return FALSE;

  //Validate our parameters
  ATLASSERT(GetShellVersion() >= 5); //Only supported on Shell v5 or later
#ifdef _DEBUG
  NOTIFYICONDATA_2 dummy;
  DBG_UNREFERENCED_LOCAL_VARIABLE(dummy);
  ATLASSERT(_tcslen(pszBalloonText) < sizeof(dummy.szInfo)/sizeof(TCHAR));
  ATLASSERT(_tcslen(pszBalloonCaption) < sizeof(dummy.szInfoTitle)/sizeof(TCHAR));
#endif

  //Call the Shell_NotifyIcon function
  m_NotifyIconData.uFlags = NIF_INFO;
#if (_MSC_VER >= 1400)
  _tcscpy_s(m_NotifyIconData.szInfo, sizeof(m_NotifyIconData.szInfo)/sizeof(TCHAR), pszBalloonText);
  _tcscpy_s(m_NotifyIconData.szInfoTitle, sizeof(m_NotifyIconData.szInfoTitle)/sizeof(TCHAR), pszBalloonCaption);
#else  
  _tcscpy(m_NotifyIconData.szInfo, pszBalloonText);
  _tcscpy(m_NotifyIconData.szInfoTitle, pszBalloonCaption);
#endif  
  m_NotifyIconData.uTimeout = nTimeout;
  switch (style)
  {
    case Warning:
    {
      m_NotifyIconData.dwInfoFlags = NIIF_WARNING;
      break;
    }
    case Error:
    {
      m_NotifyIconData.dwInfoFlags = NIIF_ERROR;
      break;
    }
    case Info:
    {
      m_NotifyIconData.dwInfoFlags = NIIF_INFO;
      break;
    }
    case None:
    {
      m_NotifyIconData.dwInfoFlags = NIIF_NONE;
      break;
    }
    case User:
    {
      ATLASSERT(hUserIcon != NULL); //You forget to provide a user icon
      m_NotifyIconData.dwInfoFlags = NIIF_USER;
      m_NotifyIconData.hIcon = hUserIcon;
      break;
    }
    default:
    {
      ATLASSERT(FALSE);
      break;
    }
  }
  if (bNoSound)
    m_NotifyIconData.dwInfoFlags |= NIIF_NOSOUND;

  return Shell_NotifyIcon(NIM_MODIFY, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
}

CTrayNotifyIconString CTrayNotifyIcon::GetBalloonText() const
{
  //Validate our parameters
  ATLASSERT(GetShellVersion() >= 5); //Only supported on Shell v5 or later

  CTrayNotifyIconString sText;
  if (m_bCreated)
    sText = m_NotifyIconData.szInfo;

  return sText;
}

CTrayNotifyIconString CTrayNotifyIcon::GetBalloonCaption() const
{
  //Validate our parameters
  ATLASSERT(GetShellVersion() >= 5); //Only supported on Shell v5 or later

  CTrayNotifyIconString sText;
  if (m_bCreated)
    sText = m_NotifyIconData.szInfoTitle;

  return sText;
}

UINT CTrayNotifyIcon::GetBalloonTimeout() const
{
  //Validate our parameters
  ATLASSERT(GetShellVersion() >= 5); //Only supported on Shell v5 or later

  UINT nTimeout = 0;
  if (m_bCreated)
    nTimeout = m_NotifyIconData.uTimeout;

  return nTimeout;
}

BOOL CTrayNotifyIcon::SetTooltipText(LPCTSTR pszTooltipText)
{
  if (!m_bCreated)
    return FALSE;

  if (GetShellVersion() >= 5) //Allow the larger size tooltip text if on Shell v5 or later
  {
  #ifdef _DEBUG
    NOTIFYICONDATA_2 dummy;
    DBG_UNREFERENCED_LOCAL_VARIABLE(dummy);
    ATLASSERT(_tcslen(pszTooltipText) < sizeof(dummy.szTip)/sizeof(TCHAR));
  #endif
  }
  else 
  {
  #ifdef _DEBUG
    NOTIFYICONDATA_1 dummy;
    ATLASSERT(_tcslen(pszTooltipText) < sizeof(dummy.szTip)/sizeof(TCHAR));
    DBG_UNREFERENCED_LOCAL_VARIABLE(dummy);
  #endif
  }

  //Call the Shell_NotifyIcon function
  m_NotifyIconData.uFlags = NIF_TIP;
#if (_MSC_VER >= 1400)  
  _tcscpy_s(m_NotifyIconData.szTip, sizeof(m_NotifyIconData.szTip)/sizeof(TCHAR), pszTooltipText);
#else
  _tcscpy(m_NotifyIconData.szTip, pszTooltipText);
#endif  
  return Shell_NotifyIcon(NIM_MODIFY, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
}

BOOL CTrayNotifyIcon::SetTooltipText(UINT nID)
{
  CTrayNotifyIconString sToolTipText;
  sToolTipText.LoadString(nID);

  //Let the other version of the function handle the rest
  return SetTooltipText(sToolTipText);
}

BOOL CTrayNotifyIcon::SetIcon(CBitmap* pBitmap)
{
  //Convert the bitmap to an ICON
  if (m_hDynamicIcon)
    DestroyIcon(m_hDynamicIcon);
  m_hDynamicIcon = BitmapToIcon(pBitmap);

  //Pass the buck to the other function to do the work
  return SetIcon(m_hDynamicIcon);
}

BOOL CTrayNotifyIcon::SetIcon(HICON hIcon)
{
  //Validate our parameters
  ATLASSERT(hIcon);

  if (!m_bCreated)
    return FALSE;

  //Since we are going to use one icon, stop any animation
  m_HookWnd.StopUsingAnimation();

  //Call the Shell_NotifyIcon function
  m_NotifyIconData.uFlags = NIF_ICON;
  m_NotifyIconData.hIcon = hIcon;
  return Shell_NotifyIcon(NIM_MODIFY, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
}

BOOL CTrayNotifyIcon::SetIcon(LPCTSTR lpIconName)
{
  return SetIcon(LoadIconResource(lpIconName));
}

BOOL CTrayNotifyIcon::SetIcon(UINT nIDResource)
{
  return SetIcon(LoadIconResource(nIDResource));
}

BOOL CTrayNotifyIcon::SetStandardIcon(LPCTSTR lpIconName)
{
  return SetIcon(::LoadIcon(NULL, lpIconName));
}

BOOL CTrayNotifyIcon::SetStandardIcon(UINT nIDResource)
{
  return SetIcon(::LoadIcon(NULL, MAKEINTRESOURCE(nIDResource)));
}

BOOL CTrayNotifyIcon::SetIcon(HICON* phIcons, int nNumIcons, DWORD dwDelay)
{
  //Validate our parameters
  ATLASSERT(nNumIcons >= 2); //must be using at least 2 icons if you are using animation
  ATLASSERT(phIcons);
  ATLASSERT(dwDelay);

  if (!SetIcon(phIcons[0]))
    return FALSE;

  //Install the hook
  m_HookWnd.StartUsingAnimation(phIcons, nNumIcons, dwDelay);

  return TRUE;
}

HICON CTrayNotifyIcon::LoadIconResource(LPCTSTR lpIconName)
{
  //First try to load a small icon using LoadImage, if this fails, they fall back on the using LoadIcon which will just load the standard size
#ifdef _AFX
  HICON hIcon = static_cast<HICON>(::LoadImage(AfxGetResourceHandle(), lpIconName, IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_SHARED));
#else
  HICON hIcon = static_cast<HICON>(::LoadImage(ATL::_AtlBaseModule.GetResourceInstance(), lpIconName, IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_SHARED));
#endif
  if (hIcon == NULL)
  {
  #ifdef _AFX
    hIcon = AfxGetApp()->LoadIcon(lpIconName);
  #else
    hIcon = ::LoadIcon(ATL::_AtlBaseModule.GetResourceInstance(), lpIconName);
  #endif
  }

  //Return the icon handle
  return hIcon;
}

HICON CTrayNotifyIcon::LoadIconResource(UINT nIDResource)
{
  return LoadIconResource(MAKEINTRESOURCE(nIDResource));
}

#ifdef _AFX
BOOL CTrayNotifyIcon::SetNotificationWnd(CWnd* pNotifyWnd)
#else
BOOL CTrayNotifyIcon::SetNotificationWnd(CWindow* pNotifyWnd)
#endif
{
  //Validate our parameters
  ATLASSERT(pNotifyWnd && ::IsWindow(pNotifyWnd->operator HWND()));

  if (!m_bCreated)
    return FALSE;

  //Call the Shell_NotifyIcon function
  m_pNotificationWnd = pNotifyWnd;
  m_NotifyIconData.hWnd = pNotifyWnd->operator HWND();
  m_NotifyIconData.uFlags = 0;
  return Shell_NotifyIcon(NIM_MODIFY, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
}

CTrayNotifyIconString CTrayNotifyIcon::GetTooltipText() const
{
  CTrayNotifyIconString sText;
  if (m_bCreated)
    sText = m_NotifyIconData.szTip;

  return sText;
}

HICON CTrayNotifyIcon::GetIcon() const
{
  HICON hIcon = NULL;
  if (m_bCreated)
  {
    if (UsingAnimatedIcon())
      hIcon = m_HookWnd.GetCurrentIcon();
    else
      hIcon = m_NotifyIconData.hIcon;
  }

  return hIcon;
}

BOOL CTrayNotifyIcon::UsingAnimatedIcon() const
{
  return m_HookWnd.UsingAnimatedIcon();
}

#ifdef _AFX
CWnd* CTrayNotifyIcon::GetNotificationWnd() const
#else
CWindow* CTrayNotifyIcon::GetNotificationWnd() const
#endif
{
  return m_pNotificationWnd;
}

BOOL CTrayNotifyIcon::SetFocus()
{
  ATLASSERT(GetShellVersion() >= 5); //Only supported on Shell v5 or greater

  //Call the Shell_NotifyIcon function
  return Shell_NotifyIcon(NIM_SETFOCUS, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
}

LRESULT CTrayNotifyIcon::OnTrayNotification(WPARAM wID, LPARAM lEvent)
{
  //Return quickly if its not for this tray icon
  if (wID != m_NotifyIconData.uID)
    return 0L;

#ifdef _AFX
  CMenu* pSubMenu = m_Menu.GetSubMenu(0);
  ATLASSERT(pSubMenu); //Your menu resource has been designed incorrectly
#else
  CMenuHandle subMenu = m_Menu.GetSubMenu(0);
  ATLASSERT(subMenu.IsMenu());
#endif

  //if (lEvent == WM_RBUTTONUP) //TODO on both events?
  if (lEvent == WM_RBUTTONUP || lEvent == WM_LBUTTONDBLCLK)//TODO on both events for version without UI and open
  {
    CPoint ptCursor;
    GetCursorPos(&ptCursor);
    ::SetForegroundWindow(m_NotifyIconData.hWnd);  
  #ifdef _AFX
    ::TrackPopupMenu(pSubMenu->m_hMenu, TPM_LEFTBUTTON, ptCursor.x, ptCursor.y, 0, m_NotifyIconData.hWnd, NULL);
  #else
    ::TrackPopupMenu(subMenu, TPM_LEFTBUTTON, ptCursor.x, ptCursor.y, 0, m_NotifyIconData.hWnd, NULL);
  #endif
    ::PostMessage(m_NotifyIconData.hWnd, WM_NULL, 0, 0);
  }
  else if (lEvent == WM_LBUTTONDBLCLK) //double click received, the default action is to execute first menu item 
  {
    ::SetForegroundWindow(m_NotifyIconData.hWnd);
  #ifdef _AFX
    UINT nDefaultItem = pSubMenu->GetDefaultItem(GMDI_GOINTOPOPUPS, FALSE);
  #else
    UINT nDefaultItem = subMenu.GetMenuDefaultItem(FALSE, GMDI_GOINTOPOPUPS);
  #endif
    if (nDefaultItem != -1)
      ::SendMessage(m_NotifyIconData.hWnd, WM_COMMAND, nDefaultItem, 0);
  }

  return 1; // handled
}

BOOL CTrayNotifyIcon::GetDynamicDCAndBitmap(CDC* pDC, CBitmap* pBitmap)
{
  //Validate our parameters
  ATLASSERT(pDC != NULL);
  ATLASSERT(pBitmap != NULL);

  //Get the HWND for the desktop
#ifdef _AFX
  CWnd* pWndScreen = CWnd::GetDesktopWindow();
  if (pWndScreen == NULL)
    return FALSE;
#else
  CWindow WndScreen(::GetDesktopWindow());
  if (!WndScreen.IsWindow())
    return FALSE;
#endif

  //Get the desktop HDC to create a compatible bitmap from
#ifdef _AFX
  CDC* pDCScreen = pWndScreen->GetDC();
  if (pDCScreen == NULL)
    return FALSE;
#else
  CDC DCScreen(WndScreen.GetDC());
  if (DCScreen.IsNull())
    return FALSE;
#endif

  //Get the width and height of a small icon
  int w = GetSystemMetrics(SM_CXSMICON);
  int h = GetSystemMetrics(SM_CYSMICON);

  //Create an off-screen bitmap that the dynamic tray icon 
  //can be drawn into. (Compatible with the desktop DC).
#ifdef _AFX
  BOOL bSuccess = pBitmap->CreateCompatibleBitmap(pDCScreen, w, h);
#else
  BOOL bSuccess = (pBitmap->CreateCompatibleBitmap(DCScreen.operator HDC(), w, h) != NULL);
#endif
  if (!bSuccess)
  {
  #ifdef _AFX
    pWndScreen->ReleaseDC(pDCScreen);
  #else
    WndScreen.ReleaseDC(DCScreen);
  #endif
    return FALSE;
  }

  //Get a HDC to the newly created off-screen bitmap
#ifdef _AFX
  bSuccess = pDC->CreateCompatibleDC(pDCScreen);
#else
  bSuccess = (pDC->CreateCompatibleDC(DCScreen.operator HDC()) != NULL);
#endif
  if (!bSuccess)
  {
  //Release the Screen DC now that we are finished with it
  #ifdef _AFX
    pWndScreen->ReleaseDC(pDCScreen);
  #else
    WndScreen.ReleaseDC(DCScreen);
  #endif

    //Free up the bitmap now that we are finished with it
    pBitmap->DeleteObject();

    return FALSE;
  }

  //Select the bitmap into the offscreen DC
#ifdef _AFX
  pDC->SelectObject(pBitmap);
#else
  pDC->SelectBitmap(pBitmap->operator HBITMAP());
#endif

  //Release the Screen DC now that we are finished with it
#ifdef _AFX
  pWndScreen->ReleaseDC(pDCScreen);
#else
  WndScreen.ReleaseDC(DCScreen);
#endif

  return TRUE;
}

DWORD CTrayNotifyIcon::GetShellVersion()
{
  if (sm_dwShellVersion)
    return sm_dwShellVersion;
  else
  {
    typedef HRESULT (CALLBACK DLLGETVERSION)(DLLVERSIONINFO*);
    typedef DLLGETVERSION* LPDLLGETVERSION;

    //What will be the return value
    sm_dwShellVersion = 4; //Assume version 4 of the shell

    //Try to get the details with DllGetVersion
    HMODULE hShell32 = GetModuleHandle(_T("shell32.dll"));
    LPDLLGETVERSION lpfnDllGetVersion = reinterpret_cast<LPDLLGETVERSION>(GetProcAddress(hShell32, "DllGetVersion"));
    if (lpfnDllGetVersion)
    {
      DLLVERSIONINFO vinfo;
      vinfo.cbSize = sizeof(DLLVERSIONINFO);
      if (SUCCEEDED(lpfnDllGetVersion(&vinfo)))
        sm_dwShellVersion = vinfo.dwMajorVersion;
    }
  }
  
  return sm_dwShellVersion;
}

DWORD CTrayNotifyIcon::GetNOTIFYICONDATASizeForOS()
{
  DWORD dwVersion = GetShellVersion();
  if (dwVersion >= 6)
    return sizeof(NOTIFYICONDATA_3);
  else if (dwVersion >= 5)
    return sizeof(NOTIFYICONDATA_2);
  else
    return sizeof(NOTIFYICONDATA_1);
}
