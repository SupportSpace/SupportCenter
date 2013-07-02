/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CWallpaperSwitch.cpp
///
///  Class for switching (temporary hiding) wallpaper
///
///  @author "Archer Software" Sogin M. @date 14.11.2007
///
////////////////////////////////////////////////////////////////////////

#include "..\stdafx.h"
#include "CWallpaperSwitch.h"
#include <WinInet.h>
#include <shellapi.h>
#include <shlobj.h>
#include <atlcomcli.h>
#include <Sddl.h>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/bind.hpp>
#include <AidLib/Utils/Utils.h>

int CWallpaperSwitch::m_originalPid = 0;

CWallpaperSwitch::CWallpaperSwitch()
	:	CProcessWatchDog(&CWallpaperSwitch::RestoreWall)
{
TRY_CATCH
CATCH_THROW()
}

CWallpaperSwitch::~CWallpaperSwitch()
{
TRY_CATCH
	Stop(false, STOP_TIMEOUT);
	CCritSection cs(&m_criticalSection); // Just to prevent map corruption
	if (!m_clients.empty())
		RestoreWall(); // There were outstanding hide requests, so Restoreing back wall
CATCH_LOG()
}

void CWallpaperSwitch::HideWallpaperRequest(const int pid)
{
TRY_CATCH
	CCritSection cs(&m_criticalSection);
	bool visible = m_clients.empty();
	AddClient(pid); // Adding client before hiding wall to ensure pid is correct
	if (visible)
	{
		m_originalPid = pid;
		EnableActiveDesktop(false);
	}
CATCH_THROW()
}

void CWallpaperSwitch::RestoreWallpaperRequest(const int pid)
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("Restore wallpaper request from %d"),pid);
	CCritSection cs(&m_criticalSection);
	for(std::set<HandlePidPair, SPidCompare>::iterator client = m_clients.begin();
		client != m_clients.end();
		++client)
	{
		if (client->second == pid)
		{
			Log.Add(_MESSAGE_,_T("Watching process pid(%d) handle(%d) request for wallpaper Restore"),client->second, client->first.get());
			m_clients.erase(client);
			break;
		}
	}
	SetEvent(m_unblockEvent.get()); ///Telling to main thread - something in client map changed
CATCH_THROW()
}

struct __declspec(uuid("F490EB00-1240-11D1-9888-006097DEACF9")) IActiveDesktop;
void CWallpaperSwitch::EnableActiveDesktop(const bool enable)
{
TRY_CATCH

	if (enable)
		Log.Add(_MESSAGE_,_T("Enabling wallpaper"));
	else
	{
		Log.Add(_MESSAGE_,_T("Disabling wallpaper"));
		if (FALSE == SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, "", SPIF_SENDCHANGE))
			Log.WinError(_WARNING_,_T("Failed to SystemParametersInfo: "));
	}

	CComQIPtr<IActiveDesktop, &IID_IActiveDesktop>	activeDesktop;
	HRESULT		hr;
	CoInitialize(NULL);
	hr = activeDesktop.CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER);
	if (!SUCCEEDED(hr))
		throw MCException(Format(_T("Failed to CoCreateInstance CLSID_ActiveDesktop %d"),hr));

	COMPONENTSOPT	opt;
	opt.dwSize = sizeof(opt);
	opt.fActiveDesktop = opt.fEnableComponents = enable;
	hr = activeDesktop->SetDesktopItemOptions(&opt, 0);
	if (!SUCCEEDED(hr))
		Log.Add(_WARNING_,_T("Failed to SetDesktopItemOptions %d"),hr);

	hr = activeDesktop->ApplyChanges(AD_APPLY_REFRESH);
	if (!SUCCEEDED(hr))
		Log.Add(_WARNING_,_T("Failed to activeDesktop->ApplyChanges %d"),hr);

	if (enable)
	{
		if (FALSE == SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, NULL, SPIF_SENDCHANGE))
			Log.WinError(_WARNING_,_T("Failed to SystemParametersInfo: "));
	}

CATCH_THROW()
}

void CWallpaperSwitch::RestoreWall()
{
TRY_CATCH
	CScopedTracker<HANDLE> process;
	process.reset(OpenProcess(PROCESS_QUERY_INFORMATION, NULL, m_originalPid),CloseHandle);
	if (NULL == process)
		throw MCException_Win("Failed to OpenProcess");


	HANDLE newToken, currentToken;
	if (0 == OpenProcessToken(process, TOKEN_ALL_ACCESS, &currentToken))
		throw MCException_Win("Failed to OpenProcessToken");

	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > spCurrentToken( currentToken, CloseHandle );

	if (0 == DuplicateTokenEx(spCurrentToken.get(), MAXIMUM_ALLOWED, NULL, SecurityImpersonation, TokenPrimary, &newToken))
		throw MCException_Win("Failed to Duplicate process token");

	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > spNewToken( newToken, CloseHandle );

	/// Setting highest integrity level
	TCHAR integritySid[20] = _T("S-1-16-16384");
	PSID pIntegritySid = NULL;
	if (ConvertStringSidToSid(integritySid, &pIntegritySid))
	{
		typedef struct _TOKEN_MANDATORY_LABEL 
		{
			SID_AND_ATTRIBUTES Label;
		} TOKEN_MANDATORY_LABEL,*PTOKEN_MANDATORY_LABEL;
		TOKEN_MANDATORY_LABEL TIL = {0};
		TIL.Label.Attributes = 0x00000020L /*SE_GROUP_INTEGRITY*/;
		TIL.Label.Sid = pIntegritySid;

		// Set the process integrity level
		if (!SetTokenInformation(	spNewToken.get(), 
									(TOKEN_INFORMATION_CLASS)25/*TokenIntegrityLevel*/, 
									&TIL,
									sizeof(TOKEN_MANDATORY_LABEL) + GetLengthSid(pIntegritySid)))
		{
			Log.WinError(_WARNING_,_T("Failed to SetTokenInformation"));
		}
	} else
	{
		Log.WinError(_WARNING_,_T("Failed to ConvertStringSidToSid"));
	}

	boost::shared_ptr<PROCESS_INFORMATION> pi;
	pi.reset(new PROCESS_INFORMATION, ReleaseProcessInformation);
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.lpDesktop = _T("winsta0\\default");

	TCHAR moduleName[MAX_PATH];
	GetModuleFileName(reinterpret_cast<HMODULE>(NULL), moduleName, MAX_PATH);

	if (0 == CreateProcessAsUser(	spNewToken.get(),
									NULL,
									(LPSTR)Format(_T("%s /%s"),moduleName,g_enableWallCmd.c_str()).c_str(),
									NULL,
									NULL,
									FALSE,
									NORMAL_PRIORITY_CLASS,
									NULL,
									NULL,
									&si,
									pi.get()
									))
		throw MCException_Win("Failed to CreateProcessAsUser");

	// Waiting for process termination
	if (WAIT_OBJECT_0 != WaitForSingleObject(pi->hProcess, RESTORE_WALL_TIMEOUT))
		Log.WinError(_WARNING_,_T("Wait for helper process failed: "));

CATCH_THROW()
}
