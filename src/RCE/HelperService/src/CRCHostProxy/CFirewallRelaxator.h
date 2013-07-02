/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CFirewallRelaxator.h
///
///  Declares CFirewallRelaxator class, responsible for allowing incoming
///    socket connections
///
///  @author Dmitry Netrebenko @date 25.12.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/WatchDog/CProcessWatchDog.h>
#include <AidLib/Strings/tstring.h>

#pragma comment(lib,"nwl.lib")
#pragma comment(lib,"Psapi.lib")

/// CFirewallRelaxator class, responsible for allowing incoming
///   socket connections
class CFirewallRelaxator
	:	protected CProcessWatchDog
{
private:
/// Prevents making copies of CFirewallRelaxator objects.
	CFirewallRelaxator( const CFirewallRelaxator& );
	CFirewallRelaxator& operator=( const CFirewallRelaxator& );
public:
/// Constructor
	CFirewallRelaxator();
/// Destructor
	~CFirewallRelaxator();
private:
/// Restores firewall settings for all clients
	static void AllClientsTerminated();
/// Returns path of process executable by pid
/// @param pid - process id
	static tstring GetAppPathByPid(const int pid);
public:
/// Allows incoming socket connections for process
/// @param pid - process id
	void AllowIncomingConnections(const int pid);
/// Restores firewall settings for process
/// @param pid - process id
	void RestoreFirewallSettings(const int pid);
};
