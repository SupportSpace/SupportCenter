/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  C_IRCInstallerAXCtrlEvents.cpp
///
///  C_IRCInstallerAXCtrlEvents object implementation. The object is events receiever of RCViewerAXCtrl events
///
///  @author Kirill Solovyov @date 24.10.2007
///
////////////////////////////////////////////////////////////////////////
// C_IRCInstallerAXCtrlEvents.cpp : Implementation of C_IRCInstallerAXCtrlEvents

#include "stdafx.h"
#include "C_IRCInstallerAXCtrlEvents.h"

#include <AidLib/logging/clog.h>
#include <AidLib/CException/CException.h>
#include <RCEngine/AXstuff/AXstuff.h>


// C_IRCInstallerAXCtrlEvents
C_IRCInstallerAXCtrlEvents::C_IRCInstallerAXCtrlEvents():
	m_owner(NULL)
{
TRY_CATCH
CATCH_LOG()
}

HRESULT C_IRCInstallerAXCtrlEvents::FinalConstruct()
{
TRY_CATCH

CATCH_LOG()
	return S_OK;
}

void C_IRCInstallerAXCtrlEvents::FinalRelease()
{
TRY_CATCH
CATCH_LOG()
}

STDMETHODIMP C_IRCInstallerAXCtrlEvents::NotifyLogMessage(BSTR message, long severity)
{
TRY_CATCH
	USES_CONVERSION;
	//Log.Add(McEventDesc(cLog::eSeverity(severity),_TRACE_DEBUG_,NULL,NULL),OLE2T(message));
CATCH_LOG_COMERROR()
}
STDMETHODIMP C_IRCInstallerAXCtrlEvents::NotifyFeatureInstalled(long result)
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("C_IRCInstallerAXCtrlEvents::NotifyFeatureInstalled(0x%x)"),result);
	if(!m_owner)
		throw MCException("Owner don't set");
	//m_owner->SendRspndInstalled(result);
	//m_owner->RCApproved(BRR_APPROVED);
CATCH_LOG_COMERROR()
}
STDMETHODIMP C_IRCInstallerAXCtrlEvents::NotifyInstalling(long percentCompleted, BSTR status)
{
TRY_CATCH
	USES_CONVERSION;
	if(!m_owner)
		throw MCException("Owner don't set");
	//m_owner->SendRspndInstalling(percentCompleted,tstring(OLE2T(status)));
CATCH_LOG_COMERROR()
}
