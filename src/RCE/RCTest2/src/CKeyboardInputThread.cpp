#include "CKeyboardInputThread.h"
#include <AidLib/CException/CException.h>

CKeyboardInputThread::CKeyboardInputThread()
{
TRY_CATCH
CATCH_THROW()
}

CKeyboardInputThread::~CKeyboardInputThread()
{
TRY_CATCH
CATCH_LOG()
}

void CKeyboardInputThread::GetAndProcessInput()
{
TRY_CATCH
	DWORD Ret = 0;
	BOOL Res;
	PKEYBOARD_INPUT_DATA kbrdData = new KEYBOARD_INPUT_DATA;
	Res = DeviceIoControl(m_driverHandle, CTRL_SSINPUT_GET_KBRD_DATA, NULL, 0, kbrdData, sizeof(KEYBOARD_INPUT_DATA), &Ret, NULL);
	if((TRUE == Res) && Ret)
	{
		if(FALSE == PostMessage(m_window, m_msg, reinterpret_cast<WPARAM>(kbrdData), 0))
			delete kbrdData;
	}
	else
		delete kbrdData;
	Sleep(1);

CATCH_THROW()
}
