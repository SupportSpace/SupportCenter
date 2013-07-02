#pragma once

#include "CInputThread.h"
#include "CFakedPointer.h"

class CMouseInputThread
	:	public CInputThread
{
private:
	CMouseInputThread(const CMouseInputThread&);
	CMouseInputThread& operator=(const CMouseInputThread&);
public:
	CMouseInputThread();
	~CMouseInputThread();
protected:
	virtual void GetAndProcessInput();
private:
	int m_screenX;
	int m_screenY;
};
