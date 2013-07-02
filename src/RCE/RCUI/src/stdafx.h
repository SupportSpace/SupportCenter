// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef STRICT
#define STRICT
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows 95 and Windows NT 4 or later.
	//#define WINVER 0x0400		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
	#define WINVER 0x0500 //for use AnimateWindow() win32 function
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0400	// Change this to the appropriate value to target Windows 2000 or later.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 4.0 or later.
#define _WIN32_IE 0x0400	// Change this to the appropriate value to target IE 5.0 or later.
#endif

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE
//#define _ATL_DEBUG_QI
//#define _ATL_DEBUG_INTERFACES


#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

// turns off ATL's hiding of some common and often safely ignored warning messages
#define _ATL_ALL_WARNINGS

#include <atlbase.h>
#include <atlcom.h>
#include <atlwin.h>
#include <atltypes.h>
#include <atlctl.h>
#include <atlhost.h>

#include <atlsync.h>

using namespace ATL;



/// Inner remote control messeges
enum ERemoteControlMessages
{
	//RCC_RESPONSE=0x80000000, // respons on any message, it mark original message as response via bitwise OR operation (for exapmle: RCC_START|BCC_RESPONSE - response on start),
	RCC_PING=0,              // ping message
	RCC_START,               // start remote control
	RCC_START_REQ,            // RC session request (ask user)
	RCC_START_REQ_DECLINE,    // response on RCC_START_REQ with user declined
	RCC_START_REQ_APPROVE,   // response on RCC_START_REQ with user approved
	RCC_RESERVED
};

/// RC sub stream identifiers
enum ERCSubStreamId
{
	RCSSID_SERVICE=0,    // service sub stream
	RCSSID_RC,           // remote control stream
	RCSSID_RESERVED
};

/// RC sub stream priorities
#define RCSSP_SERVICE 10L    // service sub stream
#define RCSSP_RC 1L          // remote control stream

