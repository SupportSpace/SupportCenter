//  Copyright (C) 2002 Ultr@VNC Team Members. All Rights Reserved.
//
//  Copyright (C) 2000-2002 Const Kaplinsky. All Rights Reserved.
//
// Copyright (C) 2002 RealVNC Ltd. All Rights Reserved.
//
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


// VNCOptions.cpp: implementation of the VNCOptions class.

#pragma warning( disable: 4996 )//<func> was declared deprecated

#include "stdhdrs.h"
#include "vncviewer.h"
#include "VNCOptions.h"
#include "Exception.h"
extern char sz_A2[64];
extern char sz_D1[64];
extern char sz_D2[64];
extern char sz_D3[64];
extern char sz_D4[64];
extern char sz_D5[64];
extern char sz_D6[64];
extern char sz_D7[64];
extern char sz_D8[64];
extern char sz_D9[64];
extern char sz_D10[64];
extern char sz_D11[64];
extern char sz_D12[64];
extern char sz_D13[64];
extern char sz_D14[64];
extern char sz_D15[64];
extern char sz_D16[64];
extern char sz_D17[64];
extern char sz_D18[64];
extern char sz_D19[64];
extern char sz_D20[64];
extern char sz_D21[64];
extern char sz_D22[64];
extern char sz_D23[64];
extern char sz_D24[64];
extern char sz_D25[64];
extern char sz_D26[64];
extern char sz_D27[64];
extern char sz_D28[64];


VNCOptions::VNCOptions()
{
	m_displayMode = SCALE_MODE;
	m_dx = 0.0f;
	m_dy = 0.0f;

	for (int i = 0; i <= LASTENCODING; i++)
		m_UseEnc[i] = false;

	m_UseEnc[rfbEncodingRaw] = true;
	m_UseEnc[rfbEncodingCopyRect] = true;
	m_UseEnc[rfbEncodingRRE] = true;
	m_UseEnc[rfbEncodingCoRRE] = true;
	m_UseEnc[rfbEncodingHextile] = true;
	m_UseEnc[rfbEncodingZlib] = true;
	m_UseEnc[rfbEncodingTight] = true;
	m_UseEnc[rfbEncodingZlibHex] = true;
	m_UseEnc[rfbEncodingZRLE] = true;
	m_UseEnc[rfbEncodingUltra] = true;

	m_ViewOnly = false;
	m_FullScreen = false;
	autoDetect = true;
	m_Use8Bit = rfbPFFullColors; //false;
	m_ShowToolbar = true;
	m_fAutoScaling = false;
	m_NoStatus = false;
	m_NoHotKeys = false;

#ifndef UNDER_CE
	//m_PreferredEncoding = rfbEncodingHextile;
	m_PreferredEncoding = rfbEncodingZRLE;
#else
	// With WinCE2.0, CoRRE seems more efficient since it
	// reads the whole update in one socket call.
	m_PreferredEncoding = rfbEncodingCoRRE;
#endif
	m_SwapMouse = false;
	m_Emul3Buttons = true;
	m_Emul3Timeout = 100; // milliseconds
	m_Emul3Fuzz = 4;      // pixels away before emulation is cancelled
	m_Shared = true;
	m_DeiconifyOnBell = false;
	m_DisableClipboard = false;
	m_localCursor = DOTCURSOR; // NOCURSOR;
	m_scaling = false;
	m_scaleUpEnabled = false;
	m_fAutoScaling = false;
	m_scale_num = 100;
	m_scale_den = 100;


	// Modif sf@2002 - Server Scaling
	m_nServerScale = 1;

	// Modif sf@2002 - Cache
	m_fEnableCache = false;
	// m_fAutoAdjust = false;

	m_host_options[0] = '\0';
	m_proxyhost[0] = '\0';
	m_port = -1;
	m_proxyport = -1;

	m_kbdname[0] = '\0';
	m_kbdSpecified = false;

	m_logLevel = 0;
	m_logToConsole = false;
	m_logToFile = false;
	m_logFilename[0] = '\0';

	m_delay=0;
	m_connectionSpecified = false;
	m_configSpecified = false;
	m_configFilename[0] = '\0';
	m_listening = false;
	m_listenPort = INCOMING_PORT_OFFSET;
	m_restricted = false;

	// Tight specific
	m_useCompressLevel = true;
	m_compressLevel = 6;		
	m_enableJpegCompression = true;
	m_jpegQualityLevel = 6;
	m_requestShapeUpdates = true;
	m_ignoreShapeUpdates = false;

	m_clearPassword[0] = '\0';		// sf@2002
	m_quickoption = 1;				// sf@2002 - Auto Mode as default
	m_fUseDSMPlugin = false;
	m_fUseProxy = false;
	m_szDSMPluginFilename[0] = '\0';

	// sf@2003 - Auto Scaling
	m_saved_scale_num = 100;
	m_saved_scale_den = 100;
	m_saved_scaling = false;


#ifdef UNDER_CE
	m_palmpc = false;

	// Check for PalmPC aspect 
	HDC temp_hdc = GetDC(NULL);
	int screen_width = GetDeviceCaps(temp_hdc, HORZRES);
	if (screen_width < 320)
	{
		m_palmpc = true;
	}
	ReleaseDC(NULL,temp_hdc);

	m_slowgdi = false;
#endif
}

VNCOptions& VNCOptions::operator=(VNCOptions& s)
{
	m_dx = s.m_dx;
	m_dy = s.m_dy;

	m_displayMode = s.m_displayMode;

	m_scale_num_y = s.m_scale_num_y;
	for (int i = rfbEncodingRaw; i<= LASTENCODING; i++)
		m_UseEnc[i] = s.m_UseEnc[i];

	m_ViewOnly			= s.m_ViewOnly;
	m_NoStatus			= s.m_NoStatus;
	m_FullScreen		= s.m_FullScreen;
	autoDetect = s.autoDetect;
	m_Use8Bit			= s.m_Use8Bit;
	m_PreferredEncoding = s.m_PreferredEncoding;
	m_SwapMouse			= s.m_SwapMouse;
	m_Emul3Buttons		= s.m_Emul3Buttons;
	m_Emul3Timeout		= s.m_Emul3Timeout;
	m_Emul3Fuzz			= s.m_Emul3Fuzz;      // pixels away before emulation is cancelled
	m_Shared			= s.m_Shared;
	m_DeiconifyOnBell	= s.m_DeiconifyOnBell;
	m_DisableClipboard  = s.m_DisableClipboard;
	m_scaling			= s.m_scaling;
	m_fAutoScaling    = s.m_fAutoScaling;
	m_scale_num			= s.m_scale_num;
	m_scale_den			= s.m_scale_den;
	m_localCursor		= s.m_localCursor;
	// Modif sf@2002
	m_nServerScale  	  = s.m_nServerScale;
	m_fEnableCache      = s.m_fEnableCache;
	m_quickoption       = s.m_quickoption;
	m_ShowToolbar       = s.m_ShowToolbar;
	m_fAutoScaling      = s.m_fAutoScaling;
	m_fUseDSMPlugin     = s.m_fUseDSMPlugin;
	m_NoHotKeys		  = s.m_NoHotKeys;

	// sf@2003 - Autoscaling
	m_saved_scale_num = s.m_saved_scale_num;
	m_saved_scale_den = s.m_saved_scale_den;
	m_saved_scaling   = s.m_saved_scaling;

	strcpy(m_szDSMPluginFilename, s.m_szDSMPluginFilename);

	strcpy(m_host_options, s.m_host_options);
	m_port				= s.m_port;

	strcpy(m_proxyhost, s.m_proxyhost);
	m_proxyport				= s.m_proxyport;
	m_fUseProxy	      = s.m_fUseProxy;

	strcpy(m_kbdname, s.m_kbdname);
	m_kbdSpecified		= s.m_kbdSpecified;

	m_logLevel			= s.m_logLevel;
	m_logToConsole		= s.m_logToConsole;
	m_logToFile			= s.m_logToFile;
	strcpy(m_logFilename, s.m_logFilename);

	m_delay				= s.m_delay;
	m_connectionSpecified = s.m_connectionSpecified;
	m_configSpecified   = s.m_configSpecified;
	strcpy(m_configFilename, s.m_configFilename);

	m_listening			= s.m_listening;
	m_listenPort			= s.m_listenPort;
	m_restricted			= s.m_restricted;

	// Tight specific
	m_useCompressLevel		= s.m_useCompressLevel;
	m_compressLevel			= s.m_compressLevel;
	m_enableJpegCompression	= s.m_enableJpegCompression;
	m_jpegQualityLevel		= s.m_jpegQualityLevel;
	m_requestShapeUpdates	    = s.m_requestShapeUpdates;
	m_ignoreShapeUpdates	    = s.m_ignoreShapeUpdates;

#ifdef UNDER_CE
	m_palmpc			= s.m_palmpc;
	m_slowgdi			= s.m_slowgdi;
#endif
	return *this;
}

VNCOptions::~VNCOptions()
{

}

inline bool SwitchMatch(LPCTSTR arg, LPCTSTR swtch) {
	return (arg[0] == '-' || arg[0] == '/') &&
		(_tcsicmp(&arg[1], swtch) == 0);
}

static void ArgError(LPTSTR msg) {
	MessageBox(NULL,  msg, sz_D1,MB_OK | MB_TOPMOST | MB_ICONSTOP);
}

// Greatest common denominator, by Euclid
int gcd(int a, int b) {
	if (a < b) return gcd(b,a);
	if (b == 0) return a;
	return gcd(b, a % b);
}

void VNCOptions::FixScaling()
{
	if (m_scale_num < 1 || m_scale_den < 1 || m_scale_num > 400 || m_scale_den > 100)
	{
		MessageBox(NULL,  sz_D2, 
			sz_D1,MB_OK | MB_TOPMOST | MB_ICONWARNING);
		m_scale_num = 1;
		m_scale_den = 1;	
		m_scaling = false;
	}
	int g = gcd(m_scale_num, m_scale_den);
	m_scale_num /= g;
	m_scale_den /= g;	

	// Modif sf@2002 - Server Scaling
	if (m_nServerScale < 1 || m_nServerScale > 9) m_nServerScale = 1;
}

void VNCOptions::SetFromCommandLine(LPTSTR szCmdLine) {
	/*
	// We assume no quoting here.
	// Copy the command line - we don't know what might happen to the original
	int cmdlinelen = _tcslen(szCmdLine);
	if (cmdlinelen == 0) return;

	TCHAR *cmd = new TCHAR[cmdlinelen + 1];
	_tcscpy(cmd, szCmdLine);

	// Count the number of spaces
	// This may be more than the number of arguments, but that doesn't matter.
	int nspaces = 0;
	TCHAR *p = cmd;
	TCHAR *pos = cmd;
	while ( ( pos = _tcschr(p, ' ') ) != NULL ) {
	nspaces ++;
	p = pos + 1;
	}

	// Create the array to hold pointers to each bit of string
	TCHAR **args = new LPTSTR[nspaces + 1];

	// replace spaces with nulls and
	// create an array of TCHAR*'s which points to start of each bit.
	pos = cmd;
	int i = 0;
	args[i] = cmd;
	bool inquote=false;
	for (pos = cmd; *pos != 0; pos++) {
	// Arguments are normally separated by spaces, unless there's quoting
	if ((*pos == ' ') && !inquote) {
	*pos = '\0';
	p = pos + 1;
	args[++i] = p;
	}
	if (*pos == '"') {  
	if (!inquote) {      // Are we starting a quoted argument?
	args[i] = ++pos; // It starts just after the quote
	} else {
	*pos = '\0';     // Finish a quoted argument?
	}
	inquote = !inquote;
	}
	}
	i++;

	bool hostGiven = false, portGiven = false;
	// take in order.
	for (int j = 0; j < i; j++) {
	if ( SwitchMatch(args[j], _T("help")) ||
	SwitchMatch(args[j], _T("?")) ||
	SwitchMatch(args[j], _T("h")))
	{
	m_NoStatus = true;
	ShowUsage();
	PostQuitMessage(1);
	}
	else if ( SwitchMatch(args[j], _T("listen")))
	{
	m_listening = true;
	if (j+1 < i && args[j+1][0] >= '0' && args[j+1][0] <= '9') {
	if (_stscanf(args[j+1], _T("%d"), &m_listenPort) != 1) {
	ArgError(sz_D3);
	continue;
	}
	j++;
	}
	} else if ( SwitchMatch(args[j], _T("restricted"))) {
	m_restricted = true;
	} else if ( SwitchMatch(args[j], _T("viewonly"))) {
	m_ViewOnly = true;
	} else if ( SwitchMatch(args[j], _T("nostatus"))) {
	m_NoStatus = true;
	} else if ( SwitchMatch(args[j], _T("nohotkeys"))) {
	m_NoHotKeys = true;
	} else if ( SwitchMatch(args[j], _T("notoolbar"))) {
	m_ShowToolbar = false;
	} else if ( SwitchMatch(args[j], _T("autoscaling"))) {
	m_fAutoScaling = true;
	} else if ( SwitchMatch(args[j], _T("fullscreen"))) {
	m_FullScreen = true;
	} else if ( SwitchMatch(args[j], _T("noauto"))) {
	autoDetect = false;
	m_quickoption = 0;
	} else if ( SwitchMatch(args[j], _T("8bit"))) {
	m_Use8Bit = rfbPF256Colors; //true;
	} else if ( SwitchMatch(args[j], _T("256colors"))) {
	m_Use8Bit = rfbPFFullColors;
	} else if ( SwitchMatch(args[j], _T("64colors"))) {
	m_Use8Bit = rfbPF64Colors;
	} else if ( SwitchMatch(args[j], _T("8colors"))) {
	m_Use8Bit = rfbPF8Colors;
	} else if ( SwitchMatch(args[j], _T("8greycolors"))) {
	m_Use8Bit = rfbPF8GreyColors;
	} else if ( SwitchMatch(args[j], _T("4greycolors"))) {
	m_Use8Bit = rfbPF4GreyColors;
	} else if ( SwitchMatch(args[j], _T("2greycolors"))) {
	m_Use8Bit = rfbPF2GreyColors;
	} else if ( SwitchMatch(args[j], _T("shared"))) {
	m_Shared = true;
	} else if ( SwitchMatch(args[j], _T("swapmouse"))) {
	m_SwapMouse = true;
	} else if ( SwitchMatch(args[j], _T("nocursor"))) {
	m_localCursor = NOCURSOR;
	} else if ( SwitchMatch(args[j], _T("dotcursor"))) {
	m_localCursor = DOTCURSOR;
	} else if ( SwitchMatch(args[j], _T("normalcursor"))) {
	m_localCursor = NORMALCURSOR;
	} else if ( SwitchMatch(args[j], _T("belldeiconify") )) {
	m_DeiconifyOnBell = true;
	} else if ( SwitchMatch(args[j], _T("emulate3") )) {
	m_Emul3Buttons = true;
	} else if ( SwitchMatch(args[j], _T("noemulate3") )) {
	m_Emul3Buttons = false;
	} else if ( SwitchMatch(args[j], _T("nocursorshape") )) {
	m_requestShapeUpdates = false;
	} else if ( SwitchMatch(args[j], _T("noremotecursor") )) {
	m_requestShapeUpdates = true;
	m_ignoreShapeUpdates = true;
	} else if ( SwitchMatch(args[j], _T("scale") )) {
	if (++j == i) {
	ArgError(sz_D4);
	continue;
	}
	int numscales = _stscanf(args[j], _T("%d/%d"), &m_scale_num, &m_scale_den);
	if (numscales < 1) {
	ArgError(sz_D5);
	continue;
	}
	if (numscales == 1) 
	m_scale_den = 1; // needed if you're overriding a previous setting

	} else if ( SwitchMatch(args[j], _T("emulate3timeout") )) {
	if (++j == i) {
	ArgError(sz_D6);
	continue;
	}
	if (_stscanf(args[j], _T("%d"), &m_Emul3Timeout) != 1) {
	ArgError(sz_D7);
	continue;
	}

	} else if ( SwitchMatch(args[j], _T("emulate3fuzz") )) {
	if (++j == i) {
	ArgError(sz_D8);
	continue;
	}
	if (_stscanf(args[j], _T("%d"), &m_Emul3Fuzz) != 1) {
	ArgError(sz_D9);
	continue;
	}

	} else if ( SwitchMatch(args[j], _T("disableclipboard") )) {
	m_DisableClipboard = true;
	}
	#ifdef UNDER_CE
	// Manual setting of palm vs hpc aspect ratio for dialog boxes.
	else if ( SwitchMatch(args[j], _T("hpc") )) {
	m_palmpc = false;
	} else if ( SwitchMatch(args[j], _T("palm") )) {
	m_palmpc = true;
	} else if ( SwitchMatch(args[j], _T("slow") )) {
	m_slowgdi = true;
	} 
	#endif
	else if ( SwitchMatch(args[j], _T("delay") )) {
	if (++j == i) {
	ArgError(sz_D10);
	continue;
	}
	if (_stscanf(args[j], _T("%d"), &m_delay) != 1) {
	ArgError(sz_D11);
	continue;
	}

	} else if ( SwitchMatch(args[j], _T("loglevel") )) {
	if (++j == i) {
	ArgError(sz_D12);
	continue;
	}
	if (_stscanf(args[j], _T("%d"), &m_logLevel) != 1) {
	ArgError(sz_D13);
	continue;
	}

	} else if ( SwitchMatch(args[j], _T("console") )) {
	m_logToConsole = true;
	} else if ( SwitchMatch(args[j], _T("logfile") )) {
	if (++j == i) {
	ArgError(sz_D14);
	continue;
	}
	if (_stscanf(args[j], _T("%s"), m_logFilename) != 1) {
	ArgError(sz_D15);
	continue;
	} else {
	m_logToFile = true;
	}
	} else if ( SwitchMatch(args[j], _T("config") )) {
	if (++j == i) {
	ArgError(sz_D16);
	continue;
	}
	// The GetPrivateProfile* stuff seems not to like some relative paths
	_fullpath(m_configFilename, args[j], _MAX_PATH);
	if (_access(m_configFilename, 04)) {
	ArgError(sz_D17);
	PostQuitMessage(1);
	continue;
	} else {
	Load(m_configFilename);
	m_configSpecified = true;
	}
	} else if ( SwitchMatch(args[j], _T("register") )) {
	Register();
	PostQuitMessage(0);

	}
	else if ( SwitchMatch(args[j], _T("encoding") )) {
	if (++j == i) {
	ArgError(sz_D18);
	continue;
	}
	int enc = -1;
	if (_tcsicmp(args[j], _T("raw")) == 0) {
	enc = rfbEncodingRaw;
	} else if (_tcsicmp(args[j], _T("rre")) == 0) {
	enc = rfbEncodingRRE;
	} else if (_tcsicmp(args[j], _T("corre")) == 0) {
	enc = rfbEncodingCoRRE;
	} else if (_tcsicmp(args[j], _T("hextile")) == 0) {
	enc = rfbEncodingHextile;
	} else if (_tcsicmp(args[j], _T("zlib")) == 0) {
	enc = rfbEncodingZlib;
	} else if (_tcsicmp(args[j], _T("zlibhex")) == 0) {
	enc = rfbEncodingZlibHex;
	} else if (_tcsicmp(args[j], _T("tight")) == 0) {
	enc = rfbEncodingTight;
	} else if (_tcsicmp(args[j], _T("ultra")) == 0) {
	enc = rfbEncodingUltra;
	} else {
	ArgError(sz_D19);
	continue;
	}
	if (enc != -1) {
	m_UseEnc[enc] = true;
	m_PreferredEncoding = enc;
	}
	}
	// Tight options
	else if ( SwitchMatch(args[j], _T("compresslevel") )) {
	if (++j == i) {
	ArgError(sz_D20);
	continue;
	}
	m_useCompressLevel = true;
	if (_stscanf(args[j], _T("%d"), &m_compressLevel) != 1) {
	ArgError(sz_D21);
	continue;
	}
	} else if ( SwitchMatch(args[j], _T("quality") )) {
	if (++j == i) {
	ArgError(sz_D22);
	continue;
	}
	m_enableJpegCompression = true;
	if (_stscanf(args[j], _T("%d"), &m_jpegQualityLevel) != 1) {
	ArgError(sz_D23);
	continue;
	}
	}
	// Modif sf@2002 : password in the command line
	else if ( SwitchMatch(args[j], _T("password") ))
	{
	if (++j == i)
	{
	ArgError(sz_D24);
	continue;
	}
	strcpy(m_clearPassword, args[j]);
	} // Modif sf@2002
	else if ( SwitchMatch(args[j], _T("serverscale") ))
	{
	if (++j == i)
	{
	ArgError(sz_D25);
	continue;
	}
	_stscanf(args[j], _T("%d"), &m_nServerScale);
	if (m_nServerScale < 1 || m_nServerScale > 9) m_nServerScale = 1;
	}
	// Modif sf@2002
	else if ( SwitchMatch(args[j], _T("quickoption") )) 
	{
	if (++j == i)
	{
	ArgError(sz_D26);
	continue;
	}
	_stscanf(args[j], _T("%d"), &m_quickoption);
	}
	// Modif sf@2002 - DSM Plugin 
	else if ( SwitchMatch(args[j], _T("dsmplugin") ))
	{
	if (++j == i)
	{
	ArgError(sz_D27);
	continue;
	}
	m_fUseDSMPlugin = true;
	strcpy(m_szDSMPluginFilename, args[j]);
	}
	else if ( SwitchMatch(args[j], _T("proxy") ))
	{
	if (++j == i)
	{
	ArgError(sz_D27); // sf@ - Todo: put correct message here
	continue;
	}
	TCHAR proxyhost[256];
	if (!ParseDisplay(args[j], proxyhost, 255, &m_proxyport)) {
	ShowUsage(sz_D28);
	PostQuitMessage(1);
	} else {
	m_fUseProxy = true;
	_tcscpy(m_proxyhost, proxyhost);
	}
	}
	else
	{
	TCHAR phost[256];
	if (!ParseDisplay(args[j], phost, 255, &m_port)) {
	ShowUsage(sz_D28);
	PostQuitMessage(1);
	} else {
	_tcscpy(m_host_options, phost);
	m_connectionSpecified = true;
	}
	}
	}       

	if (m_scale_num != 1 || m_scale_den != 1) 			
	m_scaling = true;

	// reduce scaling factors by greatest common denominator
	if (m_scaling) {
	FixScaling();
	}
	// tidy up
	delete [] cmd;
	delete [] args;
	*/
}



void saveInt(char *name, int value, char *fname) 
{
	char buf[4];
	sprintf(buf, "%d", value); 
	WritePrivateProfileString("options", name, buf, fname);
}

int readInt(char *name, int defval, char *fname)
{
	return GetPrivateProfileInt("options", name, defval, fname);
}