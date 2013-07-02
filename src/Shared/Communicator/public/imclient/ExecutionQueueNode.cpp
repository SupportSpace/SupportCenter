//===========================================================================
// SupportSpace ltd. @{SRCH}
//								CNodeSendMessage
//
//---------------------------------------------------------------------------
// DESCRIPTION: @{HDES}
// -----------
// CExecutionQueueNode :  class
//---------------------------------------------------------------------------
// CHANGES LOG: @{HREV}
// -----------
// Revision: 01.00
// By      : Alex Gantman
// Date    : 10/02/2007 05:21:10 PM
// Comments: First Issue
// Modified by: Anatoly Gutnick	
//===========================================================================
#include <windows.h>
#include "ExecutionQueueNode.h"
#include <AidLib/Logging/cLog.h> 
#include "AidLib/CException/CException.h"

CNodeSendMessage::CNodeSendMessage(const tstring& sTo,const  tstring& sMessageSubject,const tstring& sMessageBody, const  tstring& sToResource):
m_sTo(sTo),m_sMessageSubject(sMessageSubject),m_sMessageBody(sMessageBody),m_sToResource(sToResource)
{
TRY_CATCH
CATCH_THROW(_T("CNodeSendMessage::CNodeSendMessage"))
}

CNodeSendMessage::~CNodeSendMessage(void)
{
TRY_CATCH
CATCH_LOG(_T("CNodeSendMessage::~CNodeSendMessage"))
}

// Send IM Message
void CNodeSendMessage::PerformJob(CJabberWrapper<class CCommunicator>& wrapper)
{
TRY_CATCH
	send_msg( wrapper,m_sTo, m_sMessageBody, m_sMessageSubject, m_sToResource );
CATCH_THROW(_T("CNodeSendMessage::PerformJob"))
}