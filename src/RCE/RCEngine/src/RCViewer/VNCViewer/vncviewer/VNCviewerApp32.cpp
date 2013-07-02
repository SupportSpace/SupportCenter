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

#include "VNCviewerApp32.h"
#include "vncviewer.h"
#include "Exception.h"
extern char sz_A1[64];
extern char sz_A2[64];
extern char sz_A3[64];
extern char sz_A4[64];
extern char sz_A5[64];

#pragma warning( disable: 4996 )//<func> was declared deprecated

// --------------------------------------------------------------------------
VNCviewerApp32::VNCviewerApp32(HWND hWnd, PSTR szCmdLine) 
	:	VNCviewerApp(0, szCmdLine)
{
TRY_CATCH

	m_hWnd = hWnd;
	m_pflasher = NULL;
//HKL hkl = LoadKeyboardLayout(  "00000813", 
//			KLF_ACTIVATE | KLF_REPLACELANG | KLF_REORDER  );
	// Load a requested keyboard layout
	if (m_options.m_kbdSpecified) {
		HKL hkl = LoadKeyboardLayout(  m_options.m_kbdname, 
			KLF_ACTIVATE | KLF_REPLACELANG | KLF_REORDER  );
		if (hkl == NULL) {
			MessageBox(NULL, sz_A1, 
				sz_A2, MB_OK | MB_ICONSTOP);
			exit(1);
		}
	}

CATCH_THROW("VNCviewerApp32::VNCviewerApp32")
}


void VNCviewerApp32::NewConnection(boost::shared_ptr<CAbstractStream> stream, CRCViewer* viewerPtr)
{
TRY_CATCH

	TRY_CATCH
		m_pcc.reset(new ClientConnection(this, stream,viewerPtr , m_hWnd ));
		m_pcc->Run();
		return;
	CATCH_LOG("VNCviewerApp32::NewConnection")
	m_pcc.reset();

CATCH_THROW("VNCviewerApp32::NewConnection")
}

void VNCviewerApp32::SetDisplayMode( EDisplayMode mode )
{
TRY_CATCH

	m_options.m_displayMode = mode;
	if(m_pcc.get() != NULL) 
	{
		m_pcc->ChangeDisplayMode( mode );
	}
CATCH_THROW("VNCviewerApp32::SetDisplayMode")
}

VNCviewerApp32::~VNCviewerApp32()
{
TRY_CATCH

	// We don't need to clean up pcc if the thread has been joined.
	if (m_pflasher != NULL) delete m_pflasher;
	///if (m_pcc) delete m_pcc;
	/// Since it should be deleted from the window's thread
	m_pcc.release();

CATCH_LOG("VNCviewerApp32::~VNCviewerApp32")
}
	
