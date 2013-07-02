#pragma once

#include <AidLib/Strings/tstring.h>
#include <AidLib/Logging/CLog.h>
#include <AidLib/CException/CException.h>
#include <NWL/Events/Events.h>
#include "CRCHost.h"
#include "CRCViewer.h"
#include "PermissibleWarnings.h"

#pragma comment (lib, "Winmm.lib")
#pragma comment (lib, "Version.lib") ///TODO: if driver will be removed remove this dependency
#ifndef _SELF
	#pragma comment(lib, "RCEngine.lib")
#endif

#pragma comment(lib, "wininet.lib")

#pragma comment(lib, "nwl.lib")
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "libjpeg.lib")
#pragma comment(lib, "Xregion.lib")
#pragma comment(lib, "udt.lib")
#ifdef _DYNAMIC_NWL_
	#pragma comment(lib, "w32gnutls.lib")
#else
	#pragma comment(lib, "gnutls.lib")
#endif
#pragma comment(lib, "aidlib.lib")

// Try count for broken session
#define MAX_SESSION_RESTARTS 10
#define WAIT_RESTART_TIMEOUT 10000 /*10 secs*/
