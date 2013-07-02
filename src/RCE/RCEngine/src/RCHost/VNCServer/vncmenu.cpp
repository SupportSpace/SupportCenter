//  Copyright (C) 2002 Ultr@VNC Team Members. All Rights Reserved.
//  Copyright (C) 2002 RealVNC Ltd. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
//
//  This file is part of the VNC system.
//
//  The VNC system is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
// If the source code for the VNC system is not available from the place 
// whence you received this file, check http://www.uk.research.att.com/vnc or contact
// the authors on vnc@uk.research.att.com for information on obtaining it.


// vncMenu

// Implementation of a system tray icon & menu for WinVNC

#include "stdhdrs.h"
#include "WinVNC.h"
#include "vncService.h"
//#include "vncConnDialog.h"
#include <lmcons.h>
#include <wininet.h>
#include <shlobj.h>

// Header

#include "vncMenu.h"
//#include "HideDesktop.h"

// Constants
const UINT MENU_PROPERTIES_SHOW = RegisterWindowMessage("WinVNC.Properties.User.Show");
const UINT MENU_DEFAULT_PROPERTIES_SHOW = RegisterWindowMessage("WinVNC.Properties.Default.Show");
const UINT MENU_ABOUTBOX_SHOW = RegisterWindowMessage("WinVNC.AboutBox.Show");
const UINT MENU_SERVICEHELPER_MSG = RegisterWindowMessage("WinVNC.ServiceHelper.Message");
const UINT MENU_ADD_CLIENT_MSG = RegisterWindowMessage("WinVNC.AddClient.Message");
const UINT MENU_REMOVE_CLIENTS_MSG = RegisterWindowMessage("WinVNC.RemoveClients.Message"); // REalVNc 336
const UINT MENU_AUTO_RECONNECT_MSG = RegisterWindowMessage("WinVNC.AddAutoClient.Message");

const UINT FileTransferSendPacketMessage = RegisterWindowMessage("UltraVNC.Viewer.FileTransferSendPacketMessage");

const char *MENU_CLASS_NAME = "WinVNC Tray Icon";

BOOL g_restore_ActiveDesktop = FALSE;
bool RunningAsAdministrator ();

static void
KillWallpaper()
{
//	HideDesktop();
}

static void
RestoreWallpaper()
{
//  RestoreDesktop();
}