/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  C_IRCHostAXCtrlEvents.cpp
///
///  C_IRCHostAXCtrlEvents object implementation. The object is events receiever of RCHostAXCtrl events
///
///  @author Kirill Solovyov @date 24.10.2007
///
////////////////////////////////////////////////////////////////////////
// C_IRCHostAXCtrlEvents.cpp : Implementation of C_IRCHostAXCtrlEvents

#include "stdafx.h"
#include "C_IRCHostAXCtrlEvents.h"
#include <AidLib/CException/CException.h>
#include <RCEngine/AXstuff/AXstuff.h>



// C_IRCHostAXCtrlEvents

C_IRCHostAXCtrlEvents::C_IRCHostAXCtrlEvents():
	m_owner(NULL)
{
TRY_CATCH
CATCH_LOG()
}

HRESULT C_IRCHostAXCtrlEvents::FinalConstruct()
{
TRY_CATCH

CATCH_LOG()
	return S_OK;
}

void C_IRCHostAXCtrlEvents::FinalRelease()
{
TRY_CATCH
CATCH_LOG()
}

STDMETHODIMP C_IRCHostAXCtrlEvents::NotifySessionStart(long clientId)
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("C_IRCHostAXCtrlEvents::NotifySessionStart(0x%x)"),clientId);
	if(m_owner)
	{
		m_owner->m_hostClientId=clientId;
		m_owner->SetAccessMode(m_owner->m_accessMode);
	}
	else
		return E_NOTIMPL;
CATCH_LOG_COMERROR()
}
STDMETHODIMP C_IRCHostAXCtrlEvents::NotifySessionStop(long clientId, long reasonCode)
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("C_IRCHostAXCtrlEvents::NotifySessionStop(0x%x,0x%x)"), clientId, reasonCode);
CATCH_LOG_COMERROR()
}

STDMETHODIMP C_IRCHostAXCtrlEvents::NotifyConnecting(long percentCompleted, BSTR status)
{
TRY_CATCH
	USES_CONVERSION;
	Log.Add(_MESSAGE_,_T("C_IRCHostAXCtrlEvents::NotifyConnecting(%d,%s)"),percentCompleted,status);

	return E_NOTIMPL;
CATCH_LOG_COMERROR()
}