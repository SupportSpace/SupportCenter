#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CService.h
///
///  Windows service wrapper class
///
///  @author "Archer Software" Sogin M. @date 28.09.2007
///
////////////////////////////////////////////////////////////////////////

#include <AidLib/strings/tstring.h>
#include <AidLib/Logging/cLog.h>
#include <AidLib/CException/CException.h>
#include <atlbase.h>
#include <atlconv.h>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/shared_ptr.hpp>
#include <AidLib/CScopedTracker/CScopedTracker.h>

/// Timeout, after which service will report 'Stopped'forcedly
#define DEF_SERVICE_STOP_TIMEOUT INFINITE

/// Use this macro to start service class declaration. @see CService
#define SERVICE_CLASS(clsName)	class clsName;\
								clsName* CService<clsName>::m_this;\
								class clsName : public CService<clsName>\
								{ friend class CService<clsName>;

/// Use this macro to stop service class declaration. @see CService
#define END_OF_SERVICE_CLASS };

/// service wrapper class
///
/// usage
///
/// Descendant class should be inherited from CService specified with himself
/// For example if we implementing service in class CSomeService, declaraction should looks like
///
/// CSomeService : public CService<CSomeService>
///
/// To simplify usage of this template SERVICE_CLASS macros is defined
///
/// example
///
///			SERVICE_CLASS(CAService)
///				private:
///					bool m_stopped;
///					CAService() : m_stopped(false) {};
///
///					/// Service entry point
///					virtual void OnStart() 
///					{
///						Log.Add(_MESSAGE_,_T("Service started"));
///						while(!m_stopped)
///						{
///							Log.Add(_MESSAGE_,_T("Running"));
///							Sleep(1000);
///						}
///					};
///					/// Called when service manager try to stop service
///					virtual void OnStop() 
///					{
///						Log.Add(_MESSAGE_,_T("Service stopped"));
///						m_stopped = true;
///					};
///			END_OF_SERVICE_CLASS
///
template<class impl = void> class CService
{
public:
	/// Name of a service
	tstring m_serviceName;

	/// static this pointer
	static impl* m_this;

	/// Service status handle
	SERVICE_STATUS_HANDLE m_statusHandle;

	/// Handle to stopped event
	boost::shared_ptr<boost::remove_pointer<HANDLE>::type> m_stoppedEvent;
	
	/// A value that the service increments periodically to report 
	/// its progress during a lengthy start, stop, pause, or continue operation
	DWORD m_checkPoint;

	/// Current status of the service
	SERVICE_STATUS m_ssStatus;

	/// registers service in system
	/// @param displayName - [in] Display name of a service
	/// @param name - [in] Name of a service
	/// @param userName - [in] User name (of account for service to use)
	/// @param password  - [in] Password (of account for service to use)
	void Install(	const tstring& displayName, 
					const tstring& name,
					const tstring& userName=_T(""),
					const tstring& password=_T("") );

	/// Unregisters service from sys
	/// @param name [in] Name of service to uninstall
	void UnInstall(const tstring& name);
	

	/// ServiceMain function
	/// @param dwArgc - [in] number of command line arguments
	/// @param lpszArgv - [in] array of command line arguments
	static VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);

	/// This function is called by the SCM whenever ControlService() is
	/// @param dwCtrlCode - [in]
	static VOID WINAPI ServiceCtrl(DWORD dwCtrlCode);

	/// Service entry point
	virtual void OnStart() = NULL;

	/// Called when service manager try to stop service
	virtual void OnStop() = NULL;

	/// Sets the current status of the service and reports it to the Service
	/// ControlManager
	/// @param dwCurrentState - [in]
	/// @param dwWin32ExitCode - [in]
	/// @param dwWaitHint - [in]
	bool ReportStatusToSCM(	DWORD dwCurrentState,
							DWORD dwWin32ExitCode,
							DWORD dwWaitHint  );

protected:

	/// ctor
	CService();

public:

	/// returns instance of service
	static impl* instance()
	{
	TRY_CATCH
		static std::auto_ptr<impl> deleter;
		if (NULL == m_this)
		{
			m_this = new impl();
			deleter.reset(m_this);
		}
		return m_this;
	CATCH_THROW()
	}

	/// dtor
	virtual ~CService();

	/// Instance initializer
	/// @param lpCmdLine - [in]	Command line passed to application. /install - will install the service; /uninstall - will remove it
	/// @param serviceName - [in]	Service internal name
	/// @param displayName - [in]	Service display name
	void Init(LPSTR lpCmdLine, const tstring& serviceName=_T(""), const tstring &displayName=_T(""));

	/// Returns handle to stopped event
	inline HANDLE GetStoppedEvent() const throw()
	{
		return m_stoppedEvent.get();
	}
};

template<class impl> void CService<impl>::Install(	const tstring& displayName, 
						const tstring& name,
						const tstring& _userName,
						const tstring& password ) 
{
TRY_CATCH
	CScopedTracker<SC_HANDLE> scHandle;
	CScopedTracker<SC_HANDLE> serviceHandle;
	TCHAR modulePath[MAX_PATH];
	tstring userName(_userName);

	//Getting module name
	if (!GetModuleFileName(NULL,modulePath,MAX_PATH))
		throw MCException_Win("Failed to get module name: ");

	//Opening services manager
	scHandle.reset(OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS), CloseServiceHandle);
	if (0 == scHandle)
		throw MCException_Win("Failed to open OpenSCManager: ");
	else
	{
		if (userName.length())
		{
			if (userName.find(_T("\\"))==tstring::npos)
			{
				userName = _T(".\\") + userName;
			}
		}

		/// Uninstalling already installed service
		TRY_CATCH
			CScopedTracker<SC_HANDLE> serviceHandle; //To prevent deleted service hanging
			serviceHandle.reset(OpenService( scHandle, name.c_str(), GENERIC_READ), CloseServiceHandle);
			if (0 != serviceHandle)
				UnInstall( name );
		CATCH_LOG()

		serviceHandle.reset(CreateService(	scHandle,					// SCManager database
											name.c_str(),				// name of service
											displayName.c_str(),		// name to display
											SERVICE_ALL_ACCESS,			// desired access
											SERVICE_WIN32_OWN_PROCESS,	// service type
											SERVICE_AUTO_START,			// start type
											SERVICE_ERROR_NORMAL,		// error control type
											modulePath,					// service's binary
											NULL,						// no load ordering group
											NULL,						// no tag identifier
											NULL,						// no dependencies
											userName.length()?userName.c_str():NULL,			//UserName_T("NT AUTHORITY\\NetworkService")
											password.length()?password.c_str():NULL		//password 
											), CloseServiceHandle);

		if (0 == serviceHandle)
			throw MCException_Win("Failed to CreateService: ");

		if (0 == StartService(serviceHandle, NULL, NULL))
			throw MCException_Win("Failed to StartService: ");
	}
	Log.Add(_MESSAGE_,_T("Service \"%s\" installed successfully"),displayName.c_str());

CATCH_THROW()
}

template<class impl> void CService<impl>::UnInstall(const tstring& name) 
{
TRY_CATCH
	CScopedTracker<SC_HANDLE> scHandle;
	CScopedTracker<SC_HANDLE> serviceHandle;
	scHandle.reset(OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS),CloseServiceHandle);
	if (0 == scHandle)
		throw MCException_Win("Failed to OpenSCManager: ");
	else
	{
		serviceHandle.reset(OpenService(scHandle, name.c_str() , SERVICE_ALL_ACCESS),CloseServiceHandle);
		if (0 == serviceHandle)
			throw MCException_Win("Failed to OpenService: ");
		else
		{
			SERVICE_STATUS ss;
			if (0 == ControlService(serviceHandle, SERVICE_CONTROL_STOP, &ss))
				Log.WinError(_WARNING_,_T("Failed to stop service "));
			if (!DeleteService( serviceHandle ))
				throw MCException_Win("Failed to DeleteService: ");
		}
	}	
	Log.Add(_MESSAGE_,_T("Service \"%s\" uninstalled"),name.c_str());
	Sleep(3000); //TODO: remove!!!!
CATCH_THROW()
}

template<class impl> void CService<impl>::Init(LPSTR lpCmdLine, const tstring& serviceName, const tstring& _displayName) 
{
TRY_CATCH
	m_serviceName = serviceName;
	tstring displayName(_displayName);

	if (!m_serviceName.length()) //Setting default service name
	{
		TCHAR modulePath[MAX_PATH];
		//Getting module name
		if (!GetModuleFileName(NULL,modulePath,MAX_PATH))
			throw MCException_Win("Failed to get module name: ");
		m_serviceName=modulePath;
		size_t Pos;
		if (tstring::npos != (Pos=m_serviceName.find_last_of(_T('\\'))))
		{
			m_serviceName.erase(0,Pos+1);
		}
	}
	if (!displayName.length()) //Setting default display name
		displayName=m_serviceName;

	//Start parsing lpCmdLine---------------------------------------
	USES_CONVERSION;
	PTCHAR string = A2T(lpCmdLine);
	TCHAR seps[]   = _T(" ,\t\n");
	std::vector <tstring> Params;
	PTCHAR next_token;
	for (	PTCHAR token = _tcstok_s( string , seps , &next_token );
			token;
			token = _tcstok_s(NULL, seps, &next_token) )
		Params.push_back(token);


	if (Params.size())
	{
		if (tstring(CharUpper((PTCHAR)Params[0].c_str())) == _T("/INSTALL"))
		{
			//Installing service
			tstring UName,Password;
			if (Params.size()>1)
			{
				UName=Params[1];
				if (Params.size()>2)
					Password=Params[2];
			}
			Install(displayName,m_serviceName,UName,Password);
		} else
		if (tstring(CharUpper((PTCHAR)Params[0].c_str())) == _T("/UNINSTALL"))
		{
			//Uninstalling service
			UnInstall(m_serviceName);
		} else
			throw MCException(Format(_T("Unknown parametrs set: %s\n Usage <appName>  /install | /uninstall"),lpCmdLine));
		//End parsing lpCmdLine-----------------------------------------
	} 
	else
	{
		SERVICE_TABLE_ENTRY dispatchTable[] = 
		{
			{ (const PTCHAR)m_serviceName.c_str(), (LPSERVICE_MAIN_FUNCTION)CService::ServiceMain },
			{ NULL, NULL }
		};
		if (!StartServiceCtrlDispatcher(dispatchTable))
			throw MCException_Win("Failed to StartServiceCtrlDispatcher");
	}
CATCH_THROW()
}

template<class impl> CService<impl>::CService()  :
	m_serviceName(_T("")), 
	m_checkPoint(0)
{
TRY_CATCH
	m_stoppedEvent.reset(CreateEvent(NULL,true,false,NULL),CloseHandle);
CATCH_THROW()
}

template<class impl> CService<impl>::~CService() 
{
TRY_CATCH
	m_stoppedEvent.reset();
CATCH_LOG()
}

template<class impl> VOID WINAPI CService<impl>::ServiceCtrl(DWORD dwCtrlCode)
{
TRY_CATCH
	// Handle the requested control code.
	switch(dwCtrlCode)
	{
		// Stop the service.
	case SERVICE_CONTROL_STOP:
		// SERVICE_STOP_PENDING should be reported before
		// setting the Stop Event - StopEvent - in
		// Stop().  m_this avoids a race condition
		// which may result in a 1053 - The Service did not respond...
		// error.
		m_this->ReportStatusToSCM(SERVICE_STOP_PENDING, NOERROR, 0);
		m_this->OnStop();
		DWORD res;
		if (NULL != m_this->m_stoppedEvent.get())
			res = WaitForSingleObject(m_this->m_stoppedEvent.get(), DEF_SERVICE_STOP_TIMEOUT);
		if (WAIT_OBJECT_0 != res)
			Log.WinError(_WARNING_,_T("Wait for service stopped failed: "));
		m_this->ReportStatusToSCM(SERVICE_STOPPED, NOERROR, 0);
		return;

		//To get service state
	case SERVICE_CONTROL_INTERROGATE:
	default:
		m_this->ReportStatusToSCM(m_this->m_ssStatus.dwCurrentState, NOERROR, 0);
		break;
	}
CATCH_LOG()
}

template<class impl> bool CService<impl>::ReportStatusToSCM(	DWORD dwCurrentState,
									DWORD dwWin32ExitCode,
									DWORD dwWaitHint  )
{
TRY_CATCH
	// SERVICE_STATUS members that don't change 
	m_ssStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	m_ssStatus.dwServiceSpecificExitCode = 0;

	if (dwCurrentState == SERVICE_START_PENDING)
		m_ssStatus.dwControlsAccepted = 0;
	else
		m_ssStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

	m_ssStatus.dwCurrentState = dwCurrentState;
	m_ssStatus.dwWin32ExitCode = dwWin32ExitCode;
	m_ssStatus.dwWaitHint = dwWaitHint;

	if (( dwCurrentState == SERVICE_RUNNING ) ||
		( dwCurrentState == SERVICE_STOPPED ) )
			m_ssStatus.dwCheckPoint = 0;
	else
		m_ssStatus.dwCheckPoint = m_checkPoint++;

	// Report the status of the service to the service control manager.
	if (!SetServiceStatus( m_statusHandle, &m_ssStatus)) 
	{
		Log.WinError(_ERROR_,_T("Failed to Report the status of the service (SetServiceStatus) "));
		return false;
	}

	return true;
CATCH_THROW()
}

template<class impl> VOID WINAPI CService<impl>::ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
TRY_CATCH

	// register our service control handler:
	m_this->m_statusHandle = RegisterServiceCtrlHandler(m_this->m_serviceName.c_str(), ServiceCtrl);

	if (!m_this->m_statusHandle)
	{
		Log.WinError(_ERROR_,_T("CService::ServiceMain : RegisterServiceCtrlHandler failed "));
		return;
	}

	// Running ...
	m_this->ReportStatusToSCM(SERVICE_RUNNING, NOERROR, 0);

	if (NULL != m_this->m_stoppedEvent.get()) 
		ResetEvent(m_this->m_stoppedEvent.get());

	/// Calling service entry point method
	m_this->OnStart();

	if (NULL != m_this->m_stoppedEvent.get()) 
		SetEvent(m_this->m_stoppedEvent.get());

CATCH_LOG()
}