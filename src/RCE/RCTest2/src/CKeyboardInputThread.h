#pragma once

#include "CInputThread.h"

class CKeyboardInputThread
	:	public CInputThread
{
private:
	CKeyboardInputThread(const CKeyboardInputThread&);
	CKeyboardInputThread& operator=(const CKeyboardInputThread&);
public:
	CKeyboardInputThread();
	~CKeyboardInputThread();
protected:
	virtual void GetAndProcessInput();
};
