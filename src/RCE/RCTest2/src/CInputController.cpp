#include "CInputController.h"
#include <AidLib/CException/CException.h>
#include <AidLib/CScopedTracker/CScopedTracker.h>

CInputController::CInputController()
{
TRY_CATCH
CATCH_THROW()
}

CInputController::~CInputController()
{
TRY_CATCH
CATCH_LOG()
}

void CInputController::Init()
{
TRY_CATCH
	CScopedTracker<SC_HANDLE> manager(OpenSCManager(NULL, NULL, GENERIC_READ), CloseServiceHandle);
	if(!manager.get())
		throw MCException_Win(_T("Can not open service manager. "));
	CScopedTracker<SC_HANDLE> driver(OpenService(manager.get(), INPUT_DRIVER_NAME, SERVICE_START | SERVICE_QUERY_STATUS), CloseServiceHandle);
	if(!driver.get())
	{
		DWORD err = GetLastError();
		if((ERROR_INVALID_NAME == err) || (ERROR_SERVICE_DOES_NOT_EXIST == err))
		{
			Log.Add(_MESSAGE_, _T("Service not found. Try to reopen manager and create/install service..."));
			manager.reset(OpenSCManager(NULL, NULL, GENERIC_READ | GENERIC_WRITE), CloseServiceHandle);
			if(!manager.get())
				throw MCException_Win(_T("Can not reopen service manager for service creation. "));
			driver.reset(CreateService(
				manager.get(),
				INPUT_DRIVER_NAME,
				INPUT_DRIVER_NAME,
				SERVICE_START | SERVICE_QUERY_STATUS,
				SERVICE_KERNEL_DRIVER,
				SERVICE_DEMAND_START,
				SERVICE_ERROR_IGNORE,
				INPUT_DRIVER_PATH,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL), CloseServiceHandle);
			if(!driver.get())
				MCException_Win(_T("Can not create service. "));
		}
		else
			throw MCException(Format(_T("Can not open service. WinError: %d"), err));
	}
	SERVICE_STATUS status;
	BOOL res = QueryServiceStatus(driver.get(), &status);
	if(FALSE == res)
		throw MCException_Win(_T("Can not get service status. "));
	else
	{
		switch(status.dwCurrentState)
		{
		case SERVICE_STOPPED:
			{
				res = StartService(driver.get(), 0, NULL);
				if(FALSE == res)
					throw MCException_Win(_T("Can not get service status. "));
				break;
			}
		case SERVICE_RUNNING:
		case SERVICE_START_PENDING:
			{
				Log.Add(_MESSAGE_, _T("Service already started."));
				break;
			}
		default:
			throw MCException(_T("Service has incorrect state."));
		}
	}
CATCH_THROW()
}

void CInputController::Start(HWND window, unsigned int mouseMsg, unsigned int keyboardMsg)
{
TRY_CATCH
	if(m_driverHandle.get())
		throw MCException(_T("Controller already started."));
	HANDLE driver = CreateFile(INPUT_DEVICE_NAME, 0, 0, 0, OPEN_EXISTING, 0, 0); 
	if(INVALID_HANDLE_VALUE == driver)
		throw MCException_Win(_T("Can not open driver. "));
	m_driverHandle.reset(driver, CloseHandle);

	DWORD Ret = 0;
	BOOL Res = DeviceIoControl(driver, CTRL_SSINPUT_ENABLE, NULL, 0, NULL, 0, &Ret, NULL);
	if(FALSE == Res)
	{
		m_driverHandle.reset();
		throw MCException_Win(_T("Failed to start input handling. "));
	}

	m_mouseThread.reset(new CMouseInputThread());
	m_keyboardThread.reset(new CKeyboardInputThread());

	m_mouseThread->Init(m_driverHandle.get(), window, mouseMsg);
	m_keyboardThread->Init(m_driverHandle.get(), window, keyboardMsg);

	m_mouseThread->Start();
	m_keyboardThread->Start();
CATCH_THROW()
}

void CInputController::Stop()
{
TRY_CATCH
	if(!m_driverHandle.get())
		throw MCException(_T("Controller already stopped."));
	m_mouseThread->Terminate();
	m_keyboardThread->Terminate();
	DWORD Ret = 0;
	BOOL Res = DeviceIoControl(m_driverHandle.get(), CTRL_SSINPUT_DISABLE, NULL, 0, NULL, 0, &Ret, NULL);
	if(FALSE == Res)
		Log.Add(_WARNING_, _T("Failed to stop input handling (%d)."), GetLastError());
	m_driverHandle.reset();
	WaitForSingleObject(m_mouseThread->hTerminatedEvent.get(), INFINITE);
	m_mouseThread.reset();
	WaitForSingleObject(m_keyboardThread->hTerminatedEvent.get(), INFINITE);
	m_keyboardThread.reset();
CATCH_THROW()
}

bool CInputController::Started() const
{
TRY_CATCH
	return (NULL != m_driverHandle.get());
CATCH_THROW()
}
