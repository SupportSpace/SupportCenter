#include "CMouseInputThread.h"
#include <AidLib/CException/CException.h>

CMouseInputThread::CMouseInputThread()
	:	m_screenX(1)
	,	m_screenY(1)
{
TRY_CATCH
	m_screenX = GetSystemMetrics(SM_CXSCREEN);
	m_screenY = GetSystemMetrics(SM_CYSCREEN);
CATCH_THROW()
}

CMouseInputThread::~CMouseInputThread()
{
TRY_CATCH
CATCH_LOG()
}

void CMouseInputThread::GetAndProcessInput()
{
TRY_CATCH
	DWORD Ret = 0;
	BOOL Res;
	PMOUSE_INPUT_DATA mouseData = new MOUSE_INPUT_DATA;

	Res = DeviceIoControl(m_driverHandle, CTRL_SSINPUT_GET_MOUSE_DATA, NULL, 0, mouseData, sizeof(MOUSE_INPUT_DATA), &Ret, NULL);
	if((TRUE == Res) && Ret)
	{
//		char buf[MAX_PATH];
//		memset(buf, 0, MAX_PATH);
		if(MOUSE_MOVE_ABSOLUTE == (mouseData->Flags & MOUSE_MOVE_ABSOLUTE))
		{
			int x = mouseData->LastX * m_screenX / MAGIC;
			int y = mouseData->LastY * m_screenY / MAGIC;
//			sprintf(buf, "Mouse absolute move (%d, %d) -> (%d, %d)\n", mouseData->LastX, mouseData->LastY, x, y);
//			OutputDebugString(buf);
			mouseData->LastX = x;
			mouseData->LastY = y;
		}
		else
		{
//			sprintf(buf, "Mouse relative move (%d, %d)\n", mouseData->LastX, mouseData->LastY);
//			OutputDebugString(buf);
		}

		CFakedPointer::EState state;
		if(MOUSE_LEFT_BUTTON_DOWN == (mouseData->ButtonFlags & MOUSE_LEFT_BUTTON_DOWN))
		{
//			OutputDebugString("MOUSE LEFT BUTTON\n");
			state = CFakedPointer::LBTN_DOWN;
		}
		else
		{
			if(MOUSE_RIGHT_BUTTON_DOWN == (mouseData->ButtonFlags & MOUSE_RIGHT_BUTTON_DOWN))
				state = CFakedPointer::RBTN_DOWN;
			else
				state = CFakedPointer::NORMAL;
		}
/*
		if(MOUSE_MOVE_ABSOLUTE == (mouseData->Flags & MOUSE_MOVE_ABSOLUTE))
			OutputDebugString("MOUSE_MOVE_ABSOLUTE");
		if(MOUSE_MOVE_RELATIVE == (mouseData->Flags & MOUSE_MOVE_RELATIVE))
			OutputDebugString("MOUSE_MOVE_RELATIVE");
		if(MOUSE_VIRTUAL_DESKTOP == (mouseData->Flags & MOUSE_VIRTUAL_DESKTOP))
			OutputDebugString("MOUSE_VIRTUAL_DESKTOP");
		if(MOUSE_ATTRIBUTES_CHANGED == (mouseData->Flags & MOUSE_ATTRIBUTES_CHANGED))
			OutputDebugString("MOUSE_ATTRIBUTES_CHANGED");

*/

		if(MOUSE_MOVE_ABSOLUTE == (mouseData->Flags & MOUSE_MOVE_ABSOLUTE))
		{
			FAKED_POINTER_INSTANCE.MoveTo(mouseData->LastX,mouseData->LastY);
		}
		else
		{
			FAKED_POINTER_INSTANCE.MoveToRel(mouseData->LastX,mouseData->LastY);
			POINT p = FAKED_POINTER_INSTANCE.GetPos();
			mouseData->LastX = p.x;
			mouseData->LastY = p.y;
		}

		FAKED_POINTER_INSTANCE.SetState(state);

		if(FALSE == PostMessage(m_window, m_msg, reinterpret_cast<WPARAM>(mouseData), 0))
			delete mouseData;
	}
	else
	{
		delete mouseData;
		Sleep(1);
	}
CATCH_THROW()
}
