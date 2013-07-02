//  Copyright (C) 2002 Ultr@VNC Team Members. All Rights Reserved.
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


#include "stdhdrs.h"
#include "vncviewer.h"
#include "Exception.h"
#ifdef UNDER_CE
#include "omnithreadce.h"
#else
#include "VNCviewerApp32.h"
#endif

#pragma warning( disable: 4996 )//<func> was declared deprecated

// All logging is done via the log object

char sz_A1[64];
char sz_A2[64];
char sz_A3[64];
char sz_A4[64];
char sz_A5[64];
char sz_B1[64];
char sz_B2[64];
char sz_B3[64];
char sz_C1[64];
char sz_C2[64];
char sz_C3[64];
char sz_D1[64];
char sz_D2[64];
char sz_D3[64];
char sz_D4[64];
char sz_D5[64];
char sz_D6[64];
char sz_D7[64];
char sz_D8[64];
char sz_D9[64];
char sz_D10[64];
char sz_D11[64];
char sz_D12[64];
char sz_D13[64];
char sz_D14[64];
char sz_D15[64];
char sz_D16[64];
char sz_D17[64];
char sz_D18[64];
char sz_D19[64];
char sz_D20[64];
char sz_D21[64];
char sz_D22[64];
char sz_D23[64];
char sz_D24[64];
char sz_D25[64];
char sz_D26[64];
char sz_D27[64];
char sz_D28[64];
char sz_E1[64];
char sz_E2[64];
char sz_F1[64];
char sz_F3[64];
char sz_F4[64];
char sz_F5[128];
char sz_F6[64];
char sz_F7[128];
char sz_F8[128];
char sz_F10[64];
char sz_F11[64];
char sz_G1[64];
char sz_G2[64];
char sz_G3[64];
char sz_H1[64];
char sz_H2[64];
char sz_H3[128];
char sz_H4[64];
char sz_H5[64];
char sz_H6[64];
char sz_H7[64];
char sz_H8[64];
char sz_H9[64];
char sz_H10[64];
char sz_H11[64];
char sz_H12[64];
char sz_H13[64];
char sz_H14[64];
char sz_H15[64];
char sz_H16[64];
char sz_H17[64];
char sz_H18[64];
char sz_H19[64];
char sz_H20[64];
char sz_H21[64];
char sz_H22[64];
char sz_H23[64];
char sz_H24[64];
char sz_H25[64];
char sz_H26[64];
char sz_H27[64];
char sz_H28[64];
char sz_H29[64];
char sz_H30[64];
char sz_H31[64];
char sz_H32[64];
char sz_H33[64];
char sz_H34[64];
char sz_H35[64];
char sz_H36[64];
char sz_H37[64];
char sz_H38[128];
char sz_H39[64];
char sz_H40[64];
char sz_H41[64];
char sz_H42[64];
char sz_H43[128];
char sz_H44[64];
char sz_H45[64];
char sz_H46[128];
char sz_H47[64];
char sz_H48[64];
char sz_H49[64];
char sz_H50[64];
char sz_H51[64];
char sz_H52[64];
char sz_H53[64];
char sz_H54[64];
char sz_H55[64];
char sz_H56[64];
char sz_H57[64];

char sz_H58[64];
char sz_H59[64];
char sz_H60[64];
char sz_H61[64]; 
char sz_H62[128];
char sz_H63[64];
char sz_H64[64];
char sz_H65[64];
char sz_H66[64];
char sz_H67[64];
char sz_H68[128];
char sz_H69[64];
char sz_H70[64];
char sz_H71[64];
char sz_H72[128];
char sz_H73[64];

char sz_I1[64];
char sz_I2[64];
char sz_I3[64];
char sz_J1[128];
char sz_J2[64];
char sz_K1[64];
char sz_K2[64];
char sz_K3[256];
char sz_K4[64];
char sz_K5[64];
char sz_K6[64];
char sz_K7[64];
char sz_L1[64];
char sz_L2[64];
char sz_L3[64];
char sz_L4[64];
char sz_L5[64];
char sz_L6[64];
char sz_L7[64];
char sz_L8[64];
char sz_L9[64];
char sz_L10[64];
char sz_L11[64];
char sz_L12[64];
char sz_L13[64];
char sz_L14[64];
char sz_L15[64];
char sz_L16[64];
char sz_L17[64];
char sz_L18[64];
char sz_L19[64];
char sz_L20[64];
char sz_L21[64];
char sz_L22[64];
char sz_L23[64];
char sz_L24[64];
char sz_L25[64];
char sz_L26[64];
char sz_L27[64];
char sz_L28[64];
char sz_L29[64];
char sz_L30[64];
char sz_L31[64];
char sz_L32[64];
char sz_L33[64];
char sz_L34[64];
char sz_L35[64];
char sz_L36[64];
char sz_L37[64];
char sz_L38[64];
char sz_L39[64];
char sz_L40[64];
char sz_L41[64];
char sz_L42[64];
char sz_L43[64];
char sz_L44[64];
char sz_L45[64];
char sz_L46[64];
char sz_L47[64];
char sz_L48[64];
char sz_L49[64];
char sz_L50[64];
char sz_L51[64];
char sz_L52[64];
char sz_L53[64];
char sz_L54[64];
char sz_L55[64];
char sz_L56[64];
char sz_L57[64];
char sz_L58[64];
char sz_L59[64];
char sz_L60[64];
char sz_L61[64];
char sz_L62[64];
char sz_L63[64];
char sz_L64[64];
char sz_L65[64];
char sz_L66[64];
char sz_L67[64];
char sz_L68[64];
char sz_L69[64];
char sz_L70[64];
char sz_L71[64];
char sz_L72[64];
char sz_L73[64];
char sz_L74[64];
char sz_L75[64];
char sz_L76[64];
char sz_L77[64];
char sz_L78[64];
char sz_L79[64];
char sz_L80[64];
char sz_L81[64];
char sz_L82[64];
char sz_L83[64];
char sz_L84[64];
char sz_L85[64];
char sz_L86[64];
char sz_L87[64];
char sz_L88[64];
char sz_L89[64];
char sz_L90[64];
char sz_L91[64];
char sz_L92[64];

// File/dir Rename messages
char sz_M1[64];
char sz_M2[64];
char sz_M3[64];
char sz_M4[64];
char sz_M5[64];
char sz_M6[64];
char sz_M7[64];
char sz_M8[64];

bool command_line=true;
bool g_passwordfailed=true;


// Accelerator Keys
AccelKeys TheAccelKeys;
HINSTANCE m_hInstResDLL;


// Move the given window to the centre of the screen
// and bring it to the top.
void CentreWindow(HWND hwnd)
{
	RECT winrect, workrect;
	
	// Find how large the desktop work area is
	SystemParametersInfo(SPI_GETWORKAREA, 0, &workrect, 0);
	int workwidth = workrect.right -  workrect.left;
	int workheight = workrect.bottom - workrect.top;
	
	// And how big the window is
	GetWindowRect(hwnd, &winrect);
	int winwidth = winrect.right - winrect.left;
	int winheight = winrect.bottom - winrect.top;
	// Make sure it's not bigger than the work area
	winwidth = min(winwidth, workwidth);
	winheight = min(winheight, workheight);

	// Now centre it
	SetWindowPos(hwnd, 
		HWND_TOP,
		workrect.left + (workwidth-winwidth) / 2,
		workrect.top + (workheight-winheight) / 2,
		winwidth, winheight, 
		SWP_SHOWWINDOW);
	SetForegroundWindow(hwnd);
}


// sf@2002 - TightVNC method - RealVNC method
// Convert "host:display" or "host::port" or "host:port" if port > 100, into host and port
// Returns true if valid format, false if not.
// Takes initial string, addresses of results and size of host buffer in wchars.
// If the display info passed in is longer than the size of the host buffer, it
// is assumed to be invalid, so false is returned.
bool ParseDisplay(LPTSTR display, LPTSTR phost, int hostlen, int *pport) 
{
    if (hostlen < (int)_tcslen(display))
        return false;

    int tmp_port;
    TCHAR *colonpos = _tcschr(display, L':');
    if (colonpos == NULL)
	{
		// No colon -- use default port number
        tmp_port = RFB_PORT_OFFSET;
		_tcsncpy(phost, display, MAX_HOST_NAME_LEN);
	}
	else
	{
		_tcsncpy(phost, display, colonpos - display);
		phost[colonpos - display] = L'\0';
		if (colonpos[1] == L':') {
			// Two colons -- interpret as a port number
			if (_stscanf(colonpos + 2, TEXT("%d"), &tmp_port) != 1) 
				return false;
		}
		else
		{
			// One colon -- interpret as a display number or port number
			if (_stscanf(colonpos + 1, TEXT("%d"), &tmp_port) != 1) 
				return false;

			// RealVNC method - If port < 100 interpret as display number else as Port number
			if (tmp_port < 100)
				tmp_port += RFB_PORT_OFFSET;
		}
	}
    *pport = tmp_port;
    return true;
}

HINSTANCE g_hinstDLL;

//BOOL WINAPI DllMain(HINSTANCE hinstDLL,	DWORD fdwReason,LPVOID lpvReserved)
//{
//	if( fdwReason == DLL_PROCESS_ATTACH )
//		g_hinstDLL = hinstDLL;
//	return TRUE;
//}





