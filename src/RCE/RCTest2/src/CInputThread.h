#pragma once

#include <AidLib/CThread/CThread.h>
#include <windows.h>
#include "InputShared.h"

class CInputThread
	:	public CThread
{
private:
	CInputThread(const CInputThread&);
	CInputThread& operator=(const CInputThread&);

	virtual void Execute(void *Params);

public:
	CInputThread();
	~CInputThread();
	void Init(HANDLE driverHandle, HWND window, unsigned int msg);

protected:
	HANDLE			m_driverHandle;
	HWND			m_window;
	unsigned int	m_msg;
	virtual void GetAndProcessInput() = NULL;

};
