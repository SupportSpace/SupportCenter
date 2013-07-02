//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CCommandProxy.cpp
///
///  Implements CCommandProxy class
///  Uses in widgets for communication with CCommandManager
///  
///  @author Alexander Novak @date 22.11.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CCommandProxy.h"
#include "CCommandManager.h"

// CCommandProxy [BEGIN] /////////////////////////////////////////////////////////////////////////////////

CCommandProxy::CCommandProxy(boost::shared_ptr<CAbstractCommandManager> manager)
	:	m_commandManager(manager)
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandProxy::DispatchCommand(EWidgetCommand command, const void* commandData)
{
TRY_CATCH

	m_commandManager->ExecuteCommand(this,command,commandData);

CATCH_THROW()
}
// CCommandProxy [END] ///////////////////////////////////////////////////////////////////////////////////
