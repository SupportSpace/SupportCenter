#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <AidLib/Loki/Singleton.h>
#include "CMouseInputThread.h"
#include "CKeyboardInputThread.h"

class CInputController
{
private:
	CInputController(const CInputController&);
	CInputController& operator=(const CInputController&);
public:
	CInputController();
	~CInputController();
	void Init();
	void Start(HWND window, unsigned int mouseMsg, unsigned int keyboardMsg);
	void Stop();
	bool Started() const;
private:
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type >	m_driverHandle;
	boost::shared_ptr<CMouseInputThread>						m_mouseThread;
	boost::shared_ptr<CKeyboardInputThread>						m_keyboardThread;

};

#define INPUT_CONTROLLER_INSTANCE Loki::SingletonHolder<CInputController, Loki::CreateUsingNew, Loki::DefaultLifetime, Loki::ClassLevelLockable>::Instance()
