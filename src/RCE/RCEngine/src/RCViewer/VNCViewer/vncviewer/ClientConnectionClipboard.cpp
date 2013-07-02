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
// whence you received this file, check http://www.uk.research.att.com/vnc or 
// contact the authors on vnc@uk.research.att.com for information on obtaining it.
//

#pragma warning( disable: 4996 )//<func> was declared deprecated

#include "stdhdrs.h"
#include "vncviewer.h"
#include "ClientConnection.h"
#include "Exception.h"
extern char sz_C1[64];
extern char sz_C2[64];
extern char sz_C3[64];

// This file contains the code for getting text from, and putting text into
// the Windows clipboard.

//
// ProcessClipboardChange
// Called by ClientConnection::WndProc.
// We've been informed that the local clipboard has been updated.
// If it's text we want to send it to the server.
//

void ClientConnection::ProcessLocalClipboardChange()
{
TRY_CATCH

	//vnclog.Print(2, _T("Clipboard changed\n"));
	
	HWND hOwner = GetClipboardOwner();
	if (hOwner == m_hwnd) 
	{
		//vnclog.Print(2, _T("We changed it - ignore!\n"));
	} 
	else if (!m_initialClipboardSeen) 
	{
		//vnclog.Print(2, _T("Don't send initial clipboard!\n"));
		m_initialClipboardSeen = true;
	} 
	else if (!m_opts.m_DisableClipboard) 
	{
		
		// The clipboard should not be modified by more than one thread at once
		boost::recursive_mutex::scoped_lock l(m_clipMutex);
		
		if (OpenClipboard(m_hwnd)) 
		{ 
			HGLOBAL hglb = GetClipboardData(CF_TEXT); 
			if (hglb == NULL) 
			{
				CloseClipboard();
			} 
			else 
			{
				LPSTR lpstr = (LPSTR) GlobalLock(hglb);  
				if (lpstr != NULL && lpstr != m_recentClipboardText)
				{
					m_recentClipboardText = lpstr;
					char *contents = new char[strlen(lpstr) + 1];
					char *unixcontents = new char[strlen(lpstr) + 1];
					strcpy(contents,lpstr);
					GlobalUnlock(hglb); 
					CloseClipboard();       		
					
					// Translate to Unix-format lines before sending
					int j = 0;
					for (int i = 0; contents[i] != '\0'; i++) {
						if (contents[i] != '\x0d') {
							unixcontents[j++] = contents[i];
						}
					}
					unixcontents[j] = '\0';
					try 
					{
						SendClientCutText(unixcontents, strlen(unixcontents));
					} 
					catch (WarningException &e) 
					{
						//vnclog.Print(0, _T("Exception while sending clipboard text : %s\n"), e.m_info);
						Log.Add(_MESSAGE_, "Exception while sending clipboard text : %s", e.m_info);
						DestroyWindow(m_hwnd);
					}
					delete [] contents; 
					delete [] unixcontents;
				} else
				{
					GlobalUnlock(hglb); 
					CloseClipboard();  
				}
			}
		}
	}
	// Pass the message to the next window in clipboard viewer chain
	::SendMessage(m_hwndNextViewer, WM_DRAWCLIPBOARD , 0,0); 

CATCH_THROW("ClientConnection::ProcessLocalClipboardChange")
}

// We've read some text from the remote server, and
// we need to copy it into the local clipboard.
// Called by ClientConnection::ReadServerCutText()

void ClientConnection::UpdateLocalClipboard(char *buf, int len) 
{
TRY_CATCH	

	if (m_opts.m_DisableClipboard)
		return;

	// Copy to wincontents replacing LF with CR-LF
	std::auto_ptr<char> wincontents;
	wincontents.reset(new char[len * 2 + 2]);
	int j = 0;;
	for (int i = 0; buf[i] != 0 && i<len; i++, j++) 
	{
        if (buf[i] == '\x0a') 
		{
			wincontents.get()[j++] = '\x0d';
            len++;
        }
		wincontents.get()[j] = buf[i];
	}
	wincontents.get()[j] = 0;
	wincontents.get()[j+1] = 0;

    // The clipboard should not be modified by more than one thread at once
    {
		boost::recursive_mutex::scoped_lock l(m_clipMutex);

        if (!OpenClipboard(m_hwnd)) 
		{
			Log.WinError(_WARNING_,"OpenClipboard() failed");
			return;
        }
        if (!::EmptyClipboard()) 
		{
			Log.WinError(_WARNING_,"EmptyClipboard() failed");
			return;
        }

        // Allocate a global memory object for the text. 
        HGLOBAL hglbCopy = GlobalAlloc(GMEM_DDESHARE, (len +1) * sizeof(TCHAR));
        if (hglbCopy != NULL)
		{ 
	        // Lock the handle and copy the text to the buffer.  
	        LPTSTR lptstrCopy = (LPTSTR) GlobalLock(hglbCopy); 
			memcpy(lptstrCopy, wincontents.get(), len * sizeof(TCHAR)); 
	        lptstrCopy[len] = (TCHAR) 0;    // null character 
	        GlobalUnlock(hglbCopy);          // Place the handle on the clipboard.  
	        SetClipboardData(CF_TEXT, hglbCopy); 
        }

        if (!::CloseClipboard()) 
		{
			Log.WinError(_WARNING_,"CloseClipboard() failed");
			return;	        
        }
    }
CATCH_THROW("ClientConnection::UpdateLocalClipboard")
}
