/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CNotificationThread.h
///
///  Declares CNotificationThread class, responsible for thread which will send
///    notifications to remote side through infrastructure
///
///  @author Dmitry Netrebenko @date 12.03.2008
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/Strings/tstring.h>
#include <AidLib/CThread/CThread.h>
#include <atlbase.h>
#include "..\..\Brokers\Shared\IBrokerClient.h"
#include "..\..\Brokers\Shared\CAvailableServices.h"
#include "..\..\Brokers\Shared\BrokersTypes.h"

/// CNotificationThread class, responsible for thread which will send
///   notifications to remote side through infrastructure
/// Base class - CThread from AidLib
class CNotificationThread
	:	public CThread
{
private:
/// Prevents making copies of CNotificationThread objects.
	CNotificationThread( const CNotificationThread& );
	CNotificationThread& operator=( const CNotificationThread& );
public:
/// Constructor
	CNotificationThread();
/// Destructor
	~CNotificationThread();
/// Thread entry point
	virtual void Execute(void *Params);
/// Initializes notification thread
/// @param scriptName - name of script
/// @param brokerEvents - pointer to IBrokerClientEvents interface
	void Init(const tstring& scriptName, CComGITPtr<_IBrokerClientEvents> brokerEvents);
/// Sends activity notification
	void NotifyActivity(const tstring& activity);
/// Sends service start notification
	void NotifyServiceStart();
/// Sends service stop notification
	void NotifyServiceStop();
private:
/// Name of script
	tstring								m_scriptName;
/// Pointer to IBrokerClientEvents interface
	CComGITPtr<_IBrokerClientEvents>	m_brokerEvents;
};
