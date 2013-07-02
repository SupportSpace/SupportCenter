//===========================================================================
// SupportSpace ltd. @{SRCH}
//								IJabberInterface
//
//---------------------------------------------------------------------------
// DESCRIPTION: @{HDES}
// -----------
// CExecutionQueueNode :  class
//---------------------------------------------------------------------------
// CHANGES LOG: @{HREV}
// -----------
// Revision: 01.00
// By      : Alex Gantman
// Date    : 10/02/2007 05:21:10 PM
// Comments: First Issue
//===========================================================================
#pragma once

template<class T>class IJabberInterface
{
public:
	IJabberInterface(T& communicator):
			m_Communicator(communicator){}

	~IJabberInterface(void){};

	virtual void SignalQueueUpdate() = 0;

	virtual void CreateIMClient(
						const tstring&		username,
						const tstring&		resource,
						const tstring&		password, 
						const tstring&		server,
						const tstring& 		server_addr,
						HWND				hWnd,
						bool				bLog,
						DWORD				idleTimeout) = 0;

	virtual void DestroyIMClient() = 0;

protected:
	// Reference on Communicator class
	T&				m_Communicator;
	// Initiator window handle
	HWND		    m_hWnd;
};
