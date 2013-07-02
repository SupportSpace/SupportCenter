/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CFirewallRelaxator.cpp
///
///  Implements CFirewallRelaxator class, responsible for allowing incoming
///    socket connections
///
///  @author Dmitry Netrebenko @date 25.12.2007
///
////////////////////////////////////////////////////////////////////////

#include "CFirewallRelaxator.h"
#include <AidLib/CException/CException.h>
#include <NWL/Streaming/CFirewallConfigurator.h>
#include <AidLib/CScopedTracker/CScopedTracker.h>
#include <Psapi.h>

CFirewallRelaxator::CFirewallRelaxator()
	:	CProcessWatchDog(&CFirewallRelaxator::AllClientsTerminated)
{
TRY_CATCH
CATCH_THROW()
}

CFirewallRelaxator::~CFirewallRelaxator()
{
TRY_CATCH
CATCH_LOG()
}

void CFirewallRelaxator::AllClientsTerminated()
{
TRY_CATCH
	CoInitialize(NULL);
	FIREWALL_CONFIGURATOR_INSTANCE.RestoreAll();
	CoUninitialize();
CATCH_THROW()
}

tstring CFirewallRelaxator::GetAppPathByPid(const int pid)
{
TRY_CATCH
	CScopedTracker<HANDLE> process;
	process.reset(OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid), CloseHandle);
	if(NULL == process)
		throw MCException_Win("Failed to OpenProcess");
	TCHAR buf[MAX_PATH];
	memset(buf, 0, MAX_PATH);
	if(!GetModuleFileNameEx(process.get(), NULL, buf, MAX_PATH))
		throw MCException_Win("Failed to GetModuleFileNameEx");
	return tstring(buf);
CATCH_THROW()
}

void CFirewallRelaxator::AllowIncomingConnections(const int pid)
{
TRY_CATCH
	CCritSection cs(&m_criticalSection);
	AddClient(pid);
	TRY_CATCH
		FIREWALL_CONFIGURATOR_INSTANCE.AllowIncomingForApplication(GetAppPathByPid(pid));
	CATCH_LOG()
CATCH_THROW()
}

void CFirewallRelaxator::RestoreFirewallSettings(const int pid)
{
TRY_CATCH
	TRY_CATCH
		FIREWALL_CONFIGURATOR_INSTANCE.RestoreSettingsForApplication(GetAppPathByPid(pid));
	CATCH_LOG()
	CCritSection cs(&m_criticalSection);
	for(std::set<HandlePidPair, SPidCompare>::iterator client = m_clients.begin();
		client != m_clients.end();
		++client)
	{
		if (client->second == pid)
		{
			Log.Add(_MESSAGE_,_T("Watching process pid(%d) handle(%d) request for restore firewall settings"),client->second, client->first.get());
			m_clients.erase(client);
			break;
		}
	}
	SetEvent(m_unblockEvent.get());
CATCH_THROW()
}
